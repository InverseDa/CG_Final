#ifndef CG_FINAL_TOOLS_HPP
#define CG_FINAL_TOOLS_HPP

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

template <typename Func, typename... Args>
void measure_time(const std::string& name, Func&& func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    try {
        func(std::forward<Args>(args)...);
    } catch (const std::exception& e) {
        auto end = std::chrono::high_resolution_clock::now();
        std::cerr << "Exception caught: " << e.what() << std::endl;
        std::cout << name << " took "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms but threw an exception\n";
        throw; // Optionally rethrow the exception
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << name << " took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ms\n";
}


#endif // CG_FINAL_TOOLS_HPP
