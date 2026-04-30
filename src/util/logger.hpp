#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>

const bool AGGRESSIVE_FLUSHING = false;


enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    DISABLED
};

const LogLevel DEFAULT_LOG_LEVEL = LogLevel::INFO;

class GameLogger {
public:

    GameLogger(): 
        name("default"), 
        log_file_name("logs/default.log"),
        log_file(log_file_name, std::ios_base::app),
        current_log_level(DEFAULT_LOG_LEVEL) {
    }

    GameLogger(std::string name, LogLevel level = DEFAULT_LOG_LEVEL) : 
        name(name), 
        log_file_name("logs/" + name + ".log"),
        log_file(log_file_name, std::ios_base::app),
        current_log_level(level) {
    }

    // Delete copy constructor and assignment operator to prevent copying
    GameLogger(const GameLogger&) = delete;
    GameLogger& operator=(const GameLogger&) = delete;

    void debug(const std::string& message) {
        if (current_log_level <= LogLevel::DEBUG) {
            log("[DEBUG] " + message);
        }
    }

    void info(const std::string& message) {
        if (current_log_level <= LogLevel::INFO) {
            log("[INFO] " + message);
        }
    }

    void warning(const std::string& message) {
        if (current_log_level <= LogLevel::WARNING) {
            log("[WARNING] " + message);
        }
    }

    void error(const std::string& message) {
        if (current_log_level <= LogLevel::ERROR) {
            log("[ERROR] " + message);
            log_file.flush();
        }
    }

    void critical(const std::string& message) {
        if (current_log_level <= LogLevel::CRITICAL) {
            log("[CRITICAL] " + message);
            log_file.flush();
        }
    }


    void clear() {
        log_file.close();
        log_file.open(log_file_name, std::ios_base::trunc);
    }

    void log(const std::string& message) {

        log_file << "[" << name << "] " << message << std::endl;

        if (AGGRESSIVE_FLUSHING) {
            log_file.flush(); // Ensure the message is written to the file immediately
        }
    }

private:
    std::string name;
    std::string log_file_name;
    std::ofstream log_file;
    LogLevel current_log_level;

};

#endif // LOGGER_HPP