cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TOOLCHAIN_FILE C:/Users/miaokeda/scoop/apps/vcpkg/current/scripts/buildsystems/vcpkg.cmake)
# Google Sanitizer: -fsanitize=undefined -fsanitize=address
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address  -fno-omit-frame-pointer -g3")

project(CG_Final)

file(GLOB_RECURSE SOURCE "src/*.cpp")
add_executable(CG_Final src/newmain.cpp src/glad_src/glad.c ${SOURCE})

target_include_directories(CG_Final
    PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/include
)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(CG_Final PRIVATE assimp::assimp)
if(WIN32)
    target_link_libraries(CG_Final PRIVATE ${PROJECT_SOURCE_DIR}/library/windows-x86_64/glfw3.lib)
    target_link_libraries(CG_Final PRIVATE ${PROJECT_SOURCE_DIR}/library/windows-x86_64/freetype.lib)
endif(WIN32)