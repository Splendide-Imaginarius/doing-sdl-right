#include <iostream>
#include "Window.hpp"
#include "Renderer.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

const char Window::windowModeNames[WindowMode::fullscreen + 1][18] =
{
    "Windowed",
    "Borderless",
    "Fullscreen"
};

void Window::create()
{
    int flags = SDL_WINDOW_OPENGL, sizeX, sizeY, posX, posY;
   
    SDL_Rect rect;
    SDL_GetDisplayBounds(fullscreenDisplay, &rect);
    SDL_DisplayMode dm;
    SDL_GetDisplayMode(fullscreenDisplay, displayMode, &dm);
    
    switch(windowMode)
    {
        case WindowMode::windowed:
            flags |= SDL_WINDOW_RESIZABLE;
            posX = SDL_WINDOWPOS_UNDEFINED;
            posY = SDL_WINDOWPOS_UNDEFINED;
            sizeX = NATIVE_RES_X;
            sizeY = NATIVE_RES_Y;
            break;

        case WindowMode::borderless:
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            posX = rect.x;
            posY = rect.y;
            sizeX = rect.w;
            sizeY = rect.h;
            break;

        case WindowMode::fullscreen:
            flags |= SDL_WINDOW_FULLSCREEN;
            posX = rect.x;
            posY = rect.y;
            sizeX = dm.w;
            sizeY = dm.h;
            break;
    }
    sdlWindow = SDL_CreateWindow("SDL test", posX, posY, sizeX, sizeY, flags);
    if(windowMode == fullscreen) SDL_SetWindowDisplayMode(sdlWindow, &dm);
    context = SDL_GL_CreateContext(sdlWindow);
    SDL_GL_MakeCurrent(sdlWindow, context);
    
    // Init rendering
    program = Renderer::loadShaders("assets/basic_texture.vert", "assets/basic_texture.frag");
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    {
        GLuint attrib = glGetAttribLocation(program ,"pos");
        glVertexAttribPointer(attrib, 2, GL_FLOAT, false, 16, (void*)0);
        glEnableVertexAttribArray(attrib);
        attrib = glGetAttribLocation(program ,"textureCoord");
        glVertexAttribPointer(attrib, 2, GL_FLOAT, false, 16, (void*)8);
        glEnableVertexAttribArray(attrib);
        glBindFragDataLocation(program, 0, "fragPass");
        float data[16] =
        {
            -1, -1, 0, 1,
            -1,  1, 0, 0,
             1,  1, 1, 0,
             1, -1, 1, 1
        };
        glBufferData(GL_ARRAY_BUFFER, 16 * 4, data, GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    {
        int32_t err=glGetError();
        if(err)
            std::cerr << "Error window render " << gluErrorString(err) << std::endl;
    }

    // Init ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, context);
    ImGui_ImplOpenGL3_Init("#version 150");
    ImGui::StyleColorsDark();

}

void Window::useContext()
{
    SDL_GL_MakeCurrent(sdlWindow, context);
}

/*void Window::initImGui()
{
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, context);
}*/

void Window::destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(sdlWindow);
}
