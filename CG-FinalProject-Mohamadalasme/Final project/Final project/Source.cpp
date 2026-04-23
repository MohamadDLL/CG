#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>


#define STB_IMAGE_IMPLEMENTATION
#include "../stb-master/stb_image.h"

// ==========================================
const unsigned int WIDTH = 1200;
const unsigned int HEIGHT = 750;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float startTime = 0.0f;

glm::vec3 playerPos(0.0f, 0.6f, 8.0f);
float speed = 5.0f;
bool isSprinting = false;
bool jumping = false;
float velocityY = 0.0f;

glm::vec3 goalPos(0.0f, 1.0f, -80.0f);

//coins
std::vector<glm::vec3> coins =
{
    glm::vec3(0, 0.8f, -10),
    glm::vec3(-2,0.8f,-25),
    glm::vec3(2,0.8f,-40),
    glm::vec3(0,0.8f,-58),
    glm::vec3(-1,0.8f,-72)
};

std::vector<bool> taken(5, false);

int collectedCoins = 0;
int totalCoins = 5;
bool doorUnlocked = false;

// Traps positions
glm::vec3 trap1(0.0f, 0.5f, -15.0f);
glm::vec3 trap2(-2.5f, 0.5f, -35.0f);
glm::vec3 trap3(2.5f, 0.5f, -55.0f);

glm::vec3 laser1(0.0f, 1.0f, -25.0f);
glm::vec3 laser2(0.0f, 1.0f, -65.0f);

// ==========================================
// Shaders
// ==========================================
const char* vs = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoord;
out vec2 TexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
})";

const char* fs = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D ourTexture;
uniform vec3 filterColor;
uniform bool useTexture;
void main() {
    if(useTexture) FragColor = texture(ourTexture, TexCoord) * vec4(filterColor, 1.0);
    else FragColor = vec4(filterColor, 1.0);
})";

// ==========================================
// Utility Functions
// ==========================================
unsigned int loadTexture(char const* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        stbi_image_free(data);
    }
    return textureID;
}

unsigned int createShader() {
    unsigned int v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vs, NULL); 
    glCompileShader(v);
    unsigned int f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fs, NULL); 
    glCompileShader(f);
    unsigned int s = glCreateProgram();
    glAttachShader(s, v); glAttachShader(s, f);
    glLinkProgram(s);
    return s;
}

void resetPlayer() {
    // 1. إعادة موقع اللاعب
    playerPos = glm::vec3(0.0f, 0.6f, 8.0f);

    // 2. تصفير عداد الكوينز المجمعة
    collectedCoins = 0;

    // 3. قفل الباب مرة أخرى
    doorUnlocked = false;

    // 4. إعادة جميع الكوينز لتصبح قابلة للجمع (taken = false)
    for (int i = 0; i < taken.size(); i++) {
        taken[i] = false;
    }
}

// ==========================================
// Core Logic
// ==========================================
void processInput(GLFWwindow* window) {
    isSprinting = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    float currentSpeed = isSprinting ? speed * 2.0f : speed;
    float move = currentSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        playerPos.z -= move;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        playerPos.z += move;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        playerPos.x -= move;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        playerPos.x += move;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !jumping) {
        jumping = true; 
        velocityY = 7.0f;
    }
}

void updateWorld() {
    // Player Jump
    if (jumping) {
        velocityY -= 12.0f * deltaTime;
        playerPos.y += velocityY * deltaTime;
        if (playerPos.y <= 0.6f) { playerPos.y = 0.6f; jumping = false; }
    }

    // Walls & Goal Collision
    if (playerPos.x < -4.0f || playerPos.x > 4.0f)
    {
        std::cout << "YOU LOST!\n";
        resetPlayer();
    }

    for (int i = 0; i < coins.size(); i++)
    {
        if (!taken[i] &&
            glm::distance(playerPos, coins[i]) < 1.2f)
        {
            taken[i] = true;
            collectedCoins++;

            std::cout << "Coin Collected! "
                << collectedCoins
                << "/"
                << totalCoins
                << "\n";
        }
    }
    if (collectedCoins == totalCoins)
        doorUnlocked = true;

    if (glm::distance(playerPos, goalPos) < 2.0f && !doorUnlocked)
    {
        std::cout << "Collect all coins first!\n";
        resetPlayer();
        
    }

    if (glm::distance(playerPos, goalPos) < 2.0f && doorUnlocked)
    {
        float finishTime = glfwGetTime() - startTime;

        std::cout << "YOU WIN!\n";
        std::cout << "Finished Time: "
            << finishTime
            << " seconds\n";

        resetPlayer();

        startTime = glfwGetTime(); // إعادة العد للجولة الجديدة
    }

    // Traps Animation & Collision
    trap1.x = sin(glfwGetTime()) * 3.0f;
    trap2.x = cos(glfwGetTime()) * 3.0f;
    trap3.x = sin(glfwGetTime() * 1.5f) * 3.0f;
    laser1.y = 1.0f + sin(glfwGetTime() * 2.0f) * 1.5f;
    laser2.y = 1.0f + cos(glfwGetTime() * 2.0f) * 1.5f;

    auto checkDie = [&](glm::vec3 tPos)
    { 
        return glm::distance(playerPos, tPos) < 1.3f;
    };
    if (checkDie(trap1) || checkDie(trap2) || checkDie(trap3))
    {
        std::cout << "YOU LOST!\n";
        resetPlayer();
    }
    // Laser Collision
    if ((abs(playerPos.z - laser1.z) < 1.0f && abs(playerPos.y - laser1.y) < 0.8f) ||
        (abs(playerPos.z - laser2.z) < 1.0f && abs(playerPos.y - laser2.y) < 0.8f))
    {
        std::cout << "YOU LOST!\n";
        resetPlayer();
    }

}

