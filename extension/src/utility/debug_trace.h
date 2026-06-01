#pragma once

#include <mutex>
#include <string>

class DebugTrace {

public:
    static void create_instance();
    static DebugTrace* get_instance();

    ~DebugTrace();

    void log(const std::string& message);
    void clear();

private:
    DebugTrace();

    static DebugTrace* instance;

    std::mutex mutex;
    std::string file_name;
};