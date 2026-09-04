#include "ArenaAllocator.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <new>

// In debug builds, freed memory gets overwritten with this byte so
// that any code still reading it after reset()/rollback() shows an
// obviously wrong value instead of silently working by accident.
#ifndef NDEBUG
constexpr uint8_t ARENA_FILL_BYTE = 0xDE;
#endif

ArenaAllocator::ArenaAllocator(size_t sizeBytes, const char *debugName)
    : m_base(static_cast<uint8_t *>(::operator new(sizeBytes, std::nothrow))),
      m_capacity(sizeBytes), m_offset(0), m_debugName(debugName) {
  assert(m_base != nullptr &&
         "ArenaAllocator: couldn't get memory from the system");
}

ArenaAllocator::~ArenaAllocator() { ::operator delete(m_base); }

ArenaAllocator::ArenaAllocator(ArenaAllocator &&other) noexcept
    : m_base(other.m_base), m_capacity(other.m_capacity),
      m_offset(other.m_offset), m_debugName(other.m_debugName) {
  other.m_base = nullptr;
  other.m_capacity = 0;
  other.m_offset = 0;
}

ArenaAllocator &ArenaAllocator::operator=(ArenaAllocator &&other) noexcept {
  if (this != &other) {
    ::operator delete(m_base);

    m_base = other.m_base;
    m_capacity = other.m_capacity;
    m_offset = other.m_offset;
    m_debugName = other.m_debugName;

    other.m_base = nullptr;
    other.m_capacity = 0;
    other.m_offset = 0;
  }
  return *this;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void *ArenaAllocator::allocate(size_t size, size_t alignment) {
  const auto currentAddr = std::bit_cast<uintptr_t>(m_base + m_offset);
  const uintptr_t alignedAddr = align_up(currentAddr, alignment);
  const size_t padding = alignedAddr - currentAddr;

  if (m_offset + padding + size > m_capacity) {
    std::cerr << '[' << m_debugName << "] out of memory: requested " << size
              << " bytes, " << m_offset << " / " << m_capacity << " used\n";
    return nullptr;
  }

  m_offset += padding;
  uint8_t *result = m_base + m_offset;
  m_offset += size;

  return result;
}

void ArenaAllocator::reset() {
#ifndef NDEBUG
  std::fill_n(m_base, m_offset, ARENA_FILL_BYTE);
#endif
  m_offset = 0;
}

void ArenaAllocator::rollback(ArenaMarker marker) {
  assert(marker.offset <= m_offset &&
         "rollback: marker is ahead of current position — did you reset() "
         "after taking it?");

#ifndef NDEBUG
  std::fill_n(m_base + marker.offset, m_offset - marker.offset,
              ARENA_FILL_BYTE);
#endif

  m_offset = marker.offset;
}