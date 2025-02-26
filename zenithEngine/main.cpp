#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include<stb/stb_image.h>
#include<iostream>
#include"funcs.h"

// camera
float sensitivity = 0.1f; // change this value to your liking

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

// floor vars
glm::vec3 floorPos = glm::vec3 (0.0f,-2.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInputReg(GLFWwindow* window);


int main() {
    // initialize glfw
    init(4,6);
    //initialize window
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, glfwGetPrimaryMonitor(), NULL);
    if(window==nullptr) {
        std::cerr << "failed to open window" << std::endl;
    }
    //link GLFW to GLAD
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "failed to initialize OpenGL" << std::endl;
    }
    //tell OpenGL which pixels we're using
    glViewport(0,0,windowWidth,windowHeight);

    //shader error variables
    int success;
    char infoLog[512];
    //vShader handling
    uint32_t vShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vShader, 1, &vertSource, NULL);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vShader, 512, NULL, infoLog);
        std::cout << "ERROR::vShader::COMPILATION::FAILED" << infoLog << std::endl;
    }
    //fShader handling
    uint32_t fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragSource, NULL);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fShader, 512, NULL, infoLog);
        std::cout << "ERROR::fShader::COMPILATION::FAILED" << infoLog << std::endl;
    }
    //link and create the actual shader into one program/one instruction
    uint32_t sProgram = glCreateProgram();
    glAttachShader(sProgram, vShader);
    glAttachShader(sProgram, fShader);
    glLinkProgram(sProgram);
    glGetProgramiv(sProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(sProgram, 512, NULL, infoLog);
        std::cout << "ERROR::sProgram::COMPILATION::FAILED" << infoLog << std::endl;
    }
    //delete shaders after they've been processed
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    //vertex array, vertex buffer, index buffer
    GLuint vArray,vBuffer,iBuffer;

    //generating buffer blueprints
    glGenVertexArrays(1, &vArray);
    glGenBuffers(1, &vBuffer);
    glGenBuffers(1, &iBuffer);

    //binding buffers
    glBindVertexArray(vArray);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //0==location in shader, 3==how many values per attrib (3 vertices per line), GL_FLOAT==type of attrib, GL_FALSE==is it normalized? no., 3 * sizeof(GLfloat)==how many values in each line, (void*)0==offset for values (the vertex values start at 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    GLuint texture1;

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("wall.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else if (!data) {
        unsigned char* data2 = stbi_load("missingTexture.png", &width, &height, &nrChannels, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);



    //enabling neccesities
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glfwSwapInterval(0);

    //TEXTURE AND/OR UNIFORM HANDLING
    glUseProgram(sProgram);
    glUniform1i(glGetUniformLocation(sProgram, "texture1"), 0);

    GLuint modelLoc = glGetUniformLocation(sProgram, "model");
    GLuint viewLoc = glGetUniformLocation(sProgram, "view");
    GLuint projLoc = glGetUniformLocation(sProgram, "projection");
    //int vertexColorLocation = glGetUniformLocation(sProgram, "changingColor");

    //main loop of the window
    while(!glfwWindowShouldClose(window)) {

        //timing and deltaTime
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // clear buffers and window color
        clear(0.f,0.f,0.f,1.f);
        //shader handling
        glUseProgram(sProgram);
        //projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
        //view matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        /*
        float timeValue = glfwGetTime();
        float pinkValue = (sin(timeValue)/2.0f) + 0.5f;
        glUniform4f(vertexColorLocation, pinkValue, 0.f, pinkValue, 1.f);
        */
        glBindVertexArray(vArray);
        //model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.1f, 0.1f, 0.1f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //GL_TRIANGLES (what its drawing), 6 (number of actual indices values), GL_UNSIGNED_INT (what type is the indices), 0 (location of the vertices in the shader (location of vPos))
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glm::mat4 floorModel = glm::mat4(1.0f);
        floorModel = glm::scale(floorModel, glm::vec3(50.0f,1.0f,50.0f));
        floorModel = glm::translate(floorModel, glm::vec3(0.0f, -1.0f, 0.0f));
        floorModel = glm::rotate(floorModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(floorModel));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //input handling
        processInputReg(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //destroy everything to prevent memory leaks
    glDeleteVertexArrays(1, &vArray);
    glDeleteBuffers(1, &vBuffer);
    glDeleteBuffers(1, &iBuffer);
    glDeleteProgram(sProgram);
    glfwTerminate();
    return 0;
}

void processInputReg(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;
    glm::vec3 lockedCameraFront = glm::vec3(cameraFrontX, 0.0f, cameraFrontZ);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * lockedCameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * lockedCameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        sensitivity += 0.0001f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        sensitivity -= 0.0001f;

}

void processInputDebug(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        sensitivity += 0.0001f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        sensitivity -= 0.0001f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.9f)
        pitch = 89.9f;
    if (pitch < -89.9f)
        pitch = -89.9f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}
