#ifndef WORLD_MODEL_HPP
#define WORLD_MODEL_HPP

#include <SFML/System.hpp>
#include <vector>
#include <chrono>
#include <map>
#include <queue>

#include "util/matrix.hpp"

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
    WorldModel(sf::Vector2i worldSize) : m_world_model(worldSize.x, worldSize.y) {};

    void update(sf::Vector2i block, BlockState state) {
        m_world_model(block.x, block.y).update(state);
    }

    // void clear(int dataOlderThanSec) {
    //     int dataOlderThanMillisec = dataOlderThanSec * 1000; // Turn sec into millisec
    //     for (int x = 0; x < m_world_model.getWidth(); x++) {
    //         for (int y = 0; y < m_world_model.getHeight(); y++) {
    //             if (m_world_model(x, y).dataAge() > dataOlderThanMillisec) {
    //                 m_world_model(x, y).update(BlockState::Unknown);
    //             }
    //         }
    //     }
    // }



    // PATHFINDING STUFF
    // TODO: Move to own system that uses the world model or other source of truth for planning

    struct Node {
        sf::Vector2i pos;
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

    std::vector<sf::Vector2i> getNeighbours(sf::Vector2i block) {
        std::vector<sf::Vector2i> neighbours{};

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue; // Skip the current block

                int neighbourX = block.x + dx;
                int neighbourY = block.y + dy;

                if (neighbourX >= 0 && neighbourX < m_world_model.getWidth() &&
                    neighbourY >= 0 && neighbourY < m_world_model.getHeight()) {
                        
                        if (m_world_model(neighbourX, neighbourY).isOccupied()) {
                            continue; // Skip occupied blocks
                        }

                        // For corner moves both adjacent blocks must be free
                        if (dx != 0 && dy != 0) {
                            if (m_world_model(block.x + dx, block.y).isOccupied() || m_world_model(block.x, block.y + dy).isOccupied()) {
                                continue; // Skip diagonal moves
                            }
                        }
                        neighbours.push_back({ neighbourX, neighbourY });
                }
            }
        }

        return neighbours;
    }

    std::vector<sf::Vector2i> calculatePath(sf::Vector2i start, sf::Vector2i target) {

        // Frontier: Nodes we need to visit, sorted by lowest F-cost
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        openSet.push({start, 0});

        // Tracking costs and parentage
        std::map<sf::Vector2i, sf::Vector2i> cameFrom;
        std::map<sf::Vector2i, int> gCost;

        gCost[start] = 0;

        while (!openSet.empty()) {
            sf::Vector2i current = openSet.top().pos;
            openSet.pop();

            // Goal reached!
            if (current == target) {
                std::vector<sf::Vector2i> path;
                while (current != start) {
                    path.push_back(current);
                    current = cameFrom[current];
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            for (sf::Vector2i neighbor : getNeighbours(current)) {
                int newGCost = gCost[current] + 1; // Assuming move cost of 1

                if (gCost.find(neighbor) == gCost.end() || newGCost < gCost[neighbor]) {
                    gCost[neighbor] = newGCost;
                    int fCost = newGCost + heuristic(neighbor, target);
                    openSet.push({neighbor, fCost});
                    cameFrom[neighbor] = current;
                }
            }
        }

        return {}; // Return empty if no path found
    }


private:
    Matrix<WorldModelBlock> m_world_model;

};

#endif