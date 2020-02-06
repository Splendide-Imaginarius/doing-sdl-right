#include <iostream>
#include <SDL2/SDL_syswm.h>
#include "DisplayWindow.hpp"
#include "Renderer.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

const char DisplayWindow::windowModeNames[WindowMode::fullscreen + 1][18] =
{
    "Windowed",
    "Borderless",
    "Fullscreen"
};

void DisplayWindow::create()
{
    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
    int sizeX, sizeY, posX, posY;

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

    // Get avaiable sync modes
    if(SDL_GL_SetSwapInterval(-1) == 0) canAdaptiveSync = true;
    if(SDL_GL_GetSwapInterval() != -1) canAdaptiveSync = false;
    if(SDL_GL_SetSwapInterval(0) == 0) canNoVSync = true;
    if(SDL_GL_GetSwapInterval() != 0) canNoVSync = false;
    if(SDL_GL_SetSwapInterval(1) == 0) canVSync = true;
    if(SDL_GL_GetSwapInterval() != 1) canVSync = false;

    // Detect triple buffer
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(sdlWindow);
    glClearColor(0.1f, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(sdlWindow);
    glClearColor(0, 0.1f, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(sdlWindow);
    glClearColor(0, 0, 0.1f, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(sdlWindow);
    uint8_t col[3];
    glReadPixels(0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, col);
    tripleBuffer = col[1] == 0;
    if(!isSyncModeAvailable(syncMode))
    {
        int i = 0;
        while(!isSyncModeAvailable(static_cast<SyncMode>(i))) i++;
        syncMode = static_cast<SyncMode>(i);
    }
    setSyncMode(syncMode);

    // Init ImGui
    ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL2_InitForOpenGL(sdlWindow, context);
    ImGui_ImplOpenGL3_Init("#version 150");
    ImGui::StyleColorsDark();
}

void DisplayWindow::useContext()
{
    SDL_GL_MakeCurrent(sdlWindow, context);
}

bool DisplayWindow::isSyncModeAvailable(SyncMode syncMode)
{
    switch(syncMode)
    {
        case noVSync:
            return canNoVSync;
        case adaptiveSync:
            return canAdaptiveSync;
        case vSync:
            return canVSync;
        default:
            return false;
    }
}

DisplayWindow::SyncMode DisplayWindow::getSyncMode()
{
    return syncMode;
}

void DisplayWindow::setSyncMode(SyncMode syncMode)
{
    this->syncMode = syncMode;
    int8_t swapInterval = 0;
    switch(syncMode)
    {
        case noVSync:
            swapInterval = 0;
            break;
        case adaptiveSync:
            swapInterval = -1;
            break;
        case vSync:
            swapInterval = 1;
            break;
        default:
            break;
    }
    SDL_GL_SetSwapInterval(swapInterval);
}

void DisplayWindow::swap()
{
    //std::cout << SDL_GL_GetSwapInterval() << std::endl;
    SDL_GL_SwapWindow(sdlWindow);
}

void DisplayWindow::destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(sdlWindow);
}
