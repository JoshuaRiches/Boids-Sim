
#include "Scene.h"

// OpenGL includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// GLM includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// LearnOpenGl includes
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

// Project includes
#include "Entity.h"
#include "TransformComponent.h"
#include "ModelComponent.h"
#include "BrainComponent.h"
#include "Gizmos.h"

// IMGUI include
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

// Std includes
#include <iostream>


// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;
int NUM_OF_BOIDS = 100;
const unsigned int NUM_OF_BOID_GROUPS = 50;

glm::vec3 boxPos = glm::vec3(0);

// Statics
Scene* Scene::s_pSceneInstance = nullptr;

/// <summary>
/// Returns the instance of the scene
/// </summary>
Scene* Scene::GetInstance()
{
	if (s_pSceneInstance == nullptr)
	{
		s_pSceneInstance = new Scene();
	}

	return s_pSceneInstance;
}

// constructor
Scene::Scene() : m_window(nullptr), m_camera(nullptr), m_lastX(SCR_WIDTH / 2.0f), m_lastY(SCR_HEIGHT / 2.0f), m_firstMouse(true), m_deltaTime(0.0f), m_lastFrame(0.0f)
{
}

bool Scene::Initialise() 
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (m_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, mouse_callback);
    glfwSetScrollCallback(m_window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // Setup IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Set IMGUI style
    ImGui::StyleColorsDark();
    const char* glsl_version = "#version 150";
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // build and compile shaders
    // -------------------------
    m_shader = new Shader("shaders/model_loading.vs", "shaders/model_loading.fs");

    // load models
    // -----------
    m_model = new Model("models/fish/Guppy.obj");

    // Camera
    m_camera = new Camera(glm::vec3(0.0f, 1.0f, 10.0f));

    //---------- Creating Entity and adding components--------------\\

    // seed the random
    srand(time(nullptr));

    // set the value of num boids
    numBoids = NUM_OF_BOIDS;

    // Create entities
    for (int i = 0; i < NUM_OF_BOIDS; i++)
    {
        Entity* pEntity = new Entity();

        // Transform Component
        TransformComponent* pTransformComponent = new TransformComponent(pEntity);
        pTransformComponent->SetEntityMatrixRow(POSITION_VECTOR, glm::vec3( RandomFloatBetweenRange(-2, 2),
                                                                            RandomFloatBetweenRange(-2, 2),
                                                                            RandomFloatBetweenRange(-2, 2)));
        pEntity->AddComponent(pTransformComponent);

        // Model Component
        ModelComponent* pModelComponent = new ModelComponent(pEntity);
        pModelComponent->SetModel(m_model);
        pModelComponent->SetScale(0.01f);
        pEntity->AddComponent(pModelComponent);

        // Brain COmponent
        BrainComponent* pBrainComponent = new BrainComponent(pEntity);
        pEntity->AddComponent(pBrainComponent);

    }

    // do initial update of values for the boids
    UpdateBoidWeights();
    UpdateBoidNumber();

    m_boundingBoxSize = 4.0f;

    // create instance of gizmos
    Gizmos::create();

	return true;
}

