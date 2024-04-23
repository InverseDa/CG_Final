#include "framework/engine.hpp"

int main() {
    std::make_unique<Engine>()->Run();
    return 0;
}