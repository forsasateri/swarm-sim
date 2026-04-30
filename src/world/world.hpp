#ifndef WORLD_HPP
#define WORLD_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <algorithm>

#include "util/matrix.hpp"
#include "block.hpp"

// Extend Vector2i for use in std::map and std::set
namespace std {
    template <>
    struct less<sf::Vector2i> {
        bool operator()(const sf::Vector2i& a, const sf::Vector2i& b) const {
            return std::tie(a.x, a.y) < std::tie(b.x, b.y);
        }
    };
}


class World {

public:
    World(sf::Vector2i size) : worldSize(size), occupancyMatrix(size.x / blockSize.x, size.y / blockSize.y) {
        // occupancyMatrix = Matrix<bool>( worldSize.x / blockSize.x, worldSize.y / blockSize.y );
        buildGridLines();
    }

    void display(sf::RenderWindow& window) {
        // Draw grid lines
        window.draw(gridLines);

        // Draw occupied blocks
        for (int x = 0; x < occupancyMatrix.getWidth(); x++) {
            for (int y = 0; y < occupancyMatrix.getHeight(); y++) {
                if (occupancyMatrix(x, y).isOccupied()) {
                    
                    sf::RectangleShape blockShape({ (float)blockSize.x, (float)blockSize.y });
                    blockShape.setPosition({x * blockSize.x, y * blockSize.y});
                    blockShape.setFillColor(sf::Color::Red);
                    window.draw(blockShape);
                }
            }
        }
    }

    void handleClick(sf::Vector2i mousePos) {
        int blockX = mousePos.x / blockSize.x;
        int blockY = mousePos.y / blockSize.y;

        occupancyMatrix(blockX, blockY).toggle();
    }


    bool isOccupied(sf::Vector2f position) {
        sf::Vector2i blockPos = getBlock(position);
        int blockX = blockPos.x;
        int blockY = blockPos.y;

        if (blockX < 0 || blockX >= occupancyMatrix.getWidth() || blockY < 0 || blockY >= occupancyMatrix.getHeight()) {
            return true; // Treat out-of-bounds as occupied
        }

        return occupancyMatrix(blockX, blockY).isOccupied();
    }

    sf::Vector2i getBlock(sf::Vector2f position) {

        // Out of bounds check
        if (position.x < 0 || position.x >= worldSize.x || position.y < 0 || position.y >= worldSize.y) {
            std::cerr << "Error: Position (" << position.x << ", " << position.y << ") is out of bounds" << std::endl;
        }

        int blockX = std::floor(position.x) / blockSize.x;
        int blockY = std::floor(position.y) / blockSize.y;

        // Additional check to ensure block coordinates are within bounds of the occupancy matrix
        if (blockX < 0 || blockX >= occupancyMatrix.getWidth() || blockY < 0 || blockY >= occupancyMatrix.getHeight()) {
            std::cerr << "Error: Calculated block coordinates (" << blockX << ", " << blockY << ") are out of bounds" << std::endl;
        }

        return { blockX, blockY };
    }

    // Find all blocks in range of a position (manhattan distance)
    std::vector<sf::Vector2i> getBlocksInRange(sf::Vector2f position, int range) {
        std::vector<sf::Vector2i> blocksInRange;

        sf::Vector2i centerBlock = getBlock(position);

        for (int dx = -range; dx <= range; dx++) {
            for (int dy = -range; dy <= range; dy++) {
                sf::Vector2i blockPos = { centerBlock.x + dx, centerBlock.y + dy };

                if (blockPos.x >= 0 && blockPos.x < occupancyMatrix.getWidth() &&
                    blockPos.y >= 0 && blockPos.y < occupancyMatrix.getHeight()) {
                    blocksInRange.push_back(blockPos);
                }
            }
        }

        return blocksInRange;
    }


    sf::Vector2f getBlockCenter(sf::Vector2i block) {
        return sf::Vector2f(
            block.x * blockSize.x + blockSize.x / 2.f, 
            block.y * blockSize.y + blockSize.y / 2.f
        );
    }


