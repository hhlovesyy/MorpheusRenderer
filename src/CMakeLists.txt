# Morpheus/src/CMakeLists.txt

# 明确列出所有需要编译的 .cpp 文件
add_executable(MorpheusApp
    main.cpp
    core/Application.cpp
    renderer/Framebuffer.cpp
    renderer/Renderer.cpp
 "renderer/Mesh.cpp" "core/InputManager.cpp" "core/CameraController.cpp" "scene/Camera.cpp" "scene/Scene.cpp" "renderer/shaders/UnlitShader.cpp" "renderer/Clipping.cpp")

# 递归查找所有 .h 文件，以便在VS的解决方案资源管理器中看到它们
file(GLOB_RECURSE HEADERS "*.h")
target_sources(MorpheusApp PRIVATE ${HEADERS})

# 设置头文件搜索路径
target_include_directories(MorpheusApp PRIVATE
    "${CMAKE_SOURCE_DIR}/src" 

    # --- 这里是关键的修改 ---
    # 我们需要告诉编译器去哪里找 nlohmann/json.hpp
    # ${nlohmann_json_SOURCE_DIR}/include # <--- 新增这一行
    # -------------------------

    ${SDL_INCLUDE_DIRS}
    ${tinyobjloader_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)
    
# 链接依赖库
target_link_libraries(MorpheusApp PRIVATE
    nlohmann_json::nlohmann_json # <--- 添加这一行
    SDL2::SDL2main
    SDL2::SDL2
)

# 将 assets 文件夹复制到可执行文件旁边
# 这一行必须在 add_executable 之后
add_custom_command(TARGET MorpheusApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets" 
    "$<TARGET_FILE_DIR:MorpheusApp>/assets"
    COMMENT "Copying assets directory..."
)
    
# 将可执行文件设置为启动项目
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MorpheusApp)
