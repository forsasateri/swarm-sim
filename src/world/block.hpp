#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <SFML/Graphics.hpp>

enum class BlockType {
    Empty,
    Occupied
};


class Block {
public:

    Block() : type(BlockType::Empty), position{0, 0} {}

    Block(sf::Vector2i position) : type(BlockType::Empty), position{position} {}

    bool isOccupied() const {
        return type == BlockType::Occupied;
    }

    void toggle() {
        if (type == BlockType::Empty) {
            type = BlockType::Occupied;
        } else {
            type = BlockType::Empty;
        }
    }

    sf::Vector2i getPosition() const {
        return position;
    }

private:
    BlockType type;
    sf::Vector2i position;

};

#endif // BLOCK_HPP