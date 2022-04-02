#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <freetype/freetype.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

namespace globals {
    int window_width = 800, window_height = 600;
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

std::string getShaderSource(const std::string &relativePath) {
//    std::cout << "This prints.\n";
//    std::ifstream shaderFile{relativePath};
//    std::cout << "This does not print.\n";
//    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
//    std::string source;
//    try {
//        std::stringstream shaderStream;
//        shaderStream << shaderFile.rdbuf();
//        shaderFile.close();
//        source = shaderStream.str();
//    } catch(std::ifstream::failure &e) {
//        std::cout << "ERROR: Cannot read shader file: " << e.what() << '\n';
//    }
//    return source;
    std::ifstream ifs;
    return "zilch";
}
#define INFO_LOG_SIZE 512
GLuint createShaderProgram(const char *vertexShaderFilepath, const char *fragmentShaderFilepath) {
    GLuint shaderProgramId = glCreateProgram();

    std::string vertexShaderSource = getShaderSource(vertexShaderFilepath);
    std::string fragmentShaderSource = getShaderSource(fragmentShaderFilepath);

    int success;
    char infoLog[INFO_LOG_SIZE];
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexShaderSourceRaw = vertexShaderSource.c_str();
    glShaderSource(vertexShaderId, 1, &vertexShaderSourceRaw, nullptr);
    glCompileShader(vertexShaderId);
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (! success) {
        glGetShaderInfoLog(vertexShaderId, INFO_LOG_SIZE, nullptr, infoLog);
        std::cout << "Error compiling vertex shader: " << infoLog << '\n';
    }

    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSourceRaw = fragmentShaderSource.c_str();
    glShaderSource(fragmentShaderId, 1, &fragmentShaderSourceRaw, nullptr);
    glCompileShader(fragmentShaderId);
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (! success) {
        glGetShaderInfoLog(fragmentShaderId, INFO_LOG_SIZE, nullptr, infoLog);
        std::cout << "Error compiling fragment shader: " << infoLog << '\n';
    }

    glAttachShader(shaderProgramId, vertexShaderId);
    glAttachShader(shaderProgramId, fragmentShaderId);
    glLinkProgram(shaderProgramId);
    glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success);
    if (! success) {
        glGetProgramInfoLog(shaderProgramId, INFO_LOG_SIZE, nullptr, infoLog);
        std::cout << "Error linking shader program: " << infoLog << '\n';
    }

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return shaderProgramId;
}

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    FT_Pos       Advance;    // Offset to advance to next glyph
};
std::map<char, Character> Characters;
GLuint textVAO, textVBO;

/**
 * Intermediary function for handling inputs. May or may not pass controls
 * to external events or something else.
 * @param window current window (there is actually no guarantee that this is
 * the current window but whatever)
 */
void processInput(GLFWwindow *window);

void renderText(GLuint shader, const std::string& text, float x, float y, float scale, glm::vec3 color);

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

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Error loading freetype.\n";
        return EXIT_FAILURE;
    }

    FT_Face face;
    if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face)) {
        std::cerr << "Failed to load font.\n";
        return EXIT_FAILURE;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load glyph " << c << '\n';
            continue;
        }
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLuint textShader = createShaderProgram("shaders/vertex.glsl", "shaders/frag.glsl");

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

        renderText(textShader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        renderText(textShader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

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

void renderText(GLuint shader, const std::string& text, float x, float y, float scale, glm::vec3 color) {
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glm::mat4 projection = glm::ortho(
            0.0f,
            static_cast<float>(globals::window_width),
            0.0f,
            static_cast<float>(globals::window_height)
    );
    glUniformMatrix4fv(
            glGetUniformLocation(shader, "projection"),
            1,
            GL_FALSE,
            glm::value_ptr(projection)
            );
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    for (char c : text) {
        Character ch = Characters[c];

        float x_pos = x + static_cast<float>(ch.Bearing.x) * scale;
        float y_pos = y - static_cast<float>(ch.Size.y - ch.Bearing.y) * scale;

        float w = static_cast<float>(ch.Size.x) * scale;
        float h = static_cast<float>(ch.Size.y) * scale;

        float vertices[6][4] = {
                { x_pos,     y_pos + h,   0.0f, 0.0f },
                { x_pos,     y_pos,       0.0f, 1.0f },
                { x_pos + w, y_pos,       1.0f, 1.0f },

                { x_pos,     y_pos + h,   0.0f, 0.0f },
                { x_pos + w, y_pos,       1.0f, 1.0f },
                { x_pos + w, y_pos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