bool Scene::Update()
{
    // start imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Imgui window
    showFrameData(true);
    changeBehaviourWeights(true);

    // per-frame time logic
    // --------------------
    float currentFrame = glfwGetTime();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    // input
    // -----
    m_camera->processInput(m_window, m_deltaTime);

    // update the values for the boids
    UpdateBoidWeights();
    UpdateBoidNumber();

    // Update Entities
    // Entities are processed in groups so that they are not all processed at once
    // this makes it less intensive on processing so performs faster
    m_iSizeOfGroup = NUM_OF_BOIDS / NUM_OF_BOID_GROUPS;

    if (m_iSizeOfGroup == 0) m_iSizeOfGroup++;

    m_iCurrentGroupNum++;
    if (m_iCurrentGroupNum > NUM_OF_BOID_GROUPS)
    {
        m_iCurrentGroupNum = 1;
    }

    std::map<const unsigned int, Entity*>::const_iterator xIter;
    for (xIter = Entity::GetEntityList().begin(); xIter != Entity::GetEntityList().end(); xIter++)
    {
        Entity* pEntity = xIter->second;
        if (pEntity)
        {
            if (pEntity->GetEntityID() + 1 > ((m_iCurrentGroupNum * m_iSizeOfGroup) - m_iSizeOfGroup) && 
                pEntity->GetEntityID() + 1 <= (m_iCurrentGroupNum * m_iSizeOfGroup))
            {
                BrainComponent* pBrainComp = static_cast<BrainComponent*>(pEntity->FindComponentOfType(BRAIN));
                // process the forces in groups for the boids
                pBrainComp->UpdateForces(m_deltaTime);
            }
            // other update functions are called for all of the boids each frame rather than just the groups
            pEntity->Update(m_deltaTime, m_boundingBoxSize);
        }
    }

    Gizmos::clear();
    // create the bounding box
    Gizmos::addBox(glm::vec3(0), glm::vec3(m_boundingBoxSize), false, glm::vec4(1, 0, 0, 1));
    // create the box that the boids avoid
    Gizmos::addBox(glm::vec3(boxPos), glm::vec3(0.25f), true, glm::vec4(1, 0, 0, 1));

    // return whether to close or not
	return glfwWindowShouldClose(m_window);
}

void Scene::Render()
{
    // render
    // ------
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_model == nullptr || m_shader == nullptr)
    {
        return; // early out
    }

    // don't forget to enable shader before setting uniforms
    m_shader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(m_camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = m_camera->GetViewMatrix();
    m_shader->setMat4("projection", projection);
    m_shader->setMat4("view", view);

    // Render Enities
    std::map<const unsigned int, Entity*>::const_iterator xIter;
    for (xIter = Entity::GetEntityList().begin(); xIter != Entity::GetEntityList().end(); xIter++)
    {
        Entity* pEntity = xIter->second;
        if (pEntity)
        {
            pEntity->Draw(m_shader);
        }
    }

    // render the gizmos items (bounding box and box to avoid)
    Gizmos::draw(view, projection);

    // renders the ImGui frames
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Scene::Deinitialise()
{
    // delete the values of items in the scene
    delete m_camera;
    delete m_shader;
    delete m_model;

    // remove all entities from memory
    std::map<const unsigned int, Entity*>::const_iterator xIter;
    for (xIter = Entity::GetEntityList().begin(); xIter != Entity::GetEntityList().end(); xIter++)
    {
        Entity* pEntity = xIter->second;
        delete pEntity;
    }

    // clear the gizmos
    Gizmos::destroy();

    // Clean up IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Scene::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void Scene::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Scene* pScene = Scene::GetInstance();
    if (!pScene)
    {
        return;
    }

    if (pScene->m_firstMouse)
    {
        pScene->m_lastX = xpos;
        pScene->m_lastY = ypos;
        pScene->m_firstMouse = false;
    }

    float xoffset = xpos - pScene->m_lastX;
    float yoffset = pScene->m_lastY - ypos; // reversed since y-coordinates go from bottom to top

    pScene->m_lastX = xpos;
    pScene->m_lastY = ypos;

    if (pScene->m_camera)
    {
        pScene->m_camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void Scene::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Scene* pScene = Scene::GetInstance();
    if (!pScene && !pScene->m_camera)
    {
        return;
    }
    pScene->m_camera->ProcessMouseScroll(yoffset);
}

// returns a random integer between two values
int Scene::RandomIntBewtweenRange(int a_iLowerRange, int a_iUpperRange)
{
    return rand() % ((a_iLowerRange - a_iUpperRange)) + (a_iLowerRange);
}

// returns a random float between two values
float Scene::RandomFloatBetweenRange(float a_fLowerRange, float a_fUpperRange)
{
    return a_fLowerRange + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (a_fUpperRange - a_fLowerRange)));
}

