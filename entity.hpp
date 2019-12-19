#pragma once

#include <stddef.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std::literals;

enum Entity_t : uint8_t { EMPTY, RABBIT, FOX, ROCK };
constexpr size_t ENTITY_TYPES_N = 4;

struct Entity {
    Entity_t type;  // Which type of entity is in this cell
    short age;      // How many generations it has lived
    short hunger;   // How many generations since it last ate
};

 std::array<std::string, ENTITY_TYPES_N> ENTITY_NAME = {
    "EMPTY", "RABBIT", "FOX", "ROCK"};

 std::array<char, ENTITY_TYPES_N> ENTITY_SYMBOL = {
    ' ', 'R', 'F', '*'};

inline  Entity makeEntity(const Entity_t t) {
    return {t, 0, 0};
}

inline Entity makeEntity(const std::string s) {
    if (s == "ROCK")
        return makeEntity(Entity_t::ROCK);
    else if (s == "RABBIT")
        return makeEntity(Entity_t::RABBIT);
    else if (s == "FOX")
        return makeEntity(Entity_t::FOX);
    return makeEntity(Entity_t::EMPTY);
}

inline  std::string entityName(const Entity& e) {
    return ENTITY_NAME[static_cast<size_t>(e.type)];
}

inline  std::string entityName(const Entity_t e) {
    return ENTITY_NAME[static_cast<size_t>(e)];
}

inline void printEntity(const Entity& e) {
    std::cout << ENTITY_SYMBOL[static_cast<size_t>(e.type)];
}

inline void printEntity(const Entity_t e) {
    std::cout << ENTITY_SYMBOL[static_cast<size_t>(e)];
}