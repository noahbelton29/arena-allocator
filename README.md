# ArenaAllocator

A small, dependency-free arena (bump) allocator for C++20.

Instead of calling `malloc`/`new` for every single object, you reserve one block of memory up front. Then you hand out pieces of it by moving a pointer forward. Freeing works in bulk. You don't free objects one at a time. You free everything at once, either by resetting the whole arena or by rolling back to an earlier point.

## Why

General-purpose allocators like `malloc` have to handle any size, from any thread, freed in any order. That means real bookkeeping work on every call. Most engine code doesn't actually need that flexibility. A lot of memory gets allocated in clear, predictable batches that all die together: everything used for one frame, or everything loaded for one level. An arena is built for exactly that pattern. Allocation is just pointer arithmetic (fast, no bookkeeping), and freeing an entire batch is a single operation instead of tracking down every individual piece.

## Usage

```cpp
#include "ArenaAllocator.hpp"
#include "SizeUnits.hpp"

struct Particle {
    float x, y;
    Particle(float x, float y) : x(x), y(y) {}
};

ArenaAllocator arena(4_MiB, "FrameArena");

// allocate real, constructed objects
Particle* p = arena_new<Particle>(arena, 1.0f, 2.0f);

// scoped temporary allocations
ArenaMarker mark = arena.mark();
Particle* temp = arena_new<Particle>(arena, 0.0f, 0.0f);
arena.rollback(mark); // frees temp, leaves p untouched

// free everything at once
arena.reset();
```

## API

| Function | What it does |
|---|---|
| `ArenaAllocator(size, name)` | Reserves `size` bytes up front. `name` is used in error messages. |
| `allocate(size, alignment)` | Returns a pointer to `size` bytes, aligned to `alignment`. Returns `nullptr` if there's not enough space left. |
| `arena_new<T>(arena, args...)` | Constructs a real `T` in the arena using its constructor. `T` must be trivially destructible, see Limitations. |
| `arena_new_array<T>(arena, count)` | Reserves space for `count` `T`s as a contiguous array. Doesn't run constructors, for plain data only. |
| `mark()` | Takes a snapshot of the current position. Returns an `ArenaMarker`. |
| `rollback(marker)` | Frees everything allocated since `marker` was taken. Anything allocated before it stays untouched. |
| `reset()` | Frees everything in the arena at once. |
| `capacity()` / `offset()` | Total size / bytes currently in use. |

## Limitations (by design)

- **No individual free.** You can't free one allocation without also freeing everything after it (via `rollback`) or everything in the arena (via `reset`). If you need individual objects with independent lifetimes, this isn't the right tool. Reach for a pool allocator or the general-purpose allocator instead.
- **Destructors never run automatically.** `arena_new<T>` refuses to compile for types with a non-trivial destructor, specifically so this limitation can't be silently forgotten.
- **Not thread-safe.** Each arena is meant to be used from a single thread, or externally synchronized if shared.
- **Fixed size.** An arena doesn't grow. If it runs out of space, `allocate` returns `nullptr` instead of resizing.

## Debug behavior

In debug builds (when `NDEBUG` isn't defined), `reset()` and `rollback()` overwrite freed memory with a recognizable byte pattern (`0xDE`). If code accidentally reads from memory after it's been freed, this makes the bug obvious (garbage values) instead of it silently appearing to work. This has no effect on release builds.

## Building

```bash
cmake -B build
cmake --build build
./build/demo
./build/tests
```

## What I learned building this

Built while learning C++ memory management from scratch. Covers manual pointer arithmetic, alignment and padding, placement new vs. regular `new`, RAII, and the tradeoffs between allocation strategies (arena vs. pool vs. general-purpose). Written for use in a custom Vulkan/C++ game engine, where per-frame and per-level scoped allocations are a natural fit for this pattern.