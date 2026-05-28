#include "debug_trace.h"

#include <fstream>

DebugTrace* DebugTrace::instance = nullptr;

void DebugTrace::create_instance() {
    if (instance == nullptr) {
        instance = new DebugTrace();
    }
}

DebugTrace* DebugTrace::get_instance() {
    return instance;
}

DebugTrace::DebugTrace() {
    file_name = "debug_trace.log";

    // Create file if it doesn't exist
    std::ofstream file(file_name, std::ios::app);
}

DebugTrace::~DebugTrace() {

}

void DebugTrace::log(const std::string& message) {
    std::ofstream file(file_name, std::ios::app);

    if (file.is_open()) {
        file << message << std::endl;
    }
}

void DebugTrace::clear() {
    // Opening with trunc clears the file
    std::ofstream file(file_name, std::ios::trunc);
}