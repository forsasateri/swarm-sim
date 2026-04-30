#ifndef VECTOR_ADDITIONS_HPP
#define VECTOR_ADDITIONS_HPP

#include <SFML/System.hpp>

// Extend Vector2i for use in std::map and std::set
namespace std {
    template <>
    struct less<sf::Vector2i> {
        bool operator()(const sf::Vector2i& a, const sf::Vector2i& b) const {
            return std::tie(a.x, a.y) < std::tie(b.x, b.y);
        }
    };
}

#endif // VECTOR_ADDITIONS_HPP