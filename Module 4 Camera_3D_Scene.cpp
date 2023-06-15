//Brandon Hobbs
//CS-330
//May 16, 2023
//
//Module 4: 3D Scene with Camera

#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include <vector>
#include "meshes.h"


// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Module 3 3D Scene BHobbs"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    //camera initial settings
    // camera
    glm::vec3 cameraPos = glm::vec3(2.5f, 3.0f, 12.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    bool firstMouse = true;
    bool orthoOn = false;
    glm::mat4 projection; //move to a global to set from a keystroke

    float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    float pitch = 0.0f;
    float lastX = WINDOW_WIDTH / 2.0;
    float lastY = WINDOW_HEIGHT / 2.0;
    float fov = 45.0f;
    float sensitivity = 0.03f; // change this to a global with a default; to be controlled by scroll

    // timing
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    //Generic primative shape mesh
    Meshes meshes;

    // Shader program
    GLuint gProgramId;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

//add the new prototypes for the keys and mouse controls
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1

out vec4 vertexColor; // variable to transfer color data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
}
);

/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;
uniform vec4 uObjectColor;

void main()
{
    //fragmentColor = vec4(vertexColor);
    fragmentColor = vec4(uObjectColor);
}
);

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Create the mesh
    //UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
    meshes.CreateMeshes();

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        // Turn on wireframe mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //use wireframe for QA GL_FILL for off GL_LINE for on
        URender();

        glfwPollEvents();
    }


    //delete the meshes
    meshes.DestroyMeshes();

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    //Register the new callbacks
    glfwSetCursorPosCallback(*window, mouseCallback);
    glfwSetScrollCallback(*window, scrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);
    glfwSetKeyCallback(*window, keyCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    //TODO
    //WASD Keys to pan Left, Tight, forward, back
    float cameraSpeed = static_cast<float>(25 * deltaTime * sensitivity);
    glm::vec3 front;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        pitch += cameraSpeed*5;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        pitch -= cameraSpeed*5;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        //toggle the orthographic view
        //set the ortho view then flip the global variable
        if (orthoOn) {
            orthoOn = false;
            cout << "Ortho State" << orthoOn << endl;
        }
        else {
            orthoOn = true;

            cout << "Ortho State" << orthoOn << endl;
        }
    }
}

// FROM: https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.3.camera_mouse_zoom/camera_mouse_zoom.cpp
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    //fov -= (float)yoffset;
    //if (fov < 1.0f)
    //    fov = 1.0f;
    //if (fov > 90.0f)
    //    fov = 90.0f;
    sensitivity += 0.01 * (float)yoffset;;
    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (sensitivity > 3.0f)
        sensitivity = 3.0f;
    if (sensitivity < 0.01f)
        sensitivity = 0.01f;

    cout << sensitivity << endl;
}

// glfw: whenever the mouse moves, this callback is called
// FROM: https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/7.3.camera_mouse_zoom/camera_mouse_zoom.cpp
// -------------------------------------------------------
void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
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
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS) {
            cout << "Middle mouse button pressed - sensitivity reset" << endl;
            sensitivity = 0.03f;
        }


        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Functioned called to render a frame
void URender()
{

    glm::mat4 scale;
    glm::mat4 rotation;
    glm::mat4 translation;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 worldView;
    //glm::mat4 projection;
    GLint modelLoc;
    GLint viewLoc;
    GLint projLoc;
    GLint objectColorLoc;

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Transforms the camera: move the camera back (z axis)
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Creates a orthographic projection
    //projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f); //use the orthographic projection for QA

    if (orthoOn){
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f); //use the orthographic projection for QA
    }      
    else 
        projection = glm::perspective(glm::radians(fov), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");
    objectColorLoc = glGetUniformLocation(gProgramId, "uObjectColor");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //
    //Create the Plane
    //
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPlaneMesh.vao);
    // 1. Scales the object
    scale = glm::scale(glm::vec3(5.0f, 1.0f, 4.0f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(0.0, 1.0f, 0.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(3.0f, -2.5f, 1.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //
    //Create Pyramid 1
    // 
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPyramid4Mesh.vao);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(1.0f, 1.75f, 1.0f));
    // 2. Rotate the object
    rotation = glm::rotate(glm::radians(90.0f), glm::vec3(1.0, 0.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(30.0f), glm::vec3(0.0, 0.0f, 1.0f));
    rotation = glm::rotate(rotation, glm::radians(-5.0f), glm::vec3(1.0, 0.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(22.5f), glm::vec3(0.0, 1.0f, 0.0f));

    // 3. Position the object
    translation = glm::translate(glm::vec3(0.0f, 0.125f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    worldView = model;

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.0f, 0.5f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);
   
    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //
    //Create Pyramid 2
    // 
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gPyramid4Mesh.vao);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(1.0f, 1.75f, 1.0f));
    // 2. Rotate the object
    rotation = glm::rotate(glm::radians(90.0f), glm::vec3(1.0, 0.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(30.0f), glm::vec3(0.0, 0.0f, 1.0f));
    rotation = glm::rotate(rotation, glm::radians(-5.0f), glm::vec3(1.0, 0.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(67.5f), glm::vec3(0.0, 1.0f, 0.0f));

    // 3. Position the object
    translation = glm::translate(glm::vec3(0.0f, 0.125f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 0.0f, 0.5f, 0.15f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPyramid4Mesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //
    //Create the Taperd Cylinder
    //
    // Activate the VBOs contained within the mesh's VAO
    //glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

    //// 1. Scales the object
    //scale = glm::scale(glm::vec3(0.625f, 0.5f, 0.625f));
    //// 2. Rotate the object
    //rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    //// 3. Position the object
    //translation = glm::translate(glm::vec3(0.0f, -1.375f, 0.0f));
    //// Model matrix: transformations are applied right-to-left order
    //model = translation * rotation * scale;
    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //glProgramUniform4f(gProgramId, objectColorLoc, 0.15f, 0.14f, 0.32f, 1.0f);

    //// Draws the triangles
    //glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    //glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    //glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

    //// Deactivate the Vertex Array Object
    //glBindVertexArray(0);

    //
    //Create Tapered Cyinder 2
    //
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(0.4f, 0.5f, 0.4f));
    // 2. Rotate the object
    rotation = glm::rotate(glm::radians(180.0f), glm::vec3(1.0, 0.0f, 0.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(0.0f, -1.55f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    model = worldView * model;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 0.15f, 0.14f, 0.32f, 1.0f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //
    //CREATE THE OBLATE SPHERE
    //
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gSphereMesh.vao);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(0.48f, 1.0f, 0.48f));
    // 2. Rotate the object
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    model = translation * rotation * scale;
    model = worldView * model;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 0.15f, 0.14f, 0.32f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    //
    //CREATE THE BOX
    //
        // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(meshes.gBoxMesh.vao);

    // 1. Scales the object
    scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    // 2. Rotate the object
    rotation = glm::rotate(glm::radians(0.0f), glm::vec3(1.0, 1.0f, 1.0f));
    // 3. Position the object
    translation = glm::translate(glm::vec3(0.75f, -1.5f, -1.5f));
    // Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glProgramUniform4f(gProgramId, objectColorLoc, 0.97f, 0.48f, 0.04f, 1.0f);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
