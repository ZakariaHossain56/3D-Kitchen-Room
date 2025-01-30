

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "basic_camera.h"
#include "camera.h"

#include<iostream>
#include<vector>
#include<cmath>

using namespace std;

#define M_PI 3.14159265358979323846



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void drawFan(unsigned int VAO, Shader ourShader, glm::mat4 matrix);
void drawFurnace(unsigned int VAO, Shader ourShader, glm::mat4 matrix);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


//cylinder
const int SEGMENTS = 36; // Number of segments for the circle
const float RADIUS = 0.5f;
const float HEIGHT = 2.0f;

// Vertices and indices arrays
float lampVertices[(SEGMENTS + 1) * 6 * 2 + 12]; // (Position + Normal) * 2 (top and bottom) + 2 center points
unsigned int lampIndices[SEGMENTS * 12];        // Indices for sides and caps

float eyeX = 5.0f, eyeY = 5.0f, eyeZ = 25.0f;
float lookAtX = 5.25f, lookAtY = 4.0f, lookAtZ = 5.0f;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 eye = glm::vec3(eyeX, eyeY, eyeZ);
glm::vec3 lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);
Camera camera(glm::vec3(eyeX, eyeY, eyeZ));

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

float r = 0.0f;
bool fanOn = false;
float aspectRatio = 4.0f / 3.0f;

bool birdEyeView = false;
glm::vec3 birdEyePosition(5.25f, 10.0f, 10.0f); // Initial position (10 units above)
glm::vec3 birdEyeTarget(5.25f, 0.0f, 6.0f);   // Focus point
float birdEyeSpeed = 1.0f;                   // Speed of movement

float theta = 0.0f; // Angle around the Y-axis
float radius = 2.0f; // Distance from lookAt point

