[Inspired by this document](https://github.com/cpp-best-practices/cppbestpractices/blob/master/03-Style.md?plain=1)

# Code Style

This project uses the following naming conventions:

* Classes and types: `PascalCase`
* Functions and methods: `snake_case`
* Variables: `snake_case`
* Constants: `ALL_UPPER_CASE`
* Member variables: prefixed with `m_`

## General Rules

* Be consistent.
* Prefer readable, descriptive names over terse ones.
* Don't start any identifier with `_` (reserved).
* Use `nullptr` instead of `NULL` or `0`.
* Use `//` comments.
* Never use `using namespace` in headers.
* Use `#pragma once` for include guards.
* Use `{}` for all code blocks, even one-liners.
* Keep lines under 100 characters.
* Use `"filename.hpp"` for local includes.
* Use member initializer lists.
* Use `const` where possible.
* Avoid polluting the global namespace.
* Use `std::size_t` for sizes and indexes.
* File extensions: `.hpp` and `.cpp`

## Doxygen Comments

Use Doxygen-style comments for all public classes and functions:

```cpp
/**
 * @brief Short description of the class.
 *
 * Longer explanation if necessary.
 */
class MyClass
{
  // ...
};

/**
 * @brief Adds two integers.
 *
 * @param a First operand
 * @param b Second operand
 * @return Sum of a and b
 */
int add(int a, int b);
```

## Class Example

```cpp
class MyClass
{
public:
  MyClass(int value)
    : m_value{value}
  {
  }

  int get_value() const;

private:
  const int m_value{0};
};
```

## Function Example

```cpp
int calculate_sum(int a, int b)
{
  return a + b;
}
```

## Member Initialization

Use brace initialization:

```cpp
class AnotherClass
{
public:
  AnotherClass()
    : m_count{0}
  {
  }

private:
  std::size_t m_count{0};
};
```

## Control Structures

Always use braces:

```cpp
for (int i = 0; i < 10; ++i) {
  std::cout << i << std::endl;
}
```

## Comments

Use `//` for internal comments:

```cpp
int my_func()
{
  return 42; // return 42
}
```

## Includes

```cpp
#include <vector>
#include "my_header.hpp"
```

## Include Guards

```cpp
#pragma once
```


