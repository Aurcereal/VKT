#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <functional>

#include <GLFW/glfw3.h>

using namespace glm;
using namespace std;

class InputManager {
public:
    inline InputManager(uvec2 dim) : screenDim(dim),
        mousePosition(), lockMouseDelta(), f3(), f3Pressed(), w(), a(), s(), d(), space(), ctrl() { }

    void OnMouseMove(vec2 newPos);
    void OnMouseClick(bool leftMouse, bool pressDown);
    void OnMouseScroll(float scroll);

    void AddOnMouseMoveListener(const std::function<void(vec2, vec2)>&);
    void AddOnMouseClickListener(const std::function<void(vec2)>&);
    void AddOnMouseReleaseListener(const std::function<void(vec2)>&);
    void AddOnMouseScrollListener(const std::function<void(float)>&);
    
    void Update(GLFWwindow*);

    vec2 mousePosition;
    vec2 lockMouseDelta;
    bool lMouseDown = false;
    bool rMouseDown = false;

    bool f3, f3Pressed;
    bool w, a, s, d;
    bool space, ctrl;

    uvec2 screenDim;

private:
    vector<std::function<void(vec2, vec2)>> onMouseMoveListeners;
    vector<std::function<void(vec2)>> onMouseClickListeners;
    vector<std::function<void(vec2)>> onMouseReleaseListeners;
    vector<std::function<void(float)>> onMouseScrollListeners;
};