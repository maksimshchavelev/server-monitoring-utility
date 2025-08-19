/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ui/ui.cpp
 * @brief File with user interface
 */


#include "modules/ui/ui.hpp"
#include <ftxui/component/component.hpp>
#include <iostream>
#include <thread>


namespace smu {


// Public constructor
UI::UI() : m_screen(ftxui::ScreenInteractive::TerminalOutput()), m_tabs(make_tabs()) {}




// Public method
void UI::run_async() {
    std::thread runner([&]() {
        m_screen.Clear();
        m_screen.Loop(m_tabs);
    });
    runner.detach();
}




// Public method
void UI::stop() {
    m_screen.Clear();
    m_screen.Exit();

    while (m_screen.Active()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}




// Public method
void UI::set_data(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> data_copy = data;

    m_screen.Post([this, data_copy = std::move(data_copy)] {
        std::lock_guard<std::mutex> lock(m_data_mutex);

        std::vector<ftxui::Element>   m_tabs_headers;
        std::vector<ftxui::Component> m_tabs_content;

        // Unwrap
        std::size_t current = 0; // current byte
        current += 5; // skip version and payload size

        for (; current < data_copy.size();) {
            // If node type is container
            if (data_copy[current] == 0) {
                ++current;
                uint32_t name_length = read_uint32_be(data_copy, current);
                current += 4;
                // Read name
                auto module_name = std::string(data_copy.data() + current,
                                               data_copy.data() + current + name_length);
                current += name_length;
                uint32_t payload_size = read_uint32_be(data_copy, current);
                current += 4;
                // Read payload
                auto payload = std::vector<uint8_t>(data_copy.data() + current,
                                                    data_copy.data() + current + payload_size);
                // Insert module
                m_data[module_name] = std::move(payload);
                current += payload_size;
            }
        }

        // Set headers and content
        for (const auto& [key, value] : m_data) {
            m_tabs_headers.push_back(ftxui::text(key));
            m_tabs_content.push_back(unwrap_module(value));
        }

        m_tabs->set_data(std::move(m_tabs_headers), std::move(m_tabs_content));
        m_screen.PostEvent(ftxui::Event::Custom);
    });
}




// Private method
ftxui::Component UI::unwrap_module(const std::vector<uint8_t>& module_data) {
    using namespace ftxui;

    // ----- Quick error path -----
    if (module_data.empty()) {
        return Renderer([] { return text("Nothing to draw!"); });
    }

    // ----- Strongly typed node model with zero-copy slices -----
    enum class NodeKind : uint8_t { Container = 0, Value = 1 };

    struct ByteSlice {
        const char* ptr{nullptr};
        uint32_t    len{0};

        std::string_view sv() const {
            return std::string_view(ptr ? ptr : "", len);
        }
        bool empty() const { return len == 0; }
    };

    struct Node {
        NodeKind           kind{};
        ByteSlice          name;       // for both kinds
        std::vector<Node>  children;   // for container
        ByteSlice          units;      // for value
        ByteSlice          value;      // for value
    };

    // ----- Safe big-endian readers with bounds checks -----
    const uint8_t* begin = module_data.data();
    const uint8_t* end   = begin + module_data.size();

    auto read_u8 = [&](const uint8_t*& p, uint8_t& out) -> bool {
        if (p >= end) return false;
        out = *p++;
        return true;
    };

    auto read_u32_be = [&](const uint8_t*& p, uint32_t& out) -> bool {
        if (end - p < 4) return false;
        out = (uint32_t(p[0]) << 24) |
              (uint32_t(p[1]) << 16) |
              (uint32_t(p[2]) << 8)  |
              uint32_t(p[3]);
        p += 4;
        return true;
    };

    auto read_bytes = [&](const uint8_t*& p, uint32_t len, ByteSlice& out) -> bool {
        if (uint64_t(end - p) < uint64_t(len)) return false;
        out.ptr = reinterpret_cast<const char*>(p);
        out.len = len;
        p += len;
        return true;
    };

    // ----- Recursive MDTP parser (no allocations for text fields) -----
    std::function<bool(const uint8_t*&, Node&)> parse_node =
        [&](const uint8_t*& p, Node& out) -> bool {
        uint8_t type{};
        if (!read_u8(p, type)) return false;
        out.kind = (type == 0x00) ? NodeKind::Container : NodeKind::Value;

        uint32_t name_len{};
        if (!read_u32_be(p, name_len)) return false;
        if (!read_bytes(p, name_len, out.name)) return false;

        if (out.kind == NodeKind::Container) {
            uint32_t payload_size{};
            if (!read_u32_be(p, payload_size)) return false;
            if (uint64_t(end - p) < payload_size) return false;

            const uint8_t* payload_end = p + payload_size;
            while (p < payload_end) {
                Node child;
                if (!parse_node(p, child)) return false;
                out.children.emplace_back(std::move(child));
            }
            return p == payload_end; // strictly consume payload

        } else {
            // Value node layout (as per spec): units length + units, value length + value
            uint32_t units_len{};
            if (!read_u32_be(p, units_len)) return false;
            if (!read_bytes(p, units_len, out.units)) return false;

            uint32_t value_len{};
            if (!read_u32_be(p, value_len)) return false;
            if (!read_bytes(p, value_len, out.value)) return false;

            return true;
        }
    };

    // Parse one or more top-level nodes (module_data may contain a single container/value or a sequence)
    std::vector<Node> tops;
    {
        const uint8_t* p = begin;
        while (p < end) {
            Node n;
            if (!parse_node(p, n)) {
                // Corrupted MDTP — show a small error component.
                return Renderer([] { return text("Corrupted MDTP payload"); });
            }
            tops.emplace_back(std::move(n));
        }
    }

    // ----- Build FTXUI from parsed nodes (allocate strings lazily for rendering) -----
    std::function<Component(const Node&)> build_ui = [&](const Node& node) -> Component {
        if (node.kind == NodeKind::Value) {
            // Prepare label/value/units lazily
            std::string name  = std::string(node.name.sv());
            std::string value = std::string(node.value.sv());
            std::string units = std::string(node.units.sv());

            // Display N/A if value is empty
            if (value.empty()) value = "N/A";

            // Determine color and alarm flag
            Color text_color(Color::RGB(255, 255, 255));
            bool  alarm_flag = false;

            if (units == "%") {
                int number_value = 0;
                auto sv = std::string_view(value);
                auto* b = sv.data();
                auto* e = sv.data() + sv.size();
                std::from_chars_result r = std::from_chars(b, e, number_value);
                if (r.ec == std::errc{}) {
                    if (number_value >= 40 && number_value < 70) {
                        text_color = Color::RGB(255, 255, 0);
                        alarm_flag = true;
                    } else if (number_value >= 70 && number_value <= 100) {
                        text_color = Color::RGB(255, 0, 0);
                        alarm_flag = true;
                    }
                }
            }

            return Renderer([name = std::move(name),
                             value = std::move(value),
                             units = std::move(units),
                             alarm_flag,
                             text_color] {
                auto line = text(std::format("{}: {} {}", name, value, units)) | color(text_color);
                if (alarm_flag) line |= inverted;
                return line;
            });
        }

        // Container: render children vertically; show a window only if the container has a name
        std::vector<Component> items;
        items.reserve(node.children.size());
        for (const auto& ch : node.children) {
            items.push_back(build_ui(ch));
        }
        Component body = Container::Vertical(std::move(items));

        if (node.name.empty()) {
            // Anonymous container (e.g., root-like) — return children as is
            return body;
        }

        std::string title = std::string(node.name.sv());
        return Renderer(body, [title = std::move(title), body] {
            return hbox({
                window(text(title) | color(Color::RGB(0, 0, 255)), body->Render()),
                filler(),
            });
        });
    };

    // If there are multiple top-level nodes, wrap them into a vertical container
    std::vector<Component> roots;
    roots.reserve(tops.size());
    for (const auto& n : tops) {
        roots.push_back(build_ui(n));
    }
    return Container::Vertical(std::move(roots));
}




// Private method
std::vector<std::string> UI::get_module_names_from_mdtp(const std::vector<uint8_t>& mdtp_data) {
    std::vector<std::string> result;

    std::size_t current = 0; // current byte
    current += 5;            // skip version and payload size

    for (; current < mdtp_data.size();) {
        // If node type is container
        if (mdtp_data[current] == 0) {
            uint32_t name_length = read_uint32_be(mdtp_data, current);
            current += 4;
            // Read name
            result.push_back(
                std::string(mdtp_data.data() + current, mdtp_data.data() + current + name_length));
        }
    }
}




// Private method
uint32_t UI::read_uint32_be(std::span<const uint8_t> memory, std::size_t offset) {
    return (static_cast<uint32_t>(memory[offset]) << 24) |
           (static_cast<uint32_t>(memory[offset + 1]) << 16) |
           (static_cast<uint32_t>(memory[offset + 2]) << 8) |
           (static_cast<uint32_t>(memory[offset + 3]));
}


} // namespace smu
