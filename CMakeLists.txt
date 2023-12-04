cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
# Google Sanitizer: -fsanitize=undefined -fsanitize=address
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address  -fno-omit-frame-pointer -g3")
project(CG_Final)
aux_source_directory("./" PROJECT_SOURCES)
add_executable(CG_Final src/main.cpp)

target_include_directories(CG_Final
    PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/include
)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(CG_Final PRIVATE assimp::assimp)