void drawChair(unsigned int VAO, Shader ourShader, glm::mat4 matrix);
void drawTable(unsigned int VAO, Shader ourShader, glm::mat4 matrix);
void drawCylinder(Shader shaderProgram, glm::vec4 color, unsigned int VAO, glm::mat4 parentTrans, float posX = 0.0, float posY = 0.0, float posZ = 0.0, float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0, float scX = 1.0, float scY = 1.0, float scZ = 1.0);



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1907056: Assignment 2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("vertexShader.vs", "fragmentShader.fs");

    Shader constantShader("vertexShader.vs", "fragmentShaderV2.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cube_vertices[] = {
        0.0f, 0.0f, 0.0f, 0.647f, 0.408f, 0.294f,
        1.0f, 0.0f, 0.0f, 0.647f, 0.408f, 0.294f,
        1.0f, 1.0f, 0.0f, 0.647f, 0.408f, 0.294f,
        0.0f, 1.0f, 0.0f, 0.647f, 0.408f, 0.294f,

        0.0f, 0.0f, 1.0f, 0.647f, 0.408f, 0.294f,
        1.0f, 0.0f, 1.0f, 0.647f, 0.408f, 0.294f,
        1.0f, 1.0f, 1.0f, 0.647f, 0.408f, 0.294f,
        0.0f, 1.0f, 1.0f, 0.647f, 0.408f, 0.294f,

    };

    unsigned int cube_indices[] = {
        1, 2, 3,
        3, 0, 1,

        5, 6, 7,
        7, 4, 5,

        4, 7, 3,
        3, 0, 4,

        5, 6, 2,
        2, 1, 5,

        5, 1, 0,
        0, 4, 5,

        6, 2, 3,
        3, 7, 6,
    };


    float cylinder_vertices[] = {
        // Top circle vertices (radius = 0.5, y = 0.5)
        0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // Center of top circle (red)

        // Top circle vertices (more vertices for roundness)
        0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // Right edge of top circle (red)
        0.353f,  0.5f,  0.353f, 1.0f, 0.0f, 0.0f, // Top-right edge (red)
        0.0f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-center (red)
        -0.353f,  0.5f,  0.353f, 1.0f, 0.0f, 0.0f, // Left-top edge (red)
        -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // Left edge of top circle (red)

        // More vertices for top circle to make it rounder
        -0.353f,  0.5f,  -0.353f, 1.0f, 0.0f, 0.0f, // Left-bottom edge (red)
        0.0f,  0.5f,  -0.5f, 1.0f, 0.0f, 0.0f, // Bottom-center (red)
        0.353f,  0.5f,  -0.353f, 1.0f, 0.0f, 0.0f, // Right-bottom edge (red)

        // Bottom circle vertices (radius = 0.5, y = -0.5)
        0.0f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, // Center of bottom circle (green)

        // Bottom circle vertices (more vertices for roundness)
        0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, // Right edge of bottom circle (green)
        0.353f, -0.5f,  0.353f, 0.0f, 1.0f, 0.0f, // Bottom-right edge (green)
        0.0f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Bottom-center (green)
        -0.353f, -0.5f,  0.353f, 0.0f, 1.0f, 0.0f, // Left-bottom edge (green)
        -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, // Left edge of bottom circle (green)

        // More vertices for bottom circle to make it rounder
        -0.353f, -0.5f, -0.353f, 0.0f, 1.0f, 0.0f, // Left-top edge (green)
        0.0f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Bottom-center (green)
        0.353f, -0.5f, -0.353f, 0.0f, 1.0f, 0.0f  // Right-bottom edge (green)
    };


    unsigned int cylinder_indices[] = {
        // Top circle
        0,  1,  2,
        0,  2,  3,
        0,  3,  4,
        0,  4,  5,
        0,  5,  6,
        0,  6,  7,
        0,  7,  8,
        0,  8,  1,

        // Bottom circle
        9,  10,  11,
        9,  11,  12,
        9,  12,  13,
        9,  13,  14,
        9,  14,  15,
        9,  15,  16,
        9,  16,  17,
        9,  17,  10,

        // Side faces
        1,  10,  2,
        2,  10,  11,
        2,  11,  3,
        3,  11,  12,
        3,  12,  4,
        4,  12,  13,
        4,  13,  5,
        5,  13,  14,
        5,  14,  6,
        6,  14,  15,
        6,  15,  7,
        7,  15,  16,
        7,  16,  8,
        8,  16,  17,
        8,  17,  1
    };


    
    


    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);


    //for cylinder
    unsigned int cylinder_VBO, cylinder_VAO, cylinder_EBO;
    glGenVertexArrays(1, &cylinder_VAO);
    glGenBuffers(1, &cylinder_VBO);
    glGenBuffers(1, &cylinder_EBO);

    glBindVertexArray(cylinder_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, cylinder_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cylinder_vertices), cylinder_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cylinder_indices), cylinder_indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);






    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    ourShader.use();
    glBindVertexArray(cylinder_VAO);
    glDrawElements(GL_TRIANGLES, sizeof(cylinder_indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
    //constantShader.use();

    r = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        //glm::mat4 projection = glm::perspective(glm::radians(basic_camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::ortho(-2.0f, +2.0f, -1.5f, +1.5f, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        //constantShader.setMat4("projection", projection);

        // camera/view transformation
        //glm::mat4 view = basic_camera.createViewMatrix();
        //ourShader.setMat4("view", view);
        //constantShader.setMat4("view", view);

        glm::mat4 view;

        if (birdEyeView) {
            // Set camera position directly above the scene
            glm::vec3 up(0.0f, 1.0f, 0.0f); // Ensure the up vector points backward
            view = glm::lookAt(birdEyePosition, birdEyeTarget, up);
        }
        else {
            view = camera.GetViewMatrix();
        }

        ourShader.setMat4("view", view);
        // Modelling Transformation
        //glm::mat4 identityMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        //drawCube(ourShader, VAO, identityMatrix, translate_X, translate_Y, translate_Z, rotateAngle_X, rotateAngle_Y, rotateAngle_Z, scale_X, scale_Y, scale_Z);
        /*
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
        translateMatrix = glm::translate(identityMatrix, glm::vec3(translate_X, translate_Y, translate_Z));
        rotateXMatrix = glm::rotate(identityMatrix, glm::radians(rotateAngle_X), glm::vec3(1.0f, 0.0f, 0.0f));
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(rotateAngle_Y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotateZMatrix = glm::rotate(identityMatrix, glm::radians(rotateAngle_Z), glm::vec3(0.0f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(scale_X, scale_Y, scale_Z));
        model = identityMatrix;
        */
        //model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
        //modelCentered = glm::translate(model, glm::vec3(-0.25, -0.25, -0.25));

        //shaderProgram.setMat4("model", modelCentered);

        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;





        //first wall section 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 7.2f, 10.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //first wall section 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 7.2f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 2.8f, 10.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.88, 0.75f, 0.64f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //second wall section 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.5f, 7.2f, -0.1f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));




        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //second wall section 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 7.2f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.5f, 2.8f, -0.1f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.88, 0.75f, 0.64f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //roof
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-0.1f, 10.0f, -0.1f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.5f, 0.1f, 10.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.678, 0.678, 0.788, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //floor
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-0.1f, 0.0f, -0.1f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.5f, -0.1f, 10.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.85, 0.65f, 0.56f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //upper box 1
        
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, 2.0f, 2.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //upper boxes
        float z = 2.01f;
        float z_thin = 2.0f;
        for (int i = 0; i < 5; i++) {
            //thin separator
            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.51f, 3.7f, z_thin));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 2.0f, 0.02f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            z_thin += 1.0f;


            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 3.7f, z));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, 2.0f, 0.98f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            z += 1.0f;
        }
        
        
        //white surface 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 2.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 0.2f, 10.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //lamp base
        translateMatrix = glm::translate(identityMatrix, glm::vec3(1.30f, 2.2f, 0.30f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, 0.1f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //lamp stick
        translateMatrix = glm::translate(identityMatrix, glm::vec3(1.35f, 2.21f, 0.35f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 0.6f, 0.1f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        //lamp
        drawCylinder(ourShader, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), cylinder_VAO, identityMatrix, 1.5f, 2.81f, 0.5f, 0, 0, 0, 0.4f, 0.5f, 0.4f);

        //oven structure
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 2.2f, 3.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.8f, 0.8f, 2.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.52, 0.45f, 0.39f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //oven black left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.81f, 2.25f, 3.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 0.7f, 1.4f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.047, 0.031f, 0.031f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //oven screen
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.81f, 2.75f, 3.10f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 0.15f, 0.3f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.047, 0.031f, 0.031f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //oven keypad
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.81f, 2.40f, 3.10f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 0.3f, 0.3f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.87, 0.79f, 0.72f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //electric stove
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.1f, 2.21f, 6.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.9f, 0.01f, 0.8f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.047, 0.031f, 0.031f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //stove furnace 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.18f, 2.25f, 6.05f));
        drawFurnace(VAO, ourShader, translateMatrix);

        //stove furnace 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.58f, 2.25f, 6.05f));
        drawFurnace(VAO, ourShader, translateMatrix);

        //stove furnace 3
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.18f, 2.25f, 6.45f));
        drawFurnace(VAO, ourShader, translateMatrix);

        //stove furnace 4
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.58f, 2.25f, 6.45f));
        drawFurnace(VAO, ourShader, translateMatrix);


        /*translateMatrix = glm::translate(identityMatrix, glm::vec3(0.15f, 2.25f, 6.05f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.35f, 0.01f, 0.35f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

       


        //white surface 2

        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 2.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(7.0f, 0.2f, 1.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        //ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));
        ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //lower box 1

        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 2.0f, 1.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //lower boxes (2-4)
        float z_lower = 2.01f;
        float z_thin_lower = 2.0f;
        for (int i = 0; i < 3; i++) {
            //thin separator
            translateMatrix = glm::translate(identityMatrix, glm::vec3(1.01f, 0.0f, z_thin_lower));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 2.0f, 0.02f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            z_thin_lower += 1.0f;


            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, z_lower));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 2.0f, 0.98f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            z_lower += 1.0f;
        }

        
        //store bottom
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 5.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 0.4f, 2.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.64, 0.58f, 0.49f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //store top
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 1.6f, 5.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 0.4f, 2.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.64, 0.58f, 0.49f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //store mid
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.4f, 5.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 1.2f, 2.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //store handle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(1.01f, 1.2f, 5.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 0.05f, 1.65f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //lower boxes (5-7)
        float z_lower_left = 7.02f;
        float z_thin_lower_left = 7.0f;
        for (int i = 0; i < 3; i++) {
            //thin separator
            translateMatrix = glm::translate(identityMatrix, glm::vec3(1.01f, 0.0f, z_thin_lower_left));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 2.0f, 0.02f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            z_thin_lower_left += 1.0f;


            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, z_lower_left));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 2.0f, 0.98f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            z_lower_left += 1.0f;
        }



        //lower boxes right (1-6)
        float x_lower_right = 1.12f;
        float x_thin_lower_right = 1.1f;
        for (int i = 0; i < 6; i++) {
            //thin separator
            translateMatrix = glm::translate(identityMatrix, glm::vec3(x_thin_lower_right, 0.0f,1.01f));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.02f, 2.0f, 0.01f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            x_thin_lower_right += 1.0f;


            translateMatrix = glm::translate(identityMatrix, glm::vec3(x_lower_right, 0.0f, 0.0f));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.98f, 2.0f, 1.0f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            x_lower_right += 1.0f;
        }

        //upper box right 1

        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //upper box right 1 white front

        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.9f, 3.9f, 1.02f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.9f, 1.6f, 0.02f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.85, 0.65f, 0.56f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);



        //window mid left

        translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.15f, 2.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //window mid 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.5f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.15f, 2.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //window mid right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(6.05f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.15f, 2.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //window top left 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0f, 5.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.7f, 0.15f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //window top right 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.5f, 5.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.7f, 0.15f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //window bottom left 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.7f, 0.15f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //window bottom right 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.5f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.7f, 0.15f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //fridge back
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.1f, 0.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(3.0f, 5.0f, 1.0f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.65, 0.57f, 0.47f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


        //fridge front left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.1f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.2f, 5.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.65, 0.57f, 0.47f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fridge separator line
        translateMatrix = glm::translate(identityMatrix, glm::vec3(8.3f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.02f, 5.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fridge front right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(8.32f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.78f, 5.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.65, 0.57f, 0.47f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fridge left handle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(8.0f, 1.9f, 1.21f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 2.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fridge right handle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(8.5f, 1.9f, 1.21f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 2.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fridge password
        translateMatrix = glm::translate(identityMatrix, glm::vec3(9.0f, 2.5f, 1.21f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.6f, 0.4f, 0.01f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.87, 0.79f, 0.72f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //sink
        translateMatrix = glm::translate(identityMatrix, glm::vec3(2.5f, 2.21f, 0.05f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(2.0f, 0.01f, 0.9f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //sink lines
        float sink_z = 0.1f;
        for (int i = 0; i < 10; i++) {
            translateMatrix = glm::translate(identityMatrix, glm::vec3(2.55f, 2.22f, sink_z));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.8f, 0.01f, 0.02f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(1.0, 1.0f, 1.0f, 1.0f));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            sink_z += 0.1;
        }
        


        

        //tap 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 2.21f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.05f, 1.0f, 0.05f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.5, 0.5f, 0.5f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //tap 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 3.21f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.05f, 0.05f, 0.5f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.5, 0.5f, 0.5f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //tap 3
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 3.26f, 0.7f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.05f, -0.5f, 0.05f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.5, 0.5f, 0.5f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);



        //draw table
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 2.0f, 5.0f));
        drawTable(VAO, ourShader, translateMatrix);

        //draw chair 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.5f, 1.5f, 7.5f));
        drawChair(VAO, ourShader, translateMatrix);

        //draw chair 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(6.3f, 1.5f, 7.5f));
        drawChair(VAO, ourShader, translateMatrix);

        //draw chair 3
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.7f, 1.5f, 4.5f));
        drawChair(VAO, ourShader, translateMatrix* rotateYMatrix);

        //draw chair 4
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.4f, 1.5f, 4.5f));
        drawChair(VAO, ourShader, translateMatrix* rotateYMatrix);


        //draw fan with rotations
        //translateMatrix = glm::translate(identityMatrix, glm::vec3(6.0f, 3.0f, 7.0f));
        drawFan(VAO, ourShader, identityMatrix);
        

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void drawFan(unsigned int VAO, Shader ourShader, glm::mat4 matrix) {
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;

    if (fanOn) {
        //fan rod
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.15f, 10.0f, 4.9f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan middle
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(r), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.85f, 8.5f, 4.6f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.4f, 0.0f, -0.4f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.4f, 0.0f, 0.4f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.8f, -0.2f, 0.8f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.85f, 8.5f, 4.8f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.4f, 0.0f, -0.2f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.4f, 0.0f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-1.5f, -0.2f, 0.4f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.65f, 8.5f, 4.8f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.4f, 0.0f, -0.2f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.4f, 0.0f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.5f, -0.2f, 0.4f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars up
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.05f, 8.5f, 4.6f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.2f, 0.0f, -0.4f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.2f, 0.0f, 0.4f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.2f, -1.5f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars down
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.05f, 8.5f, 5.4f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.2f, 0.0f, 0.4f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.2f, 0.0f, -0.4f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.2f, 1.5f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        r += 0.75f;
    }

    else {
        //fan rod
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.15f, 10.0f, 4.9f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan middle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.85f, 8.5f, 4.6f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.8f, -0.2f, 0.8f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.85f, 8.5f, 4.8f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-1.5f, -0.2f, 0.4f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.65f, 8.5f, 4.8f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.5f, -0.2f, 0.4f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars up
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.05f, 8.5f, 4.6f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.2f, -1.5f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars down
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.05f, 8.5f, 5.4f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.2f, 1.5f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
}
void drawChair(unsigned int VAO, Shader ourShader, glm::mat4 matrix) {
    //chair sit
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.2f, 0.2f, 1.2f));
    model = matrix * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("color", glm::vec4(0.09f, 0.08f, 0.07f, 1.0f));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair left back leg
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //chair right back leg
    translateMatrix = glm::translate(identityMatrix, glm::vec3(1.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair left front leg
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //chair right front leg
    translateMatrix = glm::translate(identityMatrix, glm::vec3(1.0f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair left hand
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, 1.8f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //chair right hand
    translateMatrix = glm::translate(identityMatrix, glm::vec3(1.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, 1.8f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair top hand
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 1.8f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.2f, 0.2f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair mid hand
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.8f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.2f, 0.2f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    
}
void drawTable(unsigned int VAO, Shader ourShader, glm::mat4 matrix) {
    //table top
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(4.0f, 0.2f, 2.0f));
    model = matrix * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("color", glm::vec4(0.09f, 0.08f, 0.07f, 1.0f));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //table front leg left
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 1.8f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -2.0f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //table front leg right
    translateMatrix = glm::translate(identityMatrix, glm::vec3(3.8f, 0.0f, 1.8f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -2.0f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //table back leg left
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -2.0f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //table back leg right
    translateMatrix = glm::translate(identityMatrix, glm::vec3(3.8f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -2.0f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

 
}

void drawCylinder(Shader shaderProgram, glm::vec4 color, unsigned int cylinderVAO, glm::mat4 parentTrans,
    float posX, float posY, float posZ,
    float rotX, float rotY, float rotZ,
    float scX, float scY, float scZ) {
    shaderProgram.use();

    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered;

    // Transformations for the cylinder
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));

    // Adjust to center the cylinder if needed
    modelCentered = glm::translate(model, glm::vec3(-0.25f, 0.0f, -0.25f));

    // Pass the model matrix to the shader
    shaderProgram.setMat4("model", modelCentered);

    // Set the color uniform
    glUniform4fv(glGetUniformLocation(shaderProgram.ID, "color"), 1, glm::value_ptr(color));

    // Draw the cylinder
    glBindVertexArray(cylinderVAO);
    glDrawElements(GL_TRIANGLES, sizeof(lampIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
}


void drawFurnace(unsigned int VAO, Shader ourShader, glm::mat4 matrix) {
    
    //z1
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.02f, 0.01f, 0.3f));
    model = matrix * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //z2
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.3f, 0.0f,0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.02f, 0.01f, 0.3f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //x1
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.3f, 0.01f, 0.02f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //x2
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.3f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.35f, 0.01f, 0.02f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    


}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera.ProcessKeyboard(UP, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        camera.ProcessKeyboard(DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        camera.ProcessKeyboard(P_UP, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        camera.ProcessKeyboard(P_DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        camera.ProcessKeyboard(Y_LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        camera.ProcessKeyboard(Y_RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        camera.ProcessKeyboard(R_LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        camera.ProcessKeyboard(R_RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        if (fanOn) {
            fanOn = false;
        }
        else {
            fanOn = true;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        birdEyeView = !birdEyeView;
    }

    if (birdEyeView) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            birdEyePosition.z -= birdEyeSpeed * deltaTime; // Move forward along Z
            birdEyeTarget.z -= birdEyeSpeed * deltaTime;
            if (birdEyePosition.z <= 4.0) {
                birdEyePosition.z = 4.0;
            }
            if (birdEyeTarget.z <= 0.0) {
                birdEyeTarget.z = 0.0;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            birdEyePosition.z += birdEyeSpeed * deltaTime; // Move backward along Z
            birdEyeTarget.z += birdEyeSpeed * deltaTime;
            if (birdEyePosition.z >= 10.0) {
                birdEyePosition.z = 10.0;
            }
            if (birdEyeTarget.z >= 6.0) {
                birdEyeTarget.z = 6.0;
            }
        }
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!birdEyeView) {
            theta += 0.01f; // Increment angle to rotate
            // Calculate new eye position using spherical coordinates
            camera.Position.x = lookAtX + radius * sin(theta);
            camera.Position.y = lookAtY;
            camera.Position.z = lookAtZ + radius * cos(theta);
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    float aspectRatio = 4.0f / 3.0f;
    int viewWidth, viewHeight;

    if (width / (float)height > aspectRatio) {
        // Window is too wide, fit height and adjust width
        viewHeight = height;
        viewWidth = (int)(height * aspectRatio);
    }
    else {
        // Window is too tall, fit width and adjust height
        viewWidth = width;
        viewHeight = (int)(width / aspectRatio);
    }

    // Center the viewport
    int xOffset = (width - viewWidth) / 2;
    int yOffset = (height - viewHeight) / 2;

    glViewport(xOffset, yOffset, viewWidth, viewHeight);
    //glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}