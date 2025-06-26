#include "dot.h"
#include <sstream>

// Node
Node::Node()
{
    parent = nullptr;
}

Node::~Node()
{
    for (Node* child : children)
    {
        delete child;
    }
    children.clear();
}

void Node::AddChild(Node* child)
{
    child->parent = this;
    children.push_back(child);
    child->Start();
}

void Node::RemoveChild(Node* child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end())
    {
        (*it)->OnDestroy();
        delete* it;
        children.erase(it);
    }
}

Node* Node::GetParent()
{
    return parent;
}

const std::vector<Node*>& Node::GetChildren() const
{
    return children;
}

void Node::Update(float dt)
{
    ProcessInput();
    for (Node* child : children)
    {
        child->Update(dt);
    }
}

void Node::Draw()
{
    for (Node* child : children)
    {
        child->Draw();
    }
}

void Node::DebugPrint(int depth) const
{
    // Visual indentation based on depth of node hierarchy
    for (int i = 0; i < depth; ++i)
    {
        std::cout << (i == depth - 1 ? " +-- " : " |   ");

    }

    // Print class type (RTTI)
    std::cout << "[" << typeid(*this).name() << "]\n";

    // Build visual recursively.
    for (const Node* child : children)
    {
        child->DebugPrint(depth + 1);
    }
}

// SpatialNode
Matrix SpatialNode::GetTransformMatrix() const 
{
    Matrix t = MatrixTranslate(position.x, position.y, position.z);
    Matrix r = MatrixRotateXYZ(rotation);
    Matrix s = MatrixScale(scale.x, scale.y, scale.z);
    return MatrixMultiply(MatrixMultiply(s, r), t);
}

Matrix SpatialNode::GetWorldTransform() const
{
    if (parent && dynamic_cast<SpatialNode*>(parent))
    {
        auto* p = static_cast<SpatialNode*>(parent);
        return MatrixMultiply(p->GetWorldTransform(), GetTransformMatrix());
    }
    return GetTransformMatrix();
}

// CameraNode
void CameraNode::Start()
{
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.up = { 0.0f, 1.0f, 0.0f };
}

void CameraNode::Update(float dt)
{
    camera.position = position;

    if (targetNode)
    {
        // Look at the target node's position
        SpatialNode* targetSpatial = dynamic_cast<SpatialNode*>(targetNode);
        if (targetSpatial)
        {
            camera.target = targetSpatial->position;
        }
    }
    else
    {
        // Default look forward
        Matrix rotationMatrix = MatrixRotateXYZ(rotation);
        Vector3 forward = Vector3Transform({ 0.0f, 0.0f, 1.0f }, rotationMatrix);
        camera.target = Vector3Add(position, forward);
    }

    SpatialNode::Update(dt);
}

void CameraNode::SetTarget(Node* node)
{
    targetNode = node;
}

Camera3D& CameraNode::GetCamera()
{
    return camera;
}

// InputManager
void InputManager::Update(float deltaTime) 
{
    // Track previous frame input?
}

bool InputManager::IsActionPressed(const std::string& action) 
{
    auto key = keyBindings[action];
    return IsKeyPressed(key);
}

bool InputManager::IsActionDown(const std::string& action) 
{
    auto key = keyBindings[action];
    return IsKeyDown(key);
}

bool InputManager::IsActionReleased(const std::string& action) 
{
    auto key = keyBindings[action];
    return IsKeyReleased(key);
}

void InputManager::BindAction(const std::string& action, KeyboardKey key) 
{
    keyBindings[action] = key;
}

// Coroutine
WaitForSeconds::WaitForSeconds(float seconds)
    : timeRemaining(seconds) {
}

bool WaitForSeconds::IsComplete(float dt)
{
    timeRemaining -= dt;
    return timeRemaining <= 0;
}

// Coroutine Constructor
Coroutine::Coroutine(std::function<CoroutineYield* ()> f)
    : func(f), currentYield(nullptr) {
}

// CoroutineManager Singleton Getter
CoroutineManager& CoroutineManager::GetInstance()
{
    static CoroutineManager instance;
    return instance;
}

// Start a coroutine
void CoroutineManager::StartCoroutine(std::function<CoroutineYield* ()> coroutine)
{
    coroutines.emplace_back(coroutine);
}

// Update all coroutines
void CoroutineManager::Update(float dt)
{
    for (size_t i = 0; i < coroutines.size(); )
    {
        Coroutine& coroutine = coroutines[i];

        if (coroutine.currentYield)
        {
            if (!coroutine.currentYield->IsComplete(dt))
            {
                ++i;
                continue;
            }

            delete coroutine.currentYield;
            coroutine.currentYield = nullptr;
        }

        coroutine.currentYield = coroutine.func();

        if (!coroutine.currentYield)
        {
            coroutines.erase(coroutines.begin() + i);
            continue;
        }

        ++i;
    }
}

// Scene
Scene::Scene(const std::string& name)
    : name(name)
{
    root = new Node(); // Root node is always present.
    SceneManager::Get().RegisterScene(name, this);
}

const std::string& Scene::GetName() const
{
    return name;
}

Scene::~Scene()
{
    Unload();
}

void Scene::Update(float dt)
{
    if (root) root->Update(dt);
}

