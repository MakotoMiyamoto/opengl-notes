// -- Must include glad before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace globals {
    int window_width = 800, window_height = 800;
    float background_color_rgb[3]{51.0f, 76.5f, 76.5f};
}

namespace callbacks {
    /*!
     * Called every time a window in the current context is resized. The purpose
     * of this callback is to update the window viewport whenever the window is
     * resized.
     * @param window the window from which the event was fired.
     * @param width new width of the window.
     * @param height new height of the window.
     */
    void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
        globals::window_width = width;
        globals::window_height = height;
        glViewport(0, 0, globals::window_width, globals::window_height);
    }
}

/**
 * Intermediary function for handling inputs. May or may not pass controls
 * to external events or something else.
 * @param window current window (there is actually no guarantee that this is
 * the current window but whatever)
 */
void processInput(GLFWwindow *window);

int main() {
    // --- Initialize GLFW (for OpenGL bindings)
    // -- Confirm GLFW initializes properly
    if (! glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return EXIT_FAILURE;
    }
    // -- Set GLFW version to 3.3 (What I'm using)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // -- Use the GLFW core profile, which gives you access to extra
    //    features or something else...
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // --- Initialize the GLFW window
    // -- Create the window
    GLFWwindow *window = glfwCreateWindow(
            globals::window_width,
            globals::window_height,
            "Makoto's OpenGL Notes",
            nullptr, nullptr);
    // -- Check if window was created properly
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window.\n";
        // -- Un-initialize GLFW and free all resources.
        glfwTerminate();
        return EXIT_FAILURE;
    }
    // -- Sets the window as the current window context
    glfwMakeContextCurrent(window);

    // -- Initialize GLAD and check for initialization error
    if (! gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // -- Set the viewport to define the size of the rendering window
    glViewport(0, 0, globals::window_width, globals::window_height);
    // -- Set framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, &callbacks::framebufferSizeCallback);

    // --- Main rendering loop
    // -- Runs as long as glfwWindowShouldClose(window) is false.
    while (! glfwWindowShouldClose(window)) {
        // -- User input
        processInput(window);

        // -- Change background color to confirm window works properly.
        glClearColor(globals::background_color_rgb[0] / 255.0f,
                     globals::background_color_rgb[1] / 255.0f,
                     globals::background_color_rgb[2] / 255.0f,
                     1.0f);
        // -- Clear the window to erase the previous frame
        glClear(GL_COLOR_BUFFER_BIT);

        // -- Swap buffers (you should read up on this, it's pretty interesting)
        glfwSwapBuffers(window);
        // -- Polls events that may or may not have callbacks (like the
        //    framebuffer resize event)
        glfwPollEvents();
    }

    glfwTerminate();
}

void processInput(GLFWwindow *window) {

}
