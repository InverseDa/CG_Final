#define CG_FINAL_DEBUG_MSG
#include "framework/engine.hpp"

int main() {
    return std::make_unique<Engine>()->Run();
}