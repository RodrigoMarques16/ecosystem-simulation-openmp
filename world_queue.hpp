#pragma once

/**
 * world_queue.hpp
 *
 * Synchronizes (or attempts to) the simulation by assigning each row of the grid a
 * queue of entities trying to move to it.
 *
 * Moves are stored in an temporary grid to prevent moves affecting future
 * computations.
 * 
 * Each update is divided in 6 steps:
 *
 *     1) Iterate rabbits and find their moves:
 *        If the rabbit stays in the same row, update it instantly
 *        If it moves to a different row, push the move into that row's queue
 *
 *     2) Iterate through each row and apply any attemps to move in its queue. Conflict resolution is applied in this step.
 *     
 *     3) Replace the map with the temporary map
 *
 *     4, 5, 6) Repeat for foxes
 */

#ifndef DEBUG
#define DEBUG 0
#endif

#include <string>
#include <vector>
#include <tuple>

#include "concurrentvector.hpp"
#include "debug.hpp"
#include "entity.hpp"
#include "matrix.hpp"
#include "omp.h"

const size_t NTHREADS = 16;

enum Direction { NORTH, EAST, SOUTH, WEST, INPLACE };

constexpr uint8_t DIRECTIONS_N = 4;

struct World {
    using Move = std::tuple<int, int, Entity>;

    int GEN_PROC_RABBITS;  // Number of generations until a rabbit can procrate
    int GEN_PROC_FOXES;    // As above but for foxes
    int GEN_FOOD_FOXES;    // How many generations a fox can go without food
    int N_GEN;             // How many generations the world will last

    int current_gen;
    int entity_count;
    int height;
    int width;
    
    Matrix<Entity> map;
    Matrix<Entity> nextMap;

    std::vector<int> owner;
    std::vector<ConcurrentVector<Move>> sync;

    World() = delete;
    World(int gen_proc_rabbits, int gen_proc_foxes, int gen_food_foxes,
          int n_gen, int w, int h, int count)
        : GEN_PROC_RABBITS(gen_proc_rabbits),
          GEN_PROC_FOXES(gen_proc_foxes),
          GEN_FOOD_FOXES(gen_food_foxes),
          N_GEN(n_gen),
          current_gen(0),
          entity_count(count),
          height(h + 1),
          width(w + 1),
          map(Matrix<Entity>(h + 2, w + 2)),
          nextMap(Matrix<Entity>(h + 2, w + 2)),
          owner(std::vector<int>(height)),
          sync(NTHREADS) {}

    void init();
    void update();
    inline void updateRabbits();
    inline void updateFoxes();
    void updateRabbit(Entity, int, int);
    void updateFox(Entity, int, int);
    inline void clearQueues();

    inline int selectDirection(int, int, int) const;
    inline void updateCoords(Direction, int&, int&) const;
    inline bool getRabbitMove(int&, int&) const;
    inline bool getFoxMove(int&, int&) const;

    inline bool resolveConflictRabbit(const Entity&, const Entity) const;
    inline bool resolveConflictFox(const Entity&, const Entity) const;

    inline void add(const std::string_view, const int, const int);

    inline bool canMove(const Entity&, const Entity&) const;
    inline bool hasStarved(const Entity&) const;

    int countEntities() const;

    void print() const;
    void printText() const;
};

void World::init() {
    const Entity ent = makeEntity(ROCK);
    for (int i = 0; i != height + 1; ++i) {
        map(i, 0)      = nextMap(i, 0)      = ent;
        map(i, width)  = nextMap(i, width)  = ent;
        map(0, i)      = nextMap(0, i)      = ent;
        map(height, i) = nextMap(height, i) = ent;
    }

    #pragma omp parallel for num_threads(NTHREADS)
    for(int i = 1; i != height; ++i) {
        owner[i] = omp_get_thread_num();
    }
}

inline void World::clearQueues() {
    for (int th = 0; th < NTHREADS; ++th) 
        sync[th].clear();
}

void World::update() {
    updateRabbits();

    #pragma parallel for num_threads(NTHREADS)
    for (int th = 0; th < NTHREADS; ++th) {
        for (int i = 0; i < sync[th].size(); ++i) {
            auto [x, y, next] = sync[th][i];
            if (resolveConflictRabbit(next, nextMap(x, y)))
                nextMap(x, y) = next;
        }
    }

    clearQueues();
    map = nextMap;
    updateFoxes();

    #pragma parallel for num_threads(NTHREADS)
    for (int th = 0; th < NTHREADS; ++th) {
        for (int i = 0; i < sync[th].size(); ++i) {
            auto [x, y, next] = sync[th][i];
            if (resolveConflictFox(next, nextMap(x, y)))
                nextMap(x, y) = next;
        }
    }

    clearQueues();
    map = nextMap;
    current_gen++;
}

void World::updateRabbits() {
    #pragma omp parallel for num_threads(NTHREADS)
    for (int i = 1; i < height; ++i)
        for (int j = 1; j < width; ++j)
            if (map(i, j).type == RABBIT)
                updateRabbit(map(i, j), i, j);
}

void World::updateFoxes() {
    #pragma omp parallel for num_threads(NTHREADS)
    for (int i = 1; i < height; ++i)
        for (int j = 1; j < width; ++j)
            if (map(i, j).type == FOX)
                updateFox(map(i, j), i, j);
}