void Scene::Draw()
{
    if (!cameraNode)
    {
        cameraNode = FindFirstCameraNode(root);
    }

    if (cameraNode)
    {
        BeginMode3D(cameraNode->GetCamera());
        {
            if (root) root->Draw();
        }
        EndMode3D();
    }
    else
    {
        if (root) root->Draw();
    }
}

void Scene::Unload()
{
    delete root;
    root = nullptr;
    cameraNode = nullptr;
}

void Scene::Reload()
{
    std::cout << "[Scene] Resetting scene: " << name << "\n";
    Unload();              // delete the root node and children
    root = new Node();     // create a fresh root node
    cameraNode = nullptr;  // reset the camera reference
    Start();               // call the scene's setup logic again
}


void Scene::SetCamera(CameraNode* cam)
{
    cameraNode = cam;
}

CameraNode* Scene::GetCamera() const
{
    return cameraNode;
}

Node* Scene::GetRoot() const
{
    return root;
}

CameraNode* Scene::FindFirstCameraNode(Node* node)
{
    if (auto* cam = dynamic_cast<CameraNode*>(node))
    {
        return cam;
    }

    for (Node* child : node->GetChildren())
    {
        if (CameraNode* found = FindFirstCameraNode(child))
        {
            return found;
        }
    }

    return nullptr;
}

// SceneManager
SceneManager& SceneManager::Get()
{
    static SceneManager instance;
    return instance;
}

void SceneManager::RegisterScene(const std::string& name, Scene* scene)
{
    sceneMap[name] = scene;
}

void SceneManager::ChangeSceneByName(const std::string& name)
{
    auto it = sceneMap.find(name);
    // If scene not found
    if (it == sceneMap.end())
    {
        std::cout << "[SceneManager] Scene not found: " << name << "\n";
        return;
    }

    Scene* newScene = it->second;
    if (currentScene == newScene)
    {
        newScene->Reload();
    }
    else
    {
        currentScene = newScene;
        currentScene->Reload();
    }
    currentSceneName = name;
}

void SceneManager::ChangeScene(Scene* newScene)
{
    if (currentScene)
    {
        currentScene->Unload();
        delete currentScene;
    }
    currentScene = newScene;
    currentScene->Start();
}

void SceneManager::DebugPrintAvailableScenes() const
{
    std::cout << "\033[1;36mAvailable Scenes:\033[0m\n";
    for (const auto& pair : sceneMap)
    {
        std::cout << "- " << pair.first;
        if (pair.first == currentSceneName)
            std::cout << " (active)";
        std::cout << "\n";
    }
    std::cout << "========================\n";
}

void SceneManager::ProcessInput()
{
    if (currentScene) currentScene->ProcessInput();
}

void SceneManager::Update(float dt)
{
    InputManager::Get().Update(dt); // Handle all user inputs.
    if (currentScene) currentScene->Update(dt);
}

void SceneManager::Draw()
{
    if (currentScene) currentScene->Draw();
}

void SceneManager::UnloadScene()
{
    if (currentScene)
    {
        currentScene->Unload();
        delete currentScene;
        currentScene = nullptr;
    }
}

void SceneManager::UnloadAllScenes()
{
    if (currentScene)
    {
        currentScene->Unload(); // Unload scene data.
        currentScene = nullptr;
        currentSceneName = "";
    }

    for (auto& pair : sceneMap)
    {
        if (pair.second)
        {
            delete pair.second;
        }
    }

    sceneMap.clear(); // Clear the map of scene names.
}


Scene* SceneManager::GetCurrentScene() const
{
    return currentScene;
}

void ClearConsole()
{
#if defined(_WIN32)
    system("cls");
#else
    std::cout << "\033[2J\033[1;1H"; // clear screen
#endif
}

// DebugOverlay
void PrintDotHeader()
{
    const char* PURPLE_ = "\033[1;35m";
    const char* RESET = "\033[0m";

    std::cout << PURPLE_;
    std::cout << "========================================\n";
    std::cout << "  ____        _   \n";
    std::cout << " |  _ \\  ___ | |_ \n";
    std::cout << " | | | |/ _ \\| __|\n";
    std::cout << " | |_| | (_) | |_ \n";
    std::cout << " |____/ \\___/ \\__|\n";
    std::cout << "        DOT FRAMEWORK\n";
    std::cout << "========================================\n";
    std::cout << RESET;
}


void DebugOverlay::Draw()
{
    static std::string lastPrintedSceneName = "";

    Scene* scene = SceneManager::Get().GetCurrentScene();
    std::string currentSceneName = scene->GetName();

    if (currentSceneName != lastPrintedSceneName)
    {
        PrintDotHeader();

        if (scene && scene->GetRoot())
        {
            std::cout << "\033[1;36mScene Hierarchy:\033[0m\n"; // Cyan bold text
            scene->GetRoot()->DebugPrint();
        }

        std::cout << "\n";
        SceneManager::Get().DebugPrintAvailableScenes();

        lastPrintedSceneName = currentSceneName;
    }
}

// DotApp
void DotApp::Run(int width, int height, const std::string& title)
{
    InitWindow(width, height, title.c_str());
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Output scene hierarchy to console.
        DebugOverlay::Draw();

        SceneManager::Get().Update(dt);

        BeginDrawing();
        ClearBackground(LIGHTGRAY);
        SceneManager::Get().Draw();
        EndDrawing();
    }

    SceneManager::Get().UnloadAllScenes();
    CloseWindow();
}

