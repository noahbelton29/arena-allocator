#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>
#include <type_traits>

// Rounds addr up to the next multiple of alignment.
// alignment must be a power of two (1, 2, 4, 8, 16, 32...).
inline uintptr_t align_up(uintptr_t addr, size_t alignment) {
    assert(alignment > 0 && (alignment & (alignment - 1)) == 0 &&
           "alignment must be a power of two");
    return (addr + (alignment - 1)) & ~(alignment - 1);
}

// A snapshot of an arena's position at some point in time.
// Used with ArenaAllocator::mark() / rollback() to free everything
// allocated since the snapshot was taken, without resetting the
// whole arena.
struct ArenaMarker {
    size_t offset;
};


// A simple allocator that hands out memory by moving a pointer forward.
// You can't free one thing at a time, you free everything at once,
// either by calling reset() or by destroying the arena.
// Good for short-lived stuff that all dies together, like data used
// for just one frame, or data used while loading a level.
class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t sizeBytes, const char* debugName = "Arena");
    ~ArenaAllocator();

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    // Gives back a pointer to `size` bytes, aligned correctly.
    // Returns nullptr if there's not enough space left.
    [[nodiscard]] void* allocate(size_t size, size_t alignment);

    // Marks all previous allocations as free. Doesn't touch the
    // underlying memory block itself, just moves the marker back to 0.
    void reset();

    // Takes a snapshot of the current position. Everything allocated
    // AFTER this call can later be freed by calling rollback(marker),
    // without touching anything allocated BEFORE the marker.
    [[nodiscard]] ArenaMarker mark() const { return { m_offset }; }

    // Frees everything allocated since `marker` was taken.
    // marker must have come from THIS arena, and must not be older
    // than the arena's current contents (e.g. don't rollback to a
    // marker taken before a reset()).
    void rollback(ArenaMarker marker);

    size_t capacity() const { return m_capacity; }
    size_t offset()   const { return m_offset; }
    const char* name() const { return m_debugName; }

private:
    uint8_t* m_base; // start of block of memory
    size_t   m_capacity; // total size of that block
    size_t   m_offset; // how much of it we've handed out so far
    const char* m_debugName;
};

// Builds a real object of type T inside the arena, using placement new.
// Returns nullptr if the arena is out of space.
template <typename T, typename... Args>
[[nodiscard]] T* arena_new(ArenaAllocator& arena, Args&&... args) {
    static_assert(std::is_trivially_destructible_v<T>,
        "arena_new only supports trivially destructible types, "
        "the arena never calls destructors. If T needs cleanup, "
        "don't allocate it in an arena.");

    void* mem = arena.allocate(sizeof(T), alignof(T));
    if (!mem) {
        return nullptr;
    }
    return new (mem) T(std::forward<Args>(args)...);
}

// Allocates space for `count` trivially-constructible objects of type T,
// as a contiguous array. Does NOT run constructors, intended for plain
// data (float, int, small structs with no meaningful constructor).
// Returns nullptr if the arena is out of space.
template <typename T>
[[nodiscard]] T* arena_new_array(ArenaAllocator& arena, size_t count) {
    static_assert(std::is_trivially_constructible_v<T>,
        "arena_new_array only supports trivially constructible types.");

    void* mem = arena.allocate(sizeof(T) * count, alignof(T));
    return static_cast<T*>(mem);
}