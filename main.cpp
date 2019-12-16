#define DEBUG 0

#include <omp.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <vector>

#ifdef _OPENMP
#include "omp.h"
#include "world_queue.hpp"
#else
#include "world_sequential.hpp"
#endif

#include "debug.hpp"
#include "entity.hpp"
#include "malloc.h"
#include "matrix.hpp"

using namespace std::chrono;

void readEntities(World& world) {
    for (int i = 0; i != world.entity_count; ++i) {
        int x, y;
        std::string o;
        std::cin >> o >> x >> y;
        world.add(o, x, y);
    }
}

int main() {
    if constexpr (DEBUG == 0) std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    int gen_proc_rabbits, gen_proc_foxes, gen_food_foxes;
    int height, width, count, n_gen;

    std::cin >> gen_proc_rabbits >> gen_proc_foxes >> gen_food_foxes >> n_gen >>
        width >> height >> count;

    World world(gen_proc_rabbits, gen_proc_foxes, gen_food_foxes, n_gen, width,
                height, count);

    world.init();
    readEntities(world);

    auto t1 = high_resolution_clock::now();

    while (n_gen--) 
        world.update();

    auto t2 = high_resolution_clock::now();

    std::cerr << duration_cast<microseconds>(t2 - t1).count() << "μs, "
              << duration_cast<milliseconds>(t2 - t1).count() << "ms, "
              << duration_cast<seconds>(t2 - t1).count()      << "s\n";

    std::cout << gen_proc_rabbits << ' ' << gen_proc_foxes << ' '
              << gen_food_foxes   << ' ' << 0              << ' ' 
              << height           << ' ' << width          << ' '
              << world.countEntities()   << '\n';

    world.printText();

    return 0;
}
