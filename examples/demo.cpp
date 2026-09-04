#include "ArenaAllocator.hpp"
#include "SizeUnits.hpp"

#include <iostream>

struct Vec3 {
  float x, y, z;
  Vec3(float x, float y, float z) : x(x), y(y), z(z) {
    std::cout << "  Vec3 constructed at " << this << " (" << x << ", " << y
              << ", " << z << ")\n";
  }
};

struct Particle {
  float px, py;
  float lifetime;

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  Particle(float px, float py, float lifetime)
      : px(px), py(py), lifetime(lifetime) {}
};

int main() {
  ArenaAllocator arena(1_MiB, "DemoArena");
  std::cout << "arena capacity: " << arena.capacity() << " bytes\n\n";

  std::cout << "[1] allocating a permanent Vec3\n";
  Vec3 *origin = arena_new<Vec3>(arena, 0.0F, 0.0F, 0.0F);
  std::cout << "origin = " << origin << ", offset now = " << arena.offset()
            << "\n\n";

  std::cout << "[2] marking arena, then allocating temp particles\n";
  ArenaMarker beforeParticles = arena.mark();
  for (int i = 0; i < 5; ++i) {
    Particle *p = arena_new<Particle>(arena, float(i), float(i) * 2, 1.0F);
    std::cout << "  particle " << i << " at " << p << "\n";
  }
  std::cout << "offset after particles = " << arena.offset() << "\n\n";

  std::cout << "[3] rolling back to before the particles\n";
  arena.rollback(beforeParticles);
  std::cout << "offset after rollback = " << arena.offset() << "\n";
  std::cout << "origin->x is still valid: " << origin->x << "\n\n";

  std::cout << "[4] full reset\n";
  arena.reset();
  std::cout << "offset after reset = " << arena.offset() << "\n\n";

  std::cout << "[5] deliberately requesting more than a tiny arena has\n";
  ArenaAllocator tinyArena(16_B, "TinyArena");
  Vec3 *tooBig = arena_new<Vec3>(tinyArena, 1.0F, 1.0F, 1.0F);
  if (tooBig == nullptr) {
    std::cout << "allocation correctly failed (nullptr returned)\n";
  }

  return 0;
}