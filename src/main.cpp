#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>

std::string lastDroppedFile = "None";

void drop_callback(GLFWwindow* window, int count, const char** paths)
{
    for (int i = 0; i < count;i++)
    {
        std::string path = paths[i];

        if (path.find(".obj") != std::string::npos || path.find(".OBJ") != std::string::npos)
        {
            std::cout << "[System] OBJ File Dropped: " << path << std::endl;
            lastDroppedFile = path;

            // TODO: call LoadModel(path) here
        } 
        else 
        {
            std::cout << "[System] Ignored non-obj file: " << path << std::endl;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_font_scale(ImGuiIO &io)
{
    if(io.KeyShift && io.MouseWheel != 0.0f){
        float zoom_speed = 0.1f;
        io.FontGlobalScale += io.MouseWheel * zoom_speed;

        io.FontGlobalScale = std::min(std::max(io.FontGlobalScale, 0.5f), std::min(io.FontGlobalScale, 4.0f));
    }
}

int main()
{
    // initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1600, 900, "RayTracing Renderer - Day 1", NULL, NULL);

    if(window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // callback registration
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetDropCallback(window, drop_callback); 

    // glad initialization
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // imgui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();(void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // open docking

    ImGui::StyleColorsDark();

    // imgui backend initialzation
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    // main loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // new frame for imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // adjust font size by scroll
        scroll_font_scale(io);

        // create docker space for full screen, let imgui take iver whole window
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), 0, 0);

        // window widget
        ImGui::Begin("Inspector");
        {
            ImGui::Text("Transform Info");

            static float pos[3] = {0.0f, 0.0f, 0.0f};
            ImGui::DragFloat3("Position", pos, 0.1f);

            static float rot[3] = {0.0f, 0.0f, 0.0f};
            ImGui::DragFloat3("Rotation", rot, 0.1f);

            static float scale[3] = {0.0f, 0.0f, 0.0f};
            ImGui::DragFloat3("Scale", scale, 0.1f);

            ImGui::Separator();
            ImGui::Text("ray tracing settings");

            static bool enableRT = false;
            ImGui::Checkbox("Enable ray Tracing", &enableRT);
        }
        ImGui::End();

        // asset Browser
        ImGui::Begin("Asset Browser");
        {
            ImGui::Text("Project Assets");
            ImGui::Separator();
            ImGui::Text("Last Dropped File:");
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", lastDroppedFile.c_str());


            ImGui::Button("models/");
            ImGui::Button("textures/");
        }
        ImGui::End();

        // Viewport
        ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.0f,0.0f,0.0f,1.0f));
        ImGui::Begin("Viewport");
        {
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();

            // show current viewport resolution
            ImGui::SetCursorPos(ImVec2(10,10));
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Viewport Resolution: %.0f x %.0f", viewportSize.x, viewportSize.y);

            // TODO: 明天我们就在这里用 ImGui::Image() 绘制 FBO 纹理
        }
        ImGui::End();
        ImGui::PopStyleColor();

        // render
        ImGui::Render();

        // set frame buffer color
        int display_w, display_h;
        glfwGetFramebufferSize(window,&display_w,&display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw ImGui data
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        
        glfwSwapBuffers(window);
    }

    // clean up
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