// ==========================================
// Rendering
// ==========================================
void drawCube(unsigned int shader, unsigned int VAO, glm::vec3 pos, glm::vec3 scale, glm::vec3 color, unsigned int tex, bool useTex, glm::mat4 v, glm::mat4 p) {
    glUseProgram(shader);
    glUniform3fv(glGetUniformLocation(shader, "filterColor"), 1, glm::value_ptr(color));
    glUniform1i(glGetUniformLocation(shader, "useTexture"), useTex);
    if (useTex) { glBindTexture(GL_TEXTURE_2D, tex); }

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, scale);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(v));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(p));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Dark Escape 3D", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();
    glEnable(GL_DEPTH_TEST);

    startTime = glfwGetTime();

    unsigned int shader = createShader();

    // Vertices with Texture Coords (UV)
    float vertices[] = {
        // المواقع (x, y, z)    // إحداثيات النسيج (u, v)
        // Back Face
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        // Front Face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        // Left Face
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        // Right Face
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         // Bottom Face (الأرضية - التكرار المناسب)
         -0.5f, -0.5f, -0.5f,  0.0f, 8.0f,
          0.5f, -0.5f, -0.5f,  1.5f, 8.0f,
          0.5f, -0.5f,  0.5f,  1.5f, 0.0f,
          0.5f, -0.5f,  0.5f,  1.5f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 8.0f,

         // Top Face
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // التحميل (تأكد من تسمية ملفاتك floor.jpg و wall.jpg بجانب الـ exe)
    unsigned int floorTex = loadTexture("floor33.png");
    unsigned int wallTex = loadTexture("wall.png");

    while (!glfwWindowShouldClose(window)) {
        float current = glfwGetTime();
        deltaTime = current - lastFrame; 
        lastFrame = current;

        // --- HUD التحديث المستمر لـ ---
        int timer = (int)(current - startTime);
        std::string missionStatus = doorUnlocked ? "GATE OPEN!" : "Locked";
        std::string title = "Dark Escape | Coins: " + std::to_string(collectedCoins) + "/" + std::to_string(totalCoins) +
            " | Time: " + std::to_string(timer) + "s | Goal: " + missionStatus;
        glfwSetWindowTitle(window, title.c_str());

        processInput(window); 
        updateWorld();

        //Sky
        glClearColor(0.08f, 0.12f, 0.25f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = glm::lookAt(playerPos + glm::vec3(0, 4, isSprinting ? 10 : 7), playerPos, glm::vec3(0, 1, 0));

        // Draw Floor (Textured)
        drawCube(shader, VAO, glm::vec3(0, -0.5, -35), glm::vec3(10, 0.1, 100), glm::vec3(1.0), floorTex, true, view, projection);

        // Draw Walls (Textured)
        for (int i = 10; i > -90; i -= 4) {
            drawCube(shader, VAO, glm::vec3(-5, 1, i), glm::vec3(1, 2, 2), glm::vec3(0.5, 0.1, 0.1), wallTex, true, view, projection);
            drawCube(shader, VAO, glm::vec3(5, 1, i), glm::vec3(1, 2, 2), glm::vec3(0.5, 0.1, 0.1), wallTex, true, view, projection);
        }

        // Draw Goal
        drawCube(shader, VAO, goalPos, glm::vec3(2, 3, 0.5), glm::vec3(1, 0.8, 0), 0, false, view, projection);

        // Draw Traps
        auto drawSpike = [&](glm::vec3 p) {
            drawCube(shader, VAO, p, glm::vec3(1, 0.4, 1), glm::vec3(0.8, 0, 0), 0, false, view, projection);
            drawCube(shader, VAO, p + glm::vec3(0, 0.6, 0), glm::vec3(0.4, 0.8, 0.4), glm::vec3(1, 0.2, 0.2), 0, false, view, projection);
        };
        drawSpike(trap1); 
        drawSpike(trap2); 
        drawSpike(trap3);

        // Draw Lasers
        drawCube(shader, VAO, laser1, glm::vec3(10, 0.15, 0.15), glm::vec3(1, 0, 0), 0, false, view, projection);
        drawCube(shader, VAO, laser2, glm::vec3(10, 0.15, 0.15), glm::vec3(1, 0, 0), 0, false, view, projection);

        // Draw Coins
        for (int i = 0; i < coins.size(); i++)
        {
            if (!taken[i])
                drawCube(shader, VAO,
                    coins[i],
                    glm::vec3(0.4f, 0.4f, 0.1f),
                    glm::vec3(1.0f, 0.85f, 0.0f),
                    0, false, view, projection);
        }

        // Draw Player parts
        drawCube(shader, VAO, playerPos, glm::vec3(0.8, 1, 0.4), glm::vec3(0.1, 0.4, 1), 0, false, view, projection); // Body
        drawCube(shader, VAO, playerPos + glm::vec3(0, 0.9, 0), glm::vec3(0.5, 0.5, 0.5), glm::vec3(1, 0.8, 0.6), 0, false, view, projection); // Head
        // --- رسم الأقدام هنا ---
        // القدم اليمنى
        drawCube(shader, VAO, playerPos + glm::vec3(0.2, -0.7, 0), glm::vec3(0.3, 0.4, 0.3), glm::vec3(0, 0, 0), 0, false, view, projection);
        // القدم اليسرى
        drawCube(shader, VAO, playerPos + glm::vec3(-0.2, -0.7, 0), glm::vec3(0.3, 0.4, 0.3), glm::vec3(0, 0, 0), 0, false, view, projection);

        glfwSwapBuffers(window); 
        glfwPollEvents();
    }
    glfwTerminate(); 
    return 0;
}