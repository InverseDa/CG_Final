#define CG_FINAL_DEBUG_MSG
#include "framework/engine.hpp"

int main() {
    std::make_unique<Engine>()->Run();
    return 0;
}