/// <summary>
/// loops through all the boids an updates the values on their brain component
/// </summary>
void Scene::UpdateBoidWeights()
{
    // create an iterator
    const std::map<const unsigned int, Entity*>& xEntityMap = Entity::GetEntityList();
    std::map<const unsigned int, Entity*>::const_iterator xConstIter;

    // loop through all the boids
    for (xConstIter = xEntityMap.begin(); xConstIter != xEntityMap.end(); xConstIter++)
    {
        Entity* pTarget = xConstIter->second;
        if (!pTarget)
        {
            return; // early out
        }
        BrainComponent* pTargetBrain = static_cast<BrainComponent*>(pTarget->FindComponentOfType(BRAIN));

        // update the values in the brain component
        pTargetBrain->allignmentWeight = allignmentWeight;
        pTargetBrain->cohesionWeight = cohesionWeight;
        pTargetBrain->separationWeight = separationWeight;
        pTargetBrain->wanderWeight = wanderWeight;
        pTargetBrain->boxPos = boxPos;
    }
}

void Scene::UpdateBoidNumber()
{
    // increase or decrease number of boids based on slider value
    int boidsToAdd = numBoids - NUM_OF_BOIDS;

    // if the boids to add is a positive value
    if (boidsToAdd > 0)
    {
        // Create entities
        while (numBoids > NUM_OF_BOIDS)
        {
            Entity* pEntity = new Entity();

            // Transform Component
            TransformComponent* pTransformComponent = new TransformComponent(pEntity);
            pTransformComponent->SetEntityMatrixRow(POSITION_VECTOR, glm::vec3(RandomFloatBetweenRange(-2, 2),
                                                                               RandomFloatBetweenRange(-2, 2),
                                                                               RandomFloatBetweenRange(-2, 2)));
            pEntity->AddComponent(pTransformComponent);

            // Model Component
            ModelComponent* pModelComponent = new ModelComponent(pEntity);
            pModelComponent->SetModel(m_model);
            pModelComponent->SetScale(0.01f);
            pEntity->AddComponent(pModelComponent);

            // Brain COmponent
            BrainComponent* pBrainComponent = new BrainComponent(pEntity);
            pEntity->AddComponent(pBrainComponent);
            NUM_OF_BOIDS++;
        }
    }
    // if the boids to add is negative
    else if (boidsToAdd < 0)
    {
        // for each boid to remove
        while (numBoids < NUM_OF_BOIDS)
        {
            // make the iterator
            std::map<const unsigned int, Entity*>::const_iterator xIter;
            xIter = Entity::GetEntityList().begin();
            // get the second value in the iterator (the entity value not key)
            Entity* pEntity = xIter->second;
            // remove the entity from the map
            Entity::RemoveEntity(xIter);
            // clear the entity from memory
            delete pEntity;
            // reduce number of boids
            NUM_OF_BOIDS--;
        }
    }
}

// function to show the frame data on the gui
void Scene::showFrameData(bool a_bShowFrameData)
{
    // setup the gui box in the top right corner
    const float DISTANCE = 10.0f;
    static int corner = 1;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.3f);

    // gui window content
    if (ImGui::Begin("Frame Data", &a_bShowFrameData, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Separator();
        // displays the average time per frame and the average frames per second so the user can see the performance of the program
        ImGui::Text("Scene Average: %.3f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);
    }
    ImGui::End();
}

// function to change the behaviour weights on the gui
void Scene::changeBehaviourWeights(bool a_bShowBehaviour)
{
    // setup the gui box in the bottom left corner
    const float  DISTANCE = 10.0f;
    static int corner = 2;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.3f);

    // grabs the position of the box and converts it to 3 floats so it can be used in the gui
    float pos[3] = { boxPos.x, boxPos.y, boxPos.z };

    if (ImGui::Begin("Behaviours", &a_bShowBehaviour, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Separator();
        // slider to change number of boids
        ImGui::SliderInt("Number Of Boids", &numBoids, minBoids, maxBoids);
        ImGui::Separator();
        // slider to change the weighting of the behaviour forces on the boids
        ImGui::SliderFloat("Wander Weight", &wanderWeight, minWeight, maxWeight);
        ImGui::SliderFloat("Cohesion Weight", &cohesionWeight, minWeight, maxWeight);
        ImGui::SliderFloat("Allignment Weight", &allignmentWeight, minWeight, maxWeight);
        ImGui::SliderFloat("Separation Weight", &separationWeight, minWeight, maxWeight);

        // float input to change the position of the box in the scene
        ImGui::InputFloat3("Box Position", pos, "%.3f");
    }
    ImGui::End();

    boxPos = glm::vec3(pos[0], pos[1], pos[2]);
}
