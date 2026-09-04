#include "ArenaAllocator.hpp"
#include <cassert>
#include <iostream>

void test_basic_allocation() {
    ArenaAllocator arena(1024, "Test");
    void* p1 = arena.allocate(50, 8);
    void* p2 = arena.allocate(30, 8);
    assert(p1 != nullptr);
    assert(p2 != nullptr);
    assert(p2 > p1);
    std::cout << "test_basic_allocation passed\n";
}

void test_alignment() {
    ArenaAllocator arena(1024, "Test");
    void* p = arena.allocate(1, 16);
    assert(reinterpret_cast<uintptr_t>(p) % 16 == 0);
    std::cout << "test_alignment passed\n";
}

void test_out_of_space_returns_nullptr() {
    ArenaAllocator arena(16, "Test");
    void* p = arena.allocate(1000, 8);
    assert(p == nullptr);
    std::cout << "test_out_of_space_returns_nullptr passed\n";
}

void test_reset_reuses_memory() {
    ArenaAllocator arena(1024, "Test");
    void* p1 = arena.allocate(50, 8);
    arena.reset();
    void* p2 = arena.allocate(50, 8);
    assert(p1 == p2);
    std::cout << "test_reset_reuses_memory passed\n";
}

void test_marker_rollback() {
    ArenaAllocator arena(1024, "Test");
    void* before = arena.allocate(10, 8);
    ArenaMarker mark = arena.mark();
    arena.allocate(100, 8);
    arena.rollback(mark);
    void* after = arena.allocate(10, 8);
    assert(after > before);
    std::cout << "test_marker_rollback passed\n";
}

void test_exact_capacity_fits() {
    ArenaAllocator arena(64, "Test");
    void* p = arena.allocate(64, 1); // no alignment padding, exact fit
    assert(p != nullptr);
    void* overflow = arena.allocate(1, 1); // one byte over, should fail
    assert(overflow == nullptr);
    std::cout << "test_exact_capacity_fits passed\n";
}

void test_zero_size_allocation() {
    ArenaAllocator arena(64, "Test");
    void* p = arena.allocate(0, 8);
    assert(p != nullptr); // valid pointer, just claims no bytes
    assert(arena.offset() == 0);
    std::cout << "test_zero_size_allocation passed\n";
}

int main() {
    test_basic_allocation();
    test_alignment();
    test_out_of_space_returns_nullptr();
    test_reset_reuses_memory();
    test_marker_rollback();
    test_exact_capacity_fits();
    test_zero_size_allocation();
    std::cout << "\nAll tests passed.\n";
    return 0;
}