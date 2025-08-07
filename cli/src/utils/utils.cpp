/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file utils/utils.cpp
 * @brief File with helper functions
 */

#include "utils/utils.hpp"
#include "version.hpp"
#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <poll.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>
#include <compile-time_config.hpp>

namespace smu_cli {

// ============ parse_own_arguments ============
bool parse_own_arguments(int argc, char** argv) noexcept {
    cxxopts::Options options("smu-cli",
                             "CLI part to manage smu-server included in server-monitoring-utility");

    options.add_options()("version", "Show smu-cli version");
    options.add_options()("h,help", "Show help information");
    options.add_options()("keygen",
                          "Generate and save an X509 certificate and private key for the server");

    // Add a fake command name (should be "-list commands"), since cxxopts does not support spaces
    // in the command name. When outputting help, "--list-commands" will be replaced by "--list
    // commands"
    options.add_options()("list-commands", "Show the commands available to query the smu-server");

    cxxopts::ParseResult result;

    try {
        result = options.parse(argc, argv);
    } catch (const std::exception& e) {
        return false; // not parsed
    }

    if (result.contains("version")) {
        std::cout << std::format("Version is {}.{}.{}",
                                 PROJECT_VERSION_MAJOR,
                                 PROJECT_VERSION_MINOR,
                                 PROJECT_VERSION_PATCH)
                  << std::endl;
        return true; // parsed
    }

    if (result.contains("help")) {
        std::ostringstream oss;
        oss << options.help();
        std::string help = oss.str();

        // Replace "--list-commands" to "--list commands". See cause upper
        size_t pos = help.find("--list-commands");
        if (pos != std::string::npos) {
            help.replace(pos, std::string("--list-commands").size(), "--list commands");
        }

        std::cout << help;

        return true; // parsed
    }

    if (result.contains("keygen")) {
        std::cout << "Generating certificate and private key..." << std::endl;
        auto keypair = generate_keypair();
        // Keygen error
        if (!keypair.has_value()) {
            std::cerr << "Error: " << keypair.error() << std::endl;
        } else {
            try {
                std::cout << "Saving certificate and private key..." << std::endl;

                write_keypair(SERVER_CTYPTO_CERTS_DIR, "certificate.crt", "privkey.key", keypair.value());

                std::cout << "Done!\n";
                std::cout << "Certificate saved to " << SERVER_CTYPTO_CERTS_DIR << "/certificate.crt\n";
                std::cout << "Private key saved to " << SERVER_CTYPTO_CERTS_DIR << "/privkey.key" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }

        return true;
    }

    // ELSE
    return false; // not parsed
}



// ============ handle_error ============
void handle_error(const char* str) noexcept {
    perror(str);
    exit(EXIT_FAILURE);
}




// ============ send_message ============
bool send_message(int socket_fd, const std::string_view str) noexcept {
    size_t  total_bytes = str.size();
    size_t  bytes_sent = 0; // bytes sent
    ssize_t res;

    while ((res = send(socket_fd, str.data() + bytes_sent, total_bytes - bytes_sent, 0)) <
           static_cast<ssize_t>(total_bytes)) {
        // if error
        if (res == -1) {
            if (!(errno == EAGAIN || errno == EINTR)) {
                // if fatal error
                return false;
            }
        }
        // Connection closed
        if (res == 0) {
            return false;
        }

        bytes_sent += static_cast<size_t>(res);
    }

    return true;
}




// ============ read_message ============
std::expected<std::string, std::string> read_message(int        socket_fd,
                                                     int        timeout_ms,
                                                     const char msg_end) noexcept {
    // Poll
    pollfd fd;
    fd.fd = socket_fd;
    fd.events = POLLIN;

    int poll_res;
    while ((poll_res = poll(&fd, 1, timeout_ms)) != 1) {
        // Error
        if (poll_res == -1) {
            // Fatal error
            if (errno != EINTR) {
                return std::unexpected(std::format("poll: {}", strerror(errno)));
            }
            // continue
        }
        // timeout
        else if (poll_res == 0) {
            return std::unexpected("Error: response timeout has been exceeded");
        }
    }

    // Error
    if (fd.revents & (POLLERR | POLLNVAL)) {
        return std::unexpected("socket error during poll");
    }

    std::string result;
    char        buffer[256];

    while (true) {
        ssize_t rd_res = recv(socket_fd, buffer, sizeof(buffer), 0);

        // Error occured
        if (rd_res == -1) {
            if (!(errno == EAGAIN || errno == EINTR)) {
                // Fatal error
                return std::unexpected(std::format("recv: {}", strerror(errno)));
            }
            // continue
        } else if (rd_res == 0) {
            // Connection closed
            break;
        } // else

        result.append(buffer, static_cast<size_t>(rd_res));

        if (auto pos = result.find(msg_end); pos != std::string::npos) {
            result.resize(pos);
            break;
        }
    }

    return result;
}




// ============ generate_keypair ============
std::expected<KeyPair, std::string> generate_keypair() {
    // Thanks to the ChatGPT! :)

    EVP_PKEY*     pkey = nullptr;
    EVP_PKEY_CTX* ctx = nullptr;
    X509*         x509 = nullptr;
    BIO*          bioKey = nullptr;
    BIO*          bioCert = nullptr;

    // Make context for rsa generation
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx)
        return std::unexpected("EVP_PKEY_CTX_new_id() failed");

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return std::unexpected("EVP_PKEY_keygen_init() failed");
    }

