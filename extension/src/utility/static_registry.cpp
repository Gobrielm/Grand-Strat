#include "static_registry.hpp"
#include "../singletons/recipe_info.hpp"
#include "../singletons/factory_creator.hpp"
#include "../singletons/user_singletons/country_manager.hpp"
#include "../singletons/pop_manager.hpp"
#include "../singletons/ai_manager.hpp"

#include <cstdlib>
#include <thread>
#include <chrono>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

static PROCESS_INFORMATION pythonProcess;

void StaticRegistry::initialize() {

#ifdef _WIN32
    STARTUPINFO si = { sizeof(si) };
    ZeroMemory(&pythonProcess, sizeof(pythonProcess));
    std::wstring cmd = L"python ml_server.py";
    if (CreateProcessW(
        NULL,
        cmd.data(), // mutable buffer
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pythonProcess
    ))
    {
        std::cout << "Python server started.\n";
        // Optional: wait a moment to let it initialize
        std::this_thread::sleep_for(std::chrono::seconds(2));
    } else {
        std::cerr << "Failed to start Python server.\n";
    }
#else
    system("python3 ml_server.py &");
    std::this_thread::sleep_for(std::chrono::seconds(2));
#endif

    CargoInfo::initialize_singleton();
    LocalPriceController::set_base_prices();
    RecipeInfo::create();
    FactoryCreator::create();
    
    CountryManager::create();
    PopManager::create();
    
    BasePop::create_base_needs();
    BasePop::create_base_wants();
    AiManager::create();
}

void StaticRegistry::uninitialize() {

#ifdef _WIN32
    if (pythonProcess.hProcess) {
        TerminateProcess(pythonProcess.hProcess, 0);
        CloseHandle(pythonProcess.hProcess);
        CloseHandle(pythonProcess.hThread);
        std::cout << "Python server terminated.\n";
    }
#else
    system("pkill -f ml_server.py");
#endif

    AiManager::cleanup();
    PopManager::cleanup();
    CountryManager::cleanup();
    FactoryCreator::cleanup();
    RecipeInfo::cleanup();
    CargoInfo::cleanup();
    
}