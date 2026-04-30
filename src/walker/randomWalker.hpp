#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <utility>
#include <memory>

#include "util/random.hpp"

#include "world/world.hpp"
#include "worldModel.hpp"
#include "util/logger.hpp"
#include "world/coordinateSystem.hpp"

class RandomWalker {
public:
    RandomWalker(
        sf::Color color, 
        World& world,
        std::shared_ptr<GameLogger> walkerLogger
    ) : world(world),
        logger(std::move(walkerLogger)),
        worldModel(logger) {
        
        this->color = color;
        position = { 0.f, 0.f };
        currentSpeed = 0;
        maxSpeed = 1500.f;

        setNewTarget();
    }

    RandomWalker(const RandomWalker&) = delete;
    RandomWalker& operator=(const RandomWalker&) = delete;
    RandomWalker(RandomWalker&&) noexcept = default;
    RandomWalker& operator=(RandomWalker&&) noexcept = delete;

    void updateLogic() {
        logger->info("Updating logic");

        logger->info("Current position: (" + std::to_string(position.x) + ", " + std::to_string(position.y) + 
            "), current block: (" + std::to_string(currentBlock.x) + ", " + std::to_string(currentBlock.y) + 
            "), target: (" + std::to_string(target.x) + ", " + std::to_string(target.y) + 
            "), route length: " + std::to_string(currentRoute.size()));

        logger->debug("Observing world");
        observeWorld();
        logger->debug("World observed");

        if (isPathBlocked()) {
            logger->info("Path is blocked, recalculating route");
            calculateRoute();
        }

        bool reachedTarget = routeIndex >= currentRoute.size();
        bool noValidRoute = currentRoute.empty();

        int make_new_route_attempts = 0;
        while (noValidRoute || reachedTarget) {

            make_new_route_attempts++;
            if (make_new_route_attempts > 10) {
                logger->error("Failed to find valid route after 10 attempts, giving up");
                exit(-1);
            }

            logger->info("No valid route or target reached, picking new target");
            setNewTarget();
            calculateRoute();

            reachedTarget = world.getBlock(position) == world.getBlock(target);
            noValidRoute = currentRoute.empty();
            logger->info("Set new target and calculated route. Reached target: " + std::to_string(reachedTarget) + ", no valid route: " + std::to_string(noValidRoute));
            logger->info("Path length: " + std::to_string(currentRoute.size()));
        }
    }

    void step(float deltaTime)
    {
        logger->debug("Stepping with deltaTime: " + std::to_string(deltaTime));
        
        int route_length = currentRoute.size();

        if (currentRoute.empty() || routeIndex >= currentRoute.size()) {
            logger->debug("No active route to step along; updating logic before moving");
            updateLogic();
            return;
        }

        sf::Vector2i newCurrentBlock = world.getBlock(position);
        if (newCurrentBlock != currentBlock || route_length == 0) {
            logger->info("Entered new block: (" + std::to_string(newCurrentBlock.x) + ", " + std::to_string(newCurrentBlock.y) + ")");
            currentBlock = newCurrentBlock;
            updateLogic();
            logger->debug("Logic updated");

            logger->info("Current position: (" + std::to_string(position.x) + ", " + std::to_string(position.y) + 
                "), current block: (" + std::to_string(currentBlock.x) + ", " + std::to_string(currentBlock.y) + 
                "), target: (" + std::to_string(target.x) + ", " + std::to_string(target.y) + 
                "), route length: " + std::to_string(currentRoute.size()));
        }

        logger->debug("Route length: " + std::to_string(currentRoute.size()) + ", current index: " + std::to_string(routeIndex));
        sf::Vector2f nextTarget = gridToWorld(currentRoute[routeIndex]); //world.getBlockCenter(currentRoute[routeIndex]);

        sf::Vector2f direction = nextTarget - position;
        float distanceToTarget = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        
        currentSpeed = 250.f; // Set a constant speed for now

        float movementThisTick = currentSpeed * deltaTime;

        if (movementThisTick > distanceToTarget) {

            logger->debug("Overshooting target, moving directly to target and advancing route index");
            
            position = nextTarget; // Move directly to the target if we would overshoot
            routeIndex++; // Advance to the next route point
            // updateLogic();
            logger->info("Set position to next target: (" + std::to_string(position.x) + ", " + std::to_string(position.y) + "). Advanced route index to " + std::to_string(routeIndex));

        } else {
            direction /= distanceToTarget; // Normalize the direction
            position += direction * currentSpeed * deltaTime; 
        }     
    }


    bool isPathBlocked() {
        for (std::size_t i = routeIndex; i < currentRoute.size(); ++i) {
            if (world.isOccupied(gridToWorld(currentRoute[i]))) {
                return true;
            }
        }
        return false;
    }


    void observeWorld() {
        // worldModel.clear(5); // Clear any data older than 5 seconds

        // For each block in range, update the world model with the current state of the block
        int observationRange = 3;
        std::vector<sf::Vector2i> blocksInRange = world.getBlocksInRange(position, observationRange);
        logger->debug("Observing " + std::to_string(blocksInRange.size()) + " blocks in range");

        for (const auto& block : blocksInRange) {

            logger->debug("Attempting update for block (" + std::to_string(block.x) + ", " + std::to_string(block.y) + ")");

            bool occupied = world.isOccupied(gridToWorld(block));

            logger->debug("Block (" + std::to_string(block.x) + ", " + std::to_string(block.y) + ") is " + (occupied ? "Occupied" : "Empty"));

            worldModel.update(block, occupied ? BlockState::Occupied : BlockState::Empty);

            logger->debug("Block (" + std::to_string(block.x) + ", " + std::to_string(block.y) + ") update complete");
        }
    }


    void setTargetFromUSerInput(sf::Vector2i mousePos) {
        target = gridToWorld(world.getBlock({ mousePos.x, mousePos.y }));
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
        for (std::size_t i = routeIndex; i < currentRoute.size(); ++i) {
            sf::Vector2f stepPos = gridToWorld(currentRoute[i]);
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
    std::shared_ptr<GameLogger> logger;

    sf::Vector2f position;
    sf::Vector2f target;

    sf::Vector2i currentBlock;

    float currentSpeed;
    float maxSpeed;

    float size = 20.f;

    sf::Color color;
    World& world;
    std::vector<sf::Vector2i> currentRoute;
    std::size_t routeIndex = 0;

    WorldModel worldModel;

    void setNewTarget() {
        Random rand;
        int randX = rand.random(0, GRID_SIZE_X - 1);
        int randY = rand.random(0, GRID_SIZE_Y - 1);
        target = gridToWorld(randX, randY);
    }


    void calculateRoute() {
        currentRoute = worldModel.calculatePath(world.getBlock(position), world.getBlock(target));
        routeIndex = 0;
    }
};