    // Set key length to 2048 bits
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return std::unexpected("EVP_PKEY_CTX_set_rsa_keygen_bits() failed");
    }

    // Generate key
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return std::unexpected("EVP_PKEY_keygen() failed");
    }
    EVP_PKEY_CTX_free(ctx);

    // Make x509 cert
    x509 = X509_new();
    if (!x509) {
        EVP_PKEY_free(pkey);
        return std::unexpected("X509_new() failed");
    }

    // Serial number = 1
    if (!ASN1_INTEGER_set(X509_get_serialNumber(x509), 1)) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("Setting serial number failed");
    }

    // Duration of validity is 1 year
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);

    // Paste default key
    if (X509_set_pubkey(x509, pkey) != 1) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("X509_set_pubkey() failed");
    }

    // Subject & issuer: only CN=localhost
    X509_NAME* name = X509_get_subject_name(x509);
    if (!X509_NAME_add_entry_by_txt(name,
                                    "CN",
                                    MBSTRING_ASC,
                                    reinterpret_cast<const unsigned char*>("localhost"),
                                    -1,
                                    -1,
                                    0)) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("Adding CN to certificate failed");
    }
    X509_set_issuer_name(x509, name);
    if (X509_sign(x509, pkey, EVP_sha256()) == 0) {
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("X509_sign() failed");
    }

    // Write to memory via BIO
    bioKey = BIO_new(BIO_s_mem());
    bioCert = BIO_new(BIO_s_mem());
    if (!bioKey || !bioCert) {
        if (bioKey)
            BIO_free(bioKey);
        if (bioCert)
            BIO_free(bioCert);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("BIO_new() failed");
    }
    if (!PEM_write_bio_PrivateKey(bioKey, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
        BIO_free(bioKey);
        BIO_free(bioCert);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("Writing private key to BIO failed");
    }
    if (!PEM_write_bio_X509(bioCert, x509)) {
        BIO_free(bioKey);
        BIO_free(bioCert);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("Writing certificate to BIO failed");
    }

    // Read from BIO to string
    char* data = nullptr;
    long  len = BIO_get_mem_data(bioKey, &data);
    if (len <= 0) {
        BIO_free(bioKey);
        BIO_free(bioCert);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("Reading private key PEM failed");
    }
    std::string privPem(data, static_cast<size_t>(len));

    len = BIO_get_mem_data(bioCert, &data);
    if (len <= 0) {
        BIO_free(bioKey);
        BIO_free(bioCert);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        return std::unexpected("Reading certificate PEM failed");
    }
    std::string certPem(data, static_cast<size_t>(len));

    // Free and return
    BIO_free(bioKey);
    BIO_free(bioCert);
    X509_free(x509);
    EVP_PKEY_free(pkey);

    return KeyPair{std::move(certPem), std::move(privPem)};
}




// ============ write_keypair ============
void write_keypair(const std::string_view directory,
                   const std::string_view cert_name,
                   const std::string_view privkey_name,
                   const KeyPair&         keypair) {
    namespace fs = std::filesystem;

    const std::string cert_path = std::format("{}/{}", directory, cert_name);
    const std::string privkey_path = std::format("{}/{}", directory, privkey_name);

    // WRITING CERTIFICATE
    std::ofstream cert_ofs(cert_path, std::ios::binary | std::ios::trunc);

    if (!cert_ofs.is_open()) {
        throw std::system_error(
            errno, std::generic_category(), std::format("Failed to open '{}'", cert_path));
    }

    cert_ofs << keypair.certifiacte;
    cert_ofs.close();

    if (!cert_ofs) {
        throw std::system_error(
            errno, std::generic_category(), std::format("Failed to write to '{}'", cert_path));
    }

    // UID = 0, GID = 0
    if (chown(cert_path.c_str(), 0, 0) == -1) {
        throw std::system_error(errno,
                                std::generic_category(),
                                std::format("Failed to change owner of '{}'", cert_path));
    }

    // Change permissions
    fs::permissions(cert_path, fs::perms::owner_all);



    // WRITING PRIVATE KEY
    std::ofstream privkey_ofs(privkey_path, std::ios::binary | std::ios::trunc);

    if (!privkey_ofs.is_open()) {
        throw std::system_error(
            errno, std::generic_category(), std::format("Failed to open '{}'", privkey_path));
    }

    privkey_ofs << keypair.private_key;
    privkey_ofs.close();

    if (!privkey_ofs) {
        throw std::system_error(
            errno, std::generic_category(), std::format("Failed to write to '{}'", privkey_path));
    }

    // UID = 0, GID = 0
    if (chown(privkey_path.c_str(), 0, 0) == -1) {
        throw std::system_error(errno,
                                std::generic_category(),
                                std::format("Failed to change owner of '{}'", privkey_path));
    }

    // Change permissions
    fs::permissions(privkey_path, fs::perms::owner_all);
}


} // namespace smu_cli
