#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <iostream>

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Character
bool isWalking = false;
float moveSpeed = 5.0f;
float characterRotation = 0.0f;
glm::vec3 characterPosition(0.0f, 0.4f, 0.0f);
float cameraDistance = 1.5f;
float cameraHeight = 0.6f;
bool isDying = false;
bool isDancing = false;


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Third-Person Character Demo", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    Shader ourShader("anim_model.vs", "anim_model.fs");
    Shader planeShader("anim_model.vs", "anim_model.fs");

    // Load animated model
    Model ourModel(FileSystem::getPath("resources/objects/gugu/Ty.dae"));
    Animation standAnimation(FileSystem::getPath("resources/objects/gugu/Standing.dae"), &ourModel);
    Animation walkAnimation(FileSystem::getPath("resources/objects/gugu/Walking.dae"), &ourModel);
    Animation dyingAnimation(FileSystem::getPath("resources/objects/gugu/Dying.dae"), &ourModel);
    Animation danceAnimation(FileSystem::getPath("resources/objects/gugu/Silly_Dancing.dae"), &ourModel);

    Animator animator(&standAnimation);
    Animation* currentAnimation = &standAnimation;
    
   

    // --- Create simple ground plane (large quad) ---
    float planeVertices[] = {
        // positions            // normals        // texcoords
        -50.0f, -0.4f, -50.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         50.0f, -0.4f, -50.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         50.0f, -0.4f,  50.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -50.0f, -0.4f,  50.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
    };
    unsigned int planeIndices[] = { 0, 1, 2, 2, 3, 0 };

    unsigned int planeVAO, planeVBO, planeEBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glGenBuffers(1, &planeEBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- Main render loop ---
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        static float danceTime = 0.0f;
        static float dieTime = 0.0f;

        if (isDying) {
            dieTime += deltaTime;
            if (dieTime >= dyingAnimation.GetDuration()) {
                isDying = false;
                dieTime = 0.0f;
            }
        }

        if (isDancing) {
            danceTime += deltaTime;
            if (danceTime >= danceAnimation.GetDuration()) {
                isDancing = false;
                danceTime = 0.0f;
            }
        }

        processInput(window);

        if (isDying) {
            if (currentAnimation != &dyingAnimation) {
                animator.PlayAnimation(&dyingAnimation);
                currentAnimation = &dyingAnimation;
            }
        }
        else if (isDancing) {
            if (currentAnimation != &danceAnimation) {
                animator.PlayAnimation(&danceAnimation);
                currentAnimation = &danceAnimation;
            }
        }
        else if (isWalking) {
            if (currentAnimation != &walkAnimation) {
                animator.PlayAnimation(&walkAnimation);
                currentAnimation = &walkAnimation;
            }
        }
        else {
            if (currentAnimation != &standAnimation) {
                animator.PlayAnimation(&standAnimation);
                currentAnimation = &standAnimation;
            }
        }


        animator.UpdateAnimation(deltaTime);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Set up camera ---
       
        glm::vec3 behind(
            sin(glm::radians(characterRotation)) * cameraDistance,
            cameraHeight,
            cos(glm::radians(characterRotation)) * cameraDistance
        );
        glm::vec3 cameraPos = characterPosition - behind;
        glm::vec3 cameraTarget = characterPosition + glm::vec3(0.0f, 0.3f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // --- Draw floor plane ---
        planeShader.use();
        planeShader.setMat4("projection", projection);
        planeShader.setMat4("view", view);
        glm::mat4 planeModel = glm::mat4(1.0f);
        planeShader.setMat4("model", planeModel);
        glBindVertexArray(planeVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // --- Draw animated model ---
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, characterPosition);
        model = glm::rotate(model, glm::radians(characterRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.15f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// --- Handle Input ---
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    isWalking = false;
    float velocity = moveSpeed * deltaTime;

    glm::vec3 forwardDir(
        sin(glm::radians(characterRotation)),
        0.0f,
        cos(glm::radians(characterRotation))
    );
    glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        characterPosition += forwardDir * velocity;
        isWalking = true;
        isDying = false;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        characterPosition -= forwardDir * velocity;
        isWalking = true;
        isDying = false;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        characterPosition -= rightDir * velocity;
        isWalking = true;
        isDying = false;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        characterPosition += rightDir * velocity;
        isWalking = true;
        isDying = false;
    }
 
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraHeight += 2.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraHeight -= 2.0f * deltaTime;

    // Trigger dying animation with L key
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !isDying) {
        isDying = true;
    }

    // Trigger dancing animation with K key
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !isDancing && !isDying) {
        isDancing = true;
    }

}

// --- Callbacks ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
