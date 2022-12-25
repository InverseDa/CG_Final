cmake_minimum_required(VERSION 3.24)
project(CG_Final)

set(CMAKE_CXX_STANDARD 17)

# Google Sanitizer (Only WSL)-fsanitize=undefined -fsanitize=address
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address  -fno-omit-frame-pointer -g3")

# 暂存环境变量
set(GLFW_HOME $ENV{GLFW_HOME})
set(GLAD_HOME $ENV{GLAD_HOME})
set(ASSIMP_HOME $ENV{ASSIMP_HOME})

# 设置头文件目录
include_directories("${GLFW_HOME}/include")
include_directories("${GLAD_HOME}/include")
include_directories("${ASSIMP_HOME}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")

# 添加 GLFW3 预编译库
add_library(glfw SHARED IMPORTED)
SET_TARGET_PROPERTIES(glfw PROPERTIES IMPORTED_LOCATION "${GLFW_HOME}/lib/libglfw.dylib")
# 链接 GLFW3 预编译库
link_libraries(glfw)

# 寻找assimp
add_library(assimp SHARED IMPORTED)
SET_TARGET_PROPERTIES(assimp PROPERTIES IMPORTED_LOCATION "${ASSIMP_HOME}/lib/libassimp.5.2.4.dylib")
# 链接assimp库
link_libraries(assimp)

add_executable(Final main.cpp src/glad.c include/Texture/Texture.cpp include/Camera/Camera.cpp src/TriMesh.cpp)
