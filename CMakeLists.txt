# Morpheus/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

project(Morpheus VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- MSVC特定设置，避免一些烦人的警告 ---
if(MSVC)
    add_compile_options(/W4 /WX-) # W4是高警告等级, WX-是不要把警告当错误
    # 避免min/max宏冲突
    add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- 第三方依赖管理 (FetchContent) ---
include(FetchContent)

# --- 新增 nlohmann-json 的下载声明 ---
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2 # 使用一个稳定的版本
)
FetchContent_MakeAvailable(nlohmann_json)
# -------------------------------------

# SDL2 for windowing
FetchContent_Declare(
  SDL
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.28.2
)
FetchContent_MakeAvailable(SDL)

# tinyobjloader for model loading
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG v2.0.0rc10
)
FetchContent_MakeAvailable(tinyobjloader)

# stb for image loading (我们现在不用，但先加进来)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# 启用文件夹视图，让VS中的项目结构更清晰
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 添加我们的源代码
add_subdirectory(src)