    std::vector<sf::Vector2i> getNeighbours(sf::Vector2i block) {
        std::vector<sf::Vector2i> neighbours{};

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue; // Skip the current block

                int neighbourX = block.x + dx;
                int neighbourY = block.y + dy;

                if (neighbourX >= 0 && neighbourX < occupancyMatrix.getWidth() &&
                    neighbourY >= 0 && neighbourY < occupancyMatrix.getHeight()) {
                        
                        if (occupancyMatrix(neighbourX, neighbourY).isOccupied()) {
                            continue; // Skip occupied blocks
                        }

                        // For corner moves both adjacent blocks must be free
                        if (dx != 0 && dy != 0) {
                            if (occupancyMatrix(block.x + dx, block.y).isOccupied() || occupancyMatrix(block.x, block.y + dy).isOccupied()) {
                                continue; // Skip diagonal moves
                            }
                        }
                        neighbours.push_back({ neighbourX, neighbourY });
                }
            }
        }

        return neighbours;
    }



    void newRandomWalls() {
        Random random;
        int number_horizontal_walls = random.random(7, 20);
        int number_vertical_walls = random.random(10, 25);

        // Remove all current walls
        for (int x = 0; x < occupancyMatrix.getWidth(); x++) {
            for (int y = 0; y < occupancyMatrix.getHeight(); y++) {
                occupancyMatrix(x, y).set(BlockType::Empty);
            }
        }

        for (int i = 0; i < number_horizontal_walls; i++) {

            int wall_y = random.random(0, occupancyMatrix.getHeight() - 1);
            int wall_start_x = random.random(0, occupancyMatrix.getWidth() - 1);
            int wall_length = random.random(1, occupancyMatrix.getWidth() / 2);

            // Cap wall length to prevent out of bounds
            if (wall_start_x + wall_length >= occupancyMatrix.getWidth()) {
                wall_length = occupancyMatrix.getWidth() - wall_start_x - 1;
            }

            for (int x = wall_start_x; x < wall_start_x + wall_length && x < occupancyMatrix.getWidth(); x++) {
                occupancyMatrix(x, wall_y).set(BlockType::Occupied);
            }
        }

        for (int i = 0; i < number_vertical_walls; i++) {

            int wall_x = random.random(0, occupancyMatrix.getWidth() - 1);
            int wall_start_y = random.random(0, occupancyMatrix.getHeight() - 1);
            int wall_length = random.random(1, occupancyMatrix.getHeight() / 2);

            // Cap wall length to prevent out of bounds
            if (wall_start_y + wall_length >= occupancyMatrix.getHeight()) {
                wall_length = occupancyMatrix.getHeight() - wall_start_y - 1;
            }

            for (int y = wall_start_y; y < wall_start_y + wall_length && y < occupancyMatrix.getHeight(); y++) {
                occupancyMatrix(wall_x, y).set(BlockType::Occupied);
            }
        }

    }


private:
sf::Vector2i worldSize;
sf::Vector2i blockSize = { 25, 25 };

// Use 2d matrix to track which blocks are occupied (wall)
Matrix<Block> occupancyMatrix;

sf::Color gridColor = sf::Color::White;
sf::VertexArray gridLines;


void buildGridLines() {
    gridLines.clear();
    gridLines.setPrimitiveType(sf::PrimitiveType::Lines);
    
    for (float x = 0; x < worldSize.x; x += blockSize.x) {
        gridLines.append(sf::Vertex{{ x, 0 }, gridColor });
        gridLines.append(sf::Vertex{{ x, worldSize.y }, gridColor});
    }
    for (float y = 0; y < worldSize.y; y += blockSize.y) {
        gridLines.append(sf::Vertex{{ 0, y }, gridColor });
        gridLines.append(sf::Vertex{{ worldSize.x, y }, gridColor});
    }
}

};


#endif // WORLD_HPP