void World::updateRabbit(Entity ent, int x, int y) {
    dbg::LOGLN("\nRabbit (%d,%d)", x, y);
    int oldX = x, oldY = y;

    ++ent.age;
    dbg::LOGLN("Age: %d", ent.age);

    if (getRabbitMove(x, y) == false) {
        dbg::LOGLN("Staying still");
        nextMap(oldX, oldY) = ent;
        return;
    }

    dbg::LOGLN("Moving to (%d,%d)", x, y);

    // if (resolveConflictRabbit(ent, nextMap(x, y)) == false) {
    //     dbg::LOGLN("Died attacking");
    // }

    if (ent.age > GEN_PROC_RABBITS) {
        ent.age = 0;
        nextMap(oldX, oldY) = {RABBIT};
    } else {
        nextMap(oldX, oldY) = {EMPTY};
    }
    
    int th = omp_get_thread_num();
    if (th == owner[x]) {
        if (resolveConflictRabbit(ent, nextMap(x, y)))
            nextMap(x, y) = ent;
    } else
        sync[th].push_back({x, y, ent});
}

void World::updateFox(Entity ent, int x, int y) {
    dbg::LOGLN("\nFox (%d,%d)", x, y);
    int oldX = x, oldY = y;

    ++ent.age;
    dbg::LOGLN("Age: %d", ent.age);
    dbg::LOGLN("Hunger: %d", ent.hunger + 1);

    if (getFoxMove(x, y) == true) {
        dbg::LOGLN("Ate rabbit");
        ent.hunger = 0;
    } else {
        ++ent.hunger;
        if (ent.hunger >= GEN_FOOD_FOXES) {
            dbg::LOGLN("Starved");
            nextMap(oldX, oldY) = {EMPTY};
            return;
        }
        if (getRabbitMove(x, y) == false) {
            dbg::LOGLN("Staying still");
            nextMap(oldX, oldY) = ent;
            return;
        }
    }

    dbg::LOGLN("Moving to (%d,%d)", x, y);

    if (ent.age > GEN_PROC_FOXES) {
        ent.age = 0;
        nextMap(oldX, oldY) = {FOX};
    } else {
        nextMap(oldX, oldY) = {EMPTY};
    }

    int th = omp_get_thread_num();
    if (th == owner[x]) {
        if (resolveConflictFox(ent, nextMap(x, y)))
            nextMap(x, y) = ent;
    } else {
        sync[th].push_back({x, y, ent});
    }
}

inline void World::add(const std::string_view e, const int x, const int y) {
    map(x + 1, y + 1) = nextMap(x + 1, y + 1) = makeEntity(e);
}

inline bool World::getRabbitMove(int& x, int& y) const {
    Direction arr[4];
    int dirs = 0;
    if (map(x - 1, y).type == EMPTY) arr[dirs++] = NORTH;
    if (map(x, y + 1).type == EMPTY) arr[dirs++] = EAST;
    if (map(x + 1, y).type == EMPTY) arr[dirs++] = SOUTH;
    if (map(x, y - 1).type == EMPTY) arr[dirs++] = WEST;
    if (dirs > 0) {
        int rnd = selectDirection(x, y, dirs);
        updateCoords(arr[rnd], x, y);
        return true;
    }
    return false;
}

inline bool World::getFoxMove(int& x, int& y) const {
    Direction arr[4];
    int dirs = 0;
    if (map(x - 1, y).type == RABBIT) arr[dirs++] = NORTH;
    if (map(x, y + 1).type == RABBIT) arr[dirs++] = EAST;
    if (map(x + 1, y).type == RABBIT) arr[dirs++] = SOUTH;
    if (map(x, y - 1).type == RABBIT) arr[dirs++] = WEST;
    if (dirs > 0) {
        int rnd = selectDirection(x, y, dirs);
        updateCoords(arr[rnd], x, y);
        return true;
    }
    return false;
}

inline void World::updateCoords(Direction dir, int& x, int& y) const {
    switch (dir) {
        case NORTH: x = x - 1; break;
        case EAST:  y = y + 1; break;
        case SOUTH: x = x + 1; break;
        case WEST:  y = y - 1; break;
        case INPLACE: break;
    }
}

inline int World::selectDirection(int x, int y, int ndirs) const {
    return (x + y - 2 + current_gen) % (ndirs);
}

inline bool World::resolveConflictRabbit(const Entity& a, const Entity b) const {
    return b.type == EMPTY || a.age > b.age;
}

inline bool World::resolveConflictFox(const Entity& a, const Entity b) const {
    return b.type == RABBIT || b.type == EMPTY || a.age > b.age || (a.age == b.age && a.hunger < b.hunger);
}

int World::countEntities() const {
    int k = 0;
    for (int i = 1; i != height; ++i) {
        for (int j = 1; j != width; ++j) {
            auto ent = map(i, j).type;
            if (ent != Entity_t::EMPTY) k++;
        }
    }
    return k;
}

void World::print() const {
    for (int i = 0; i < width + 1; ++i) std::cout << "-";
    std::cout << '\n';
    for (int i = 1; i < height; ++i) {
        std::cout << '|';
        for (int j = 1; j < width; ++j) {
            printEntity(map(i, j).type);
        }
        std::cout << "|\n";
    }
    for (int i = 0; i < width + 1; ++i) std::cout << "-";
    std::cout << '\n';
}

void World::printText() const {
    for (int i = 1; i < height; ++i) {
        for (int j = 1; j < width; ++j) {
            if (map(i, j).type != EMPTY)
                std::cout << entityName(map(i, j).type) << ' ' << i - 1 << ' '
                          << j - 1 << '\n';
        }
    }
}