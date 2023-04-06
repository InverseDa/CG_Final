cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
# Google Sanitizer: -fsanitize=undefined -fsanitize=address
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address  -fno-omit-frame-pointer -g3")

if (APPLE)
    project(CG_Final)
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

    # 添加 assimp 预编译库
    add_library(assimp SHARED IMPORTED)
    SET_TARGET_PROPERTIES(assimp PROPERTIES IMPORTED_LOCATION "${ASSIMP_HOME}/lib/libassimp.5.2.4.dylib")
    # 链接 assimp 预编译库
    link_libraries(assimp)

    add_executable(CG_Final main.cpp src/glad.c include/Texture/Texture.cpp include/Camera/Camera.cpp src/TriMesh.cpp)
elseif (CMAKE_HOST_UNIX)
    project(CG_Final)
    aux_source_directory("./" PROJECT_SOURCES)
    add_executable(CG_Final main.cpp src/glad.c include/Texture/Texture.cpp include/Camera/Camera.cpp src/TriMesh.cpp)
    target_include_directories(CG_Final PRIVATE include)
    include_directories(${PROJECT_SOURCE_DIR}/include)
    # dependency
    find_package(glfw3 CONFIG REQUIRED)
    find_package(assimp CONFIG REQUIRED)
    # link
    target_link_libraries(CG_Final PRIVATE glfw)
    target_link_libraries(CG_Final PRIVATE assimp)
elseif (CMAKE_HOST_WIN32)
    message("Checking vcpkg...")
    # vcpkg
    if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
                CACHE STRING "")
    endif ()
    if (DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
    endif ()

    project(CG_Final)

    aux_source_directory("./" PROJECT_SOURCES)
    add_executable(CG_Final main.cpp src/glad.c include/Texture/Texture.cpp include/Camera/Camera.cpp src/TriMesh.cpp)
    include_directories(${PROJECT_SOURCE_DIR}/include)

    # dependency
    find_package(glad CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(assimp CONFIG REQUIRED)

    # link
    target_link_libraries(CG_Final PRIVATE glad::glad)
    target_link_libraries(CG_Final PRIVATE glfw)
    target_link_libraries(CG_Final PRIVATE assimp::assimp)
endif ()