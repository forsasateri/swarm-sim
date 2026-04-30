#ifndef COORDINATE_SYSTEM_HPP
#define COORDINATE_SYSTEM_HPP

#include <SFML/System.hpp>
#include <cstdint>

constexpr uint32_t BLOCK_SIZE_X = 25;
constexpr uint32_t BLOCK_SIZE_Y = 25;

constexpr uint32_t WORLD_SIZE_X = 2000;
constexpr uint32_t WORLD_SIZE_Y = 1000;

constexpr uint32_t GRID_SIZE_X = WORLD_SIZE_X / BLOCK_SIZE_X;
constexpr uint32_t GRID_SIZE_Y = WORLD_SIZE_Y / BLOCK_SIZE_Y;


sf::Vector2i worldToGrid(sf::Vector2f worldPos) {
    return sf::Vector2i(
        static_cast<int>(worldPos.x) / BLOCK_SIZE_X,
        static_cast<int>(worldPos.y) / BLOCK_SIZE_Y
    );
}

sf::Vector2f gridToWorld(int x, int y) {
    return sf::Vector2f(
        x * BLOCK_SIZE_X + BLOCK_SIZE_X / 2.f,
        y * BLOCK_SIZE_Y + BLOCK_SIZE_Y / 2.f
    );
}

sf::Vector2f gridToWorld(sf::Vector2i gridPos) {
    return gridToWorld(gridPos.x, gridPos.y);
}

#endif // COORDINATE_SYSTEM_HPP