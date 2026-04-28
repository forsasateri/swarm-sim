#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

#include "random.cc"

#include "world/world.hpp"
#include "world_model.hpp"

class RandomWalker {
public:
    RandomWalker(sf::Vector2i worldSize, sf::Color color, World& world) : world(world), worldModel(worldSize) {
        this->worldSize = worldSize;
        this->color = color;
        position = { 0.f, 0.f };
        setNewTarget();
        currentSpeed = 0;
        maxSpeed = 1500.f;
    }

    void step(float deltaTime)
    {

        observeWorld();

        // If route is empty, set new target and calculate route
        if (currentRoute.empty()) {
            setNewTarget();
            calculateRoute();
            return;
        }

        sf::Vector2f nextTarget = world.getBlockCenter(currentRoute.front());

        sf::Vector2f direction = nextTarget - position;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        
        // // Accelerate towards the target
        // if (currentSpeed < maxSpeed) {

        //     // Make acceleration proportional to current speed compared with max speed
        //     // Gives an acceleration curve that starts fast and slows down as it approaches max speed
        //     float acceleration = 10.f * (1 - currentSpeed / maxSpeed); // Adjust the 10.f to control overall acceleration
        //     currentSpeed += acceleration * deltaTime; // Adjust acceleration as needed
            
        //     if (currentSpeed > maxSpeed) {
        //         currentSpeed = maxSpeed;
        //     }
        // }
        currentSpeed = 250.f; // Set a constant speed for now
        
        if (length > 3) {
            direction /= length; // Normalize the direction
            position += direction * currentSpeed * deltaTime; 
        } else {
            // currentSpeed /= 2; // Stop when close to the target
            
            // Remove the reached target from the route
            currentRoute.erase(currentRoute.begin());

            // Check if route cut of by new walls, if so recalculate
            for (const auto& step : currentRoute) {
                if (world.isOccupied(world.getBlockCenter(step))) {
                    
                    // If target is blocked then we should also update it
                    if  (world.isOccupied(target)) {
                        setNewTarget();
                    }

                    calculateRoute();
                    return;
                }
            }
        }
    }


    void observeWorld() {
        // worldModel.clear(5); // Clear any data older than 5 seconds

        // For each block in range, update the world model with the current state of the block
        float observationRange = 100.f;
        std::vector<sf::Vector2i> blocksInRange = world.getBlocksInRange(position, observationRange);

        for (const auto& block : blocksInRange) {
            bool occupied = world.isOccupied(world.getBlockCenter(block));
            worldModel.update(block, occupied ? BlockState::Occupied : BlockState::Empty);
        }
    }


    void setTargetFromUSerInput(sf::Vector2i mousePos) {
        target = world.getBlockCenter(world.getBlock({ mousePos.x, mousePos.y }));
        calculateRoute();
    }


    void addVertexesToArray( sf::VertexArray& array ) {
        // Draw the walker as a square using two triangles (6 vertices)
        sf::Vector2f topLeft = position - sf::Vector2f(size / 2, size / 2);
        sf::Vector2f topRight = position + sf::Vector2f(size / 2, -size / 2);
        sf::Vector2f bottomLeft = position + sf::Vector2f(-size / 2, size / 2);
        sf::Vector2f bottomRight = position + sf::Vector2f(size / 2, size / 2);

        array.append({ topLeft, color });
        array.append({ bottomLeft, color });
        array.append({ topRight, color });

        array.append({ topRight, color });
        array.append({ bottomLeft, color });
        array.append({ bottomRight, color });

        // Do smal indicator for each route step
        for (const auto& step : currentRoute) {
            sf::Vector2f stepPos = world.getBlockCenter(step);
            sf::Vector2f stepTopLeft = stepPos - sf::Vector2f(5.f, 5.f);
            sf::Vector2f stepTopRight = stepPos + sf::Vector2f(5.f, -5.f);
            sf::Vector2f stepBottomLeft = stepPos + sf::Vector2f(-5.f, 5.f);
            sf::Vector2f stepBottomRight = stepPos + sf::Vector2f(5.f, 5.f);

            array.append({ stepTopLeft, color });
            array.append({ stepBottomLeft, color });
            array.append({ stepTopRight, color });

            array.append({ stepTopRight, color });
            array.append({ stepBottomLeft, color });
            array.append({ stepBottomRight, color });
        }
    }

private:
    sf::Vector2f position;
    sf::Vector2f target;

    float currentSpeed;
    float maxSpeed;

    float size = 20.f;

    sf::Vector2i worldSize;
    sf::Color color;
    World& world;
    std::vector<sf::Vector2i> currentRoute;

    WorldModel worldModel;

    void setNewTarget() {
        Random rand;
        int randX = rand.random(0, worldSize.x);
        int randY = rand.random(0, worldSize.y);
        target = world.getBlockCenter({ (int)randX, (int)randY });
    }


    void calculateRoute() {
        currentRoute = worldModel.calculatePath(world.getBlock(position), world.getBlock(target));
    }
};