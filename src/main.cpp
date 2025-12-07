// main.cpp
// 依赖：GLFW、GLAD、GLM
// 编译时链接：opengl32.lib glfw3.lib 等（根据你自己工程配置）

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// =================== 工具函数：读文件 ===================
std::string loadTextFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// =================== 工具函数：编译 Shader ===================
GLuint compileShader(GLenum type, const std::string &src, const std::string &debugName)
{
    GLuint shader = glCreateShader(type);
    const char *cstr = src.c_str();
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, '\0');
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        std::cerr << "Shader compile error (" << debugName << "):\n"
                  << log << std::endl;
    }
    return shader;
}

GLuint linkProgram(const std::vector<GLuint> &shaders, const std::string &debugName)
{
    GLuint prog = glCreateProgram();
    for (auto s : shaders)
        glAttachShader(prog, s);

    glLinkProgram(prog);

    GLint success = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint logLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
        std::string log(logLen, '\0');
        glGetProgramInfoLog(prog, logLen, nullptr, log.data());
        std::cerr << "Program link error (" << debugName << "):\n"
                  << log << std::endl;
    }

    for (auto s : shaders)
        glDeleteShader(s);

    return prog;
}

// =================== 场景中的 Sphere 结构 ===================
// 注意：和 GLSL 里保持对齐一致（std430），我们用了两个 vec4：
// centerRadius = (cx, cy, cz, radius)
// colorEmission = (r, g, b, emission)
struct SphereCPU
{
    glm::vec4 centerRadius;
    glm::vec4 colorEmission;
};

int main()
{
    // ---------- 初始化 GLFW ----------
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 1280;
    int height = 720;

    GLFWwindow *window = glfwCreateWindow(width, height, "Compute Shader Ray Tracing (Spheres)", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    // ---------- 初始化 GLAD ----------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // ---------- 创建输出纹理 ----------
    GLuint rayTex = 0;
    glGenTextures(1, &rayTex);
    glBindTexture(GL_TEXTURE_2D, rayTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // ---------- 创建 Sphere 的 SSBO ----------
    std::vector<SphereCPU> spheres;
    spheres.push_back({
        glm::vec4(0.0f, 0.0f, -5.0f, 1.0f), // center (0,0,-5), radius 1
        glm::vec4(1.0f, 0.2f, 0.2f, 0.0f)   // red, emission 0
    });

    spheres.push_back({
        glm::vec4(2.0f, 0.0f, -6.0f, 1.0f),
        glm::vec4(0.2f, 1.0f, 0.2f, 0.0f) // green
    });

    spheres.push_back({
        glm::vec4(-2.0f, 0.0f, -4.0f, 1.0f),
        glm::vec4(0.2f, 0.2f, 1.0f, 0.0f) // blue
    });

    // 一个大地面球
    spheres.push_back({glm::vec4(0.0f, -1001.0f, -5.0f, 1000.0f),
                       glm::vec4(0.8f, 0.8f, 0.8f, 0.0f)});

    GLuint sphereSSBO = 0;
    glGenBuffers(1, &sphereSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 spheres.size() * sizeof(SphereCPU),
                 spheres.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphereSSBO); // binding = 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    int sphereCount = static_cast<int>(spheres.size());

    // ---------- 编译 Compute Shader ----------
    std::string csSrc = loadTextFile("E://cpp_review//capstone_sdust//ray_tracing//src//raytrace.comp");
    GLuint cs = compileShader(GL_COMPUTE_SHADER, csSrc, "raytrace.comp");
    GLuint rayProgram = linkProgram({cs}, "RaytraceProgram");

    // ---------- 编译 显示用的全屏三角形 Shader ----------
    std::string vsSrc = loadTextFile("E://cpp_review//capstone_sdust//ray_tracing//src//fullscreen.vert");
    std::string fsSrc = loadTextFile("E://cpp_review//capstone_sdust//ray_tracing//src//fullscreen.frag");
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc, "fullscreen.vert");
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc, "fullscreen.frag");
    GLuint quadProgram = linkProgram({vs, fs}, "QuadProgram");

    // 全屏三角形不需要 VBO/VAO，直接用 gl_VertexID
    GLuint quadVAO = 0;
    glGenVertexArrays(1, &quadVAO);

    // ---------- 相机参数 ----------
    glm::vec3 camPos(0.0f, 1.0f, 0.0f);
    glm::vec3 camTarget(0.0f, 0.5f, -5.0f);
    glm::vec3 camForward = glm::normalize(camTarget - camPos);
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 camRight = glm::normalize(glm::cross(camForward, worldUp));
    glm::vec3 camUp = glm::normalize(glm::cross(camRight, camForward));
    float fov = 45.0f;

    int frame = 0;

    // =================== 主循环 ===================
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // 如果窗口大小改变，可以重新调整纹理大小（这里简单点略过）
        // glGetFramebufferSize(window, &width, &height);

        // ---------- Dispatch Compute Shader 做 Ray Tracing ----------
        glUseProgram(rayProgram);

        // 屏幕尺寸 & 相机参数
        glUniform1i(glGetUniformLocation(rayProgram, "uWidth"), width);
        glUniform1i(glGetUniformLocation(rayProgram, "uHeight"), height);
        glUniform1i(glGetUniformLocation(rayProgram, "uFrame"), frame++);
        glUniform1i(glGetUniformLocation(rayProgram, "uSphereCount"), sphereCount);

        glUniform3fv(glGetUniformLocation(rayProgram, "uCamPos"), 1, glm::value_ptr(camPos));
        glUniform3fv(glGetUniformLocation(rayProgram, "uCamForward"), 1, glm::value_ptr(camForward));
        glUniform3fv(glGetUniformLocation(rayProgram, "uCamRight"), 1, glm::value_ptr(camRight));
        glUniform3fv(glGetUniformLocation(rayProgram, "uCamUp"), 1, glm::value_ptr(camUp));
        glUniform1f(glGetUniformLocation(rayProgram, "uFov"), fov);

        // 绑定 Sphere SSBO
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphereSSBO);

        // 绑定输出图像
        glBindImageTexture(0, rayTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        // 计算工作组大小
        const int localSizeX = 8;
        const int localSizeY = 8;
        int gx = (width + localSizeX - 1) / localSizeX;
        int gy = (height + localSizeY - 1) / localSizeY;

        glDispatchCompute(gx, gy, 1);

        // 确保写完纹理再拿去画
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // ---------- 画全屏三角形显示结果 ----------
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(quadProgram);
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rayTex);
        glUniform1i(glGetUniformLocation(quadProgram, "uRayTex"), 0);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    // ---------- 资源清理 ----------
    glDeleteProgram(rayProgram);
    glDeleteProgram(quadProgram);
    glDeleteTextures(1, &rayTex);
    glDeleteBuffers(1, &sphereSSBO);
    glDeleteVertexArrays(1, &quadVAO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}