#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>

class WalkerLogger {
public:
    WalkerLogger(int id) : 
        id(id), 
        log_file("walker_" + std::to_string(id) + ".log", std::ios_base::app) {
    }

    // Delete copy constructor and assignment operator to prevent copying
    WalkerLogger(const WalkerLogger&) = delete;
    WalkerLogger& operator=(const WalkerLogger&) = delete;

    void clear() {
        log_file.close();
        log_file.open("walker_" + std::to_string(id) + ".log", std::ios_base::trunc);
    }

    void log(const std::string& message) {
        return;

        // std::cout << "[Walker " << id << "] " << message << std::endl;
    
        // Write to log file
        log_file << "[Walker " << id << "] " << message << std::endl;
        log_file.flush(); // Ensure the message is written to the file immediately
    }

private:
    int id;
    std::ofstream log_file;

};

#endif // LOGGER_HPP