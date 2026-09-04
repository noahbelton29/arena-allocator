#pragma once
#include <cstddef>

// Binary size literals, 1_KiB is 1024 bytes, not 1000 etc.
constexpr size_t operator""_B(unsigned long long n)   { return static_cast<size_t>(n); }
constexpr size_t operator""_KiB(unsigned long long n)  { return static_cast<size_t>(n) * 1024; }
constexpr size_t operator""_MiB(unsigned long long n)  { return static_cast<size_t>(n) * 1024 * 1024; }
constexpr size_t operator""_GiB(unsigned long long n)  { return static_cast<size_t>(n) * 1024 * 1024 * 1024; }