cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
if (WIN32)
    set(CMAKE_TOOLCHAIN_FILE C:/Users/miaokeda/scoop/apps/vcpkg/current/scripts/buildsystems/vcpkg.cmake)
endif (WIN32)
if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/homebrew/lib -lassimp")
endif (APPLE)
# Google Sanitizer: -fsanitize=undefined -fsanitize=address
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address  -fno-omit-frame-pointer -g3")

project(CG_Final)

file(GLOB_RECURSE SOURCE "src/*.cpp")
add_executable(CG_Final src/main.cpp ${SOURCE} src/glad_src/glad.c)

target_include_directories(CG_Final
        PRIVATE
        ${PROJECT_SOURCE_DIR}/thirdparty
        ${PROJECT_SOURCE_DIR}/include
)

if (WIN32)
    find_package(assimp CONFIG REQUIRED)
    target_link_libraries(CG_Final PRIVATE assimp::assimp)
    target_link_libraries(CG_Final PRIVATE ${PROJECT_SOURCE_DIR}/library/windows-x86_64/glfw3.lib)
    target_link_libraries(CG_Final PRIVATE ${PROJECT_SOURCE_DIR}/library/windows-x86_64/freetype.lib)
endif (WIN32)
if (APPLE)
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(CG_Final PRIVATE glfw)
    target_link_libraries(CG_Final PRIVATE assimp)
endif ()