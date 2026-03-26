#include "input-manager.h"
#include "imgui/imgui.h"
#include <iostream>

void InputManager::AddOnMouseMoveListener(const std::function<void(vec2, vec2)> &f) {
    onMouseMoveListeners.push_back(f);
}
void InputManager::AddOnMouseClickListener(const std::function<void(vec2)> &f) {
    onMouseClickListeners.push_back(f);
}
void InputManager::AddOnMouseReleaseListener(const std::function<void(vec2)> &f) {
    onMouseReleaseListeners.push_back(f);
}
void InputManager::AddOnMouseScrollListener(const std::function<void(float)> &f) {
    onMouseScrollListeners.push_back(f);
}

void InputManager::OnMouseMove(vec2 newPos) {
    ImGuiIO &io = ImGui::GetIO();
    if(io.WantCaptureMouse) return;

    for(auto &f : onMouseMoveListeners) {
        f(newPos, newPos - mousePosition);
    }
    // mousePosition = newPos;
}

void InputManager::OnMouseClick(bool leftMouse, bool pressDown) {
    ImGuiIO &io = ImGui::GetIO();
    if(io.WantCaptureMouse) return;

    if(leftMouse) {
        lMouseDown = pressDown;
        if(pressDown) {
            for(auto &f : onMouseClickListeners) {
                f(mousePosition);
            }
        } else {
            for(auto &f : onMouseReleaseListeners) {
                f(mousePosition);
            }
        }
    } else {
        rMouseDown = pressDown;
    }
}

void InputManager::OnMouseScroll(float scroll) {
    ImGuiIO &io = ImGui::GetIO();
    if(io.WantCaptureMouse) return;

    for(auto &f : onMouseScrollListeners) {
        f(scroll);
    }
}

void InputManager::Update(GLFWwindow* window) {
    ImGuiIO &io = ImGui::GetIO();
    /*if (io.WantCaptureKeyboard) {
        std::cout << "imgui steal" << std::endl;
    }*/

    // Handle mouse locking w/ f3
    bool prevF3 = f3;
    f3 = glfwGetKey(window, GLFW_KEY_F3);
    f3Pressed = f3 && !prevF3;
    bool cursorUnlocked = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
    if (f3Pressed) {
        glfwSetInputMode(window, GLFW_CURSOR, cursorUnlocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        cursorUnlocked = !cursorUnlocked;
    }

    // Update mouse
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    vec2 newPos = vec2(xpos, ypos) / vec2(screenDim);
    newPos.y = 1.0f - newPos.y;

    lockMouseDelta = cursorUnlocked ? vec3(0) : newPos - mousePosition;
    mousePosition = newPos;

    // Other keys
    w = glfwGetKey(window, GLFW_KEY_W);
    a = glfwGetKey(window, GLFW_KEY_A);
    s = glfwGetKey(window, GLFW_KEY_S);
    d = glfwGetKey(window, GLFW_KEY_D);
    space = glfwGetKey(window, GLFW_KEY_SPACE);
    ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
}