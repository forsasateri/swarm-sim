#ifndef AGENT_CONTEXT_HPP
#define AGENT_CONTEXT_HPP

#include <SFML/System.hpp>
#include <string>
#include <memory>

#include "logger.hpp"


class AgentContext {
public:

    AgentContext(
        std::string name, 
        std::shared_ptr<GameLogger> logger
    ): name(name), logger(logger) {

    }

    ~AgentContext() = default;

    std::string getName() {
        return name;
    }

    std::shared_ptr<GameLogger> getLogger() {
        return logger;
    }


private:
    std::string name;
    std::shared_ptr<GameLogger> logger;
};

#endif // AGENT_CONTEXT_HPP