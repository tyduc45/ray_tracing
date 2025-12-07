cmake_minimum_required(VERSION 3.10)
project(RayTracingRenderer)

set(CMAKE_CXX_STANDARD 17)

# --- 路径设置 ---
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)

# --- 1. GLAD (直接编译源文件) ---
add_library(glad ${VENDOR_DIR}/glad/src/glad.c)
target_include_directories(glad PUBLIC ${VENDOR_DIR}/glad/include)

# --- 2. GLFW ---
# 指向预编译的库和头文件
set(GLFW_DIR ${VENDOR_DIR}/glfw)
include_directories(${GLFW_DIR}/include)
link_directories(${GLFW_DIR}/lib-vc2022) # 确保这里对应你下载的文件夹名

# --- 3. GLM (Header Only) ---
include_directories(${VENDOR_DIR}/glm)

# --- 4. Assimp ---
include_directories(${VENDOR_DIR}/assimp/include)
link_directories(${VENDOR_DIR}/assimp/lib)

# --- 5. ImGui (需要编译源文件) ---
# 定义 ImGui 源文件列表 (核心 + Backends)
set(IMGUI_DIR ${VENDOR_DIR}/imgui)
file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
# ImGui 需要链接 GLFW 和 OpenGL
target_link_libraries(imgui glfw3.lib opengl32.lib) 

# --- 可执行文件 ---
add_executable(App 
        src/main.cpp
)

# --- 链接所有库 ---
target_link_libraries(App 
    glad
    imgui
    glfw3.lib      # GLFW 静态库名，可能是 glfw3.lib 或 glfw3dll.lib
    assimp-vc143-mtd.lib  # Assimp 库名，请根据你 vendor/assimp/lib 下的实际文件名修改！
    opengl32.lib   # Windows 系统自带
)

# --- 编译后命令：复制 DLL ---
# 这一步非常重要，否则运行会报“找不到 xxx.dll”
add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${VENDOR_DIR}/assimp/bin/assimp-vc143-mtd.dll" # 确保路径指向你下载的 assimp dll
    $<TARGET_FILE_DIR:App>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${VENDOR_DIR}/glfw/lib-vc2022/glfw3.dll" # 如果你用的是动态链接版本的 GLFW
    $<TARGET_FILE_DIR:App>
)