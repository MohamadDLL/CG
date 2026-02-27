#include <iostream>

// نستخدم GLEW و GLFW للتعامل مع OpenGL والنافذة
#define GLEW_STATIC
#include <../GL/glew.h>
#include <../GLFW/glfw3.h>

// أبعاد نافذة اللعبة
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ================= SHADERS =================

// Vertex Shader → مسؤول عن تحديد موقع كل نقطة
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"   // إحداثيات النقطة
"uniform vec2 offset;\n"                  // إزاحة لتحريك الشكل
"void main()\n"
"{\n"
// نضيف إحداثيات اللاعب لتحريك الشكل
"   gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, aPos.z, 1.0);\n"
"}\0";

// Fragment Shader → مسؤول عن اللون
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 shapeColor;\n"              // اللون الذي نرسله من C++
"void main()\n"
"{\n"
"   FragColor = shapeColor;\n"            // رسم الشكل باللون المحدد
"}\n\0";

// ================= Player Settings =================

// أبعاد الجسم والرأس
float bodyWidth = 0.08f;
float bodyHeight = 0.16f;
float headSize = 0.06f;

// مستوى الأرض
float groundLevel = -0.5f;

// مكان اللاعب (مركز الجسم)
float playerX = 0.0f;
float playerY = groundLevel + bodyHeight / 2; // حتى يقف فوق الأرض

// متغيرات الفيزياء
float velocityY = 0.0f;     // السرعة العمودية
float gravity = -0.0008f; // الجاذبية
float jumpForce = 0.035f;   // قوة القفز
bool isOnGround = true;     // هل اللاعب على الأرض؟

// ================= Input =================

void processInput(GLFWwindow* window)
{
    float speed = 0.001f; // سرعة الحركة الأفقية

    // حركة يسار
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        playerX -= speed;

    // حركة يمين
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        playerX += speed;

    // القفز (فقط إذا كان على الأرض)
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isOnGround)
    {
        velocityY = jumpForce; // إعطاء دفعة للأعلى
        isOnGround = false;    // منع القفز مرة أخرى
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
// ================= MAIN =================

int main()
{
    // تهيئة GLFW
    glfwInit();

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    // تحديد نسخة OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // إنشاء نافذة
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2D Game", NULL, NULL);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwMakeContextCurrent(window);

    // تهيئة GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    // ===== إنشاء الـ Shaders =====

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ================= الأرض =================

    float groundVertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  groundLevel, 0.0f,

        -1.0f, -1.0f, 0.0f,
         1.0f,  groundLevel, 0.0f,
        -1.0f,  groundLevel, 0.0f
    };

    unsigned int groundVAO, groundVBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ================= الجسم =================

    float bodyVertices[] = {
        -bodyWidth / 2, -bodyHeight / 2, 0.0f,
         bodyWidth / 2, -bodyHeight / 2, 0.0f,
         bodyWidth / 2,  bodyHeight / 2, 0.0f,

        -bodyWidth / 2, -bodyHeight / 2, 0.0f,
         bodyWidth / 2,  bodyHeight / 2, 0.0f,
        -bodyWidth / 2,  bodyHeight / 2, 0.0f
    };

    unsigned int bodyVAO, bodyVBO;
    glGenVertexArrays(1, &bodyVAO);
    glGenBuffers(1, &bodyVBO);

    glBindVertexArray(bodyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bodyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bodyVertices), bodyVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ================= الرأس =================

    float headVertices[] = {
        -headSize / 2, -headSize / 2, 0.0f,
         headSize / 2, -headSize / 2, 0.0f,
         headSize / 2,  headSize / 2, 0.0f,

        -headSize / 2, -headSize / 2, 0.0f,
         headSize / 2,  headSize / 2, 0.0f,
        -headSize / 2,  headSize / 2, 0.0f
    };

    unsigned int headVAO, headVBO;
    glGenVertexArrays(1, &headVAO);
    glGenBuffers(1, &headVBO);

    glBindVertexArray(headVAO);
    glBindBuffer(GL_ARRAY_BUFFER, headVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(headVertices), headVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ================= Render Loop =================

    while (!glfwWindowShouldClose(window))
    {
        processInput(window); // قراءة الإدخال

        // === الفيزياء ===
        velocityY += gravity;   // إضافة الجاذبية
        playerY += velocityY;   // تحريك اللاعب رأسياً

        // التصادم مع الأرض
        if (playerY <= groundLevel + bodyHeight / 2)
        {
            playerY = groundLevel + bodyHeight / 2; // تثبيت اللاعب
            velocityY = 0.0f;                     // إيقاف السقوط
            isOnGround = true;                    // السماح بالقفز
        }

        //  الشاشة بلون السماء
        glClearColor(0.4f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
        int colorLoc = glGetUniformLocation(shaderProgram, "shapeColor");

        // رسم الأرض
        glUniform2f(offsetLoc, 0.0f, 0.0f);
        glUniform4f(colorLoc, 0.3f, 0.2f, 0.1f, 1.0f);
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // رسم الجسم
        glUniform4f(colorLoc, 0.2f, 0.6f, 1.0f, 1.0f);
        glUniform2f(offsetLoc, playerX, playerY);
        glBindVertexArray(bodyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // رسم الرأس (فوق الجسم مباشرة)
        glUniform4f(colorLoc, 1.0f, 0.8f, 0.6f, 1.0f);
        glUniform2f(offsetLoc, playerX, playerY + bodyHeight / 2 + headSize / 2);
        glBindVertexArray(headVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window); //يعرض الإطار الجديد
        glfwPollEvents(); //يقرأ الأحداث
    }

    // حذف الأرض
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);

    // حذف الجسم
    glDeleteVertexArrays(1, &bodyVAO);
    glDeleteBuffers(1, &bodyVBO);

    // حذف الرأس
    glDeleteVertexArrays(1, &headVAO);
    glDeleteBuffers(1, &headVBO);

    // حذف برنامج الـ Shader
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}