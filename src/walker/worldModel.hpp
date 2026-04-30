#ifndef WORLD_MODEL_HPP
#define WORLD_MODEL_HPP

#include <SFML/System.hpp>
#include <vector>
#include <chrono>
#include <queue>
#include <limits>
#include <algorithm>
#include <memory>

#include "util/matrix.hpp"
#include "util/logger.hpp"

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

enum class BlockState {
    Unknown,
    Empty,
    Occupied
};

const int BASE_STALENESS_THRESHOLD_MS = 5000; // Base staleness threshold in milliseconds
const int STALENESS_INCREMENT_PER_CONFIRMATION_MS = 3000; // Additional staleness time added per confirmation
const int MAX_STALENESS_THRESHOLD_MS = 30000; // Maximum staleness threshold to prevent infinite belief
const int OBSERVATION_CONFIRMATION_INTERVAL_MS = 500; // Time interval to confirm observations

class WorldModelBlock {
public:
    WorldModelBlock() : m_state(BlockState::Unknown) {}

    bool isOccupied() const {
        if (isStale()) {
            return false; // Treat stale data as empty
        }

        return m_state == BlockState::Occupied;
    }

    void update(BlockState newState) {

        if (newState != m_state) {

            m_state = newState;
            m_last_updated = Clock::now();
            m_last_observed = Clock::now();
            m_times_confirmed = 0;

        } else {

            auto time_since_last_observed = std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now() - m_last_observed
            ).count();

            if (time_since_last_observed > OBSERVATION_CONFIRMATION_INTERVAL_MS) {

                m_last_observed = Clock::now();
                m_times_confirmed++;
 
            }
        }
    }

    bool isStale() const {
        auto age = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - m_last_observed).count();
        int stalenessThreshold = BASE_STALENESS_THRESHOLD_MS + (m_times_confirmed * STALENESS_INCREMENT_PER_CONFIRMATION_MS);
        stalenessThreshold = std::min(stalenessThreshold, MAX_STALENESS_THRESHOLD_MS); // Cap the staleness threshold
        return age > stalenessThreshold;
    }

private:
    BlockState m_state;
    TimePoint m_last_updated; // When last teh actual state changed
    TimePoint m_last_observed; // When was this block last observed (for staleness)
    unsigned int m_times_confirmed = 0; // How many times has this block been observed as the same state (for belief reinforcement)
};


class WorldModel {
public:
    WorldModel(std::shared_ptr<GameLogger> logger): 
        worldModel_(GRID_SIZE_X, GRID_SIZE_Y),    
        logger(std::move(logger)) {}

    void update(sf::Vector2i block, BlockState state) {
        if (logger == nullptr) {
            std::cerr << "Error: WorldModel logger is not set. Cannot log updates." << std::endl;
            return;
        }

        logger->debug("Updating block (" + std::to_string(block.x) + ", " + std::to_string(block.y) + ") to state " + 
            (state == BlockState::Occupied ? "Occupied" : "Empty"));

        worldModel_(block.x, block.y).update(state);
    }


    // PATHFINDING STUFF
    // TODO: Move to own system that uses the world model or other source of truth for planning

    struct Node {
        sf::Vector2i pos;
        int gCost;
        int fCost;

        // Priority queue needs to pick the SMALLEST fCost
        bool operator>(const Node& other) const {
            return fCost > other.fCost;
        }
    };

    // Manhattan distance heuristic
    int heuristic(sf::Vector2i a, sf::Vector2i b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }

    std::vector<sf::Vector2i> calculatePath(sf::Vector2i start, sf::Vector2i target) {

        logger->info("Calculating path from (" + std::to_string(start.x) + ", " + std::to_string(start.y) + 
            ") to (" + std::to_string(target.x) + ", " + std::to_string(target.y) + ")");

            
        if (start.x < 0 || start.x >= worldModel_.getWidth() || start.y < 0 || start.y >= worldModel_.getHeight()) {
            logger->critical("Start position (" + std::to_string(start.x) + ", " + std::to_string(start.y) + ") is out of bounds");
            return {};
        }

        if (target.x < 0 || target.x >= worldModel_.getWidth() || target.y < 0 || target.y >= worldModel_.getHeight()) {
            logger->critical("Target position (" + std::to_string(target.x) + ", " + std::to_string(target.y) + ") is out of bounds");
            return {};
        }

        const int width = worldModel_.getWidth();
        const int height = worldModel_.getHeight();
        const int totalCells = width * height;

        auto inBounds = [width, height](const sf::Vector2i& p) {
            return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
        };

        auto toIndex = [width](const sf::Vector2i& p) {
            return p.y * width + p.x;
        };

        auto toPos = [width](int index) {
            return sf::Vector2i{ index % width, index / width };
        };

        if (!inBounds(start) || !inBounds(target)) {
            logger->error("Cannot calculate path: start or target is out of bounds");
            return {};
        }

        // Frontier: Nodes we need to visit, sorted by lowest F-cost
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        openSet.push({ start, 0, heuristic(start, target) });

        // Tracking costs and parentage using flat arrays to avoid map overhead.
        const int INF = std::numeric_limits<int>::max();
        std::vector<int> gCost(totalCells, INF);
        std::vector<int> parent(totalCells, -1);

        const int startIndex = toIndex(start);
        const int targetIndex = toIndex(target);
        gCost[startIndex] = 0;

        while (!openSet.empty()) {
            Node currentNode = openSet.top();
            openSet.pop();

            const sf::Vector2i current = currentNode.pos;
            const int currentIndex = toIndex(current);

            // Skip stale queue entries so we only expand the latest best-known route to this node.
            if (currentNode.gCost != gCost[currentIndex]) {
                continue;
            }

            // Goal reached!
            if (current == target) {
                std::vector<sf::Vector2i> path;

                int walkIndex = targetIndex;
                while (walkIndex != startIndex) {
                    if (walkIndex < 0) {

                        logger->critical("Error reconstructing path: invalid index encountered. This should not happen.");
                        return {};
                    }

                    path.push_back(toPos(walkIndex));
                    walkIndex = parent[walkIndex];
                }

                std::reverse(path.begin(), path.end());

                logger->info("Path found with " + std::to_string(path.size()) + " steps.");

                return path;
            }

            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }

                    const int neighborX = current.x + dx;
                    const int neighborY = current.y + dy;

                    if (neighborX < 0 || neighborX >= width || neighborY < 0 || neighborY >= height) {
                        continue;
                    }

                    if (worldModel_(neighborX, neighborY).isOccupied()) {
                        continue;
                    }

                    // For corner moves both adjacent blocks must be free.
                    if (dx != 0 && dy != 0) {
                        if (worldModel_(current.x + dx, current.y).isOccupied() ||
                            worldModel_(current.x, current.y + dy).isOccupied()) {
                            continue;
                        }
                    }

                    const sf::Vector2i neighbor{ neighborX, neighborY };
                    const int neighborIndex = toIndex(neighbor);
                    int newGCost = gCost[currentIndex] + 1; // Assuming move cost of 1

                    if (newGCost < gCost[neighborIndex]) {
                        gCost[neighborIndex] = newGCost;
                        parent[neighborIndex] = currentIndex;

                        int fCost = newGCost + heuristic(neighbor, target);
                        openSet.push({ neighbor, newGCost, fCost });
                    }
                }
            }
        }

        logger->info("No path found.");
        return {}; // Return empty if no path found
    }


private:
    Matrix<WorldModelBlock> worldModel_;

    std::shared_ptr<GameLogger> logger;

};

#endif