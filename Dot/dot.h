#pragma once

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <string>

// Node
class Node 
{
public:
    Node();
    virtual ~Node();

    void AddChild(Node* child);
    void RemoveChild(Node* child);
    Node* GetParent();
    const std::vector<Node*>& GetChildren() const;

    virtual void Start() {}
    virtual void ProcessInput() {}
    virtual void Update(float dt);
    virtual void Draw();
    virtual void OnDestroy() {}

    virtual void DebugPrint(int depth = 0) const;

protected:
    Node* parent;
    std::vector<Node*> children;
};

class SpatialNode : public Node 
{
public:
    Vector3 position = { 0, 0, 0 };
    Vector3 scale = { 1, 1, 1 };
    Vector3 rotation = { 0, 0, 0 }; // radians

    Matrix GetTransformMatrix() const;
    Matrix GetWorldTransform() const;
};

class CameraNode : public SpatialNode
{
private:
    Node* targetNode = nullptr;

public:
    Camera3D camera{};

    void Start() override;
    void Update(float dt) override;
    void SetTarget(Node* node);
    Camera3D& GetCamera();
};

class CoroutineYield
{
public:
    virtual bool IsComplete(float dt) = 0;
    virtual ~CoroutineYield() {}
};

// WaitForSeconds: derived class that waits for a given time
class WaitForSeconds : public CoroutineYield
{
private:
    float timeRemaining;

public:
    explicit WaitForSeconds(float seconds);
    bool IsComplete(float dt) override;
};

// Struct to hold coroutine state
struct Coroutine
{
    std::function<CoroutineYield* ()> func;
    CoroutineYield* currentYield = nullptr;

    explicit Coroutine(std::function<CoroutineYield* ()> f);
};

// Singleton manager to handle coroutine updates
class CoroutineManager
{
private:
    std::vector<Coroutine> coroutines;
    CoroutineManager() = default;

public:
    static CoroutineManager& GetInstance();

    void StartCoroutine(std::function<CoroutineYield* ()> coroutine);
    void Update(float dt);
};

// Global helper function to start a coroutine
inline void StartCoroutine(std::function<CoroutineYield* ()> coroutine)
{
    CoroutineManager::GetInstance().StartCoroutine(coroutine);
}

// Scene
class Scene 
{
public:
    Scene(const std::string& name);
    virtual ~Scene();

    const std::string& GetName() const;

    virtual void Start() {}
    virtual void ProcessInput() {}
    virtual void Update(float dt);
    virtual void Draw();
    virtual void Unload();
    virtual void Reload();

    void SetCamera(CameraNode* cam);
    CameraNode* GetCamera() const;

    Node* GetRoot() const;

protected:
    std::string name;
    Node* root;
    CameraNode* cameraNode = nullptr;

private:
    CameraNode* FindFirstCameraNode(Node* node);
};

class SceneManager
{
public:
    static SceneManager& Get();

    void RegisterScene(const std::string& name, Scene* scene);
    void ChangeSceneByName(const std::string& name);
    void ChangeScene(Scene* newScene);

    void ProcessInput();
    void Update(float dt);
    void Draw();
    void UnloadScene();
    void UnloadAllScenes();

    Scene* GetCurrentScene() const;

    void DebugPrintAvailableScenes() const;

private:
    Scene* currentScene = nullptr;
    std::string currentSceneName = "None";

    std::unordered_map<std::string, Scene*> sceneMap;
};


// Input
class InputManager 
{
public:
    static InputManager& Get() 
    {
        static InputManager instance;
        return instance;
    }

    void Update(float deltaTime); // call this once per frame

    bool IsActionPressed(const std::string& action);
    bool IsActionDown(const std::string& action);
    bool IsActionReleased(const std::string& action);

    void BindAction(const std::string& action, KeyboardKey key);

private:
    std::unordered_map<std::string, KeyboardKey> keyBindings;
};

// DebugOverlay
class DebugOverlay
{
public:
    static void Draw();
};

// DotApp
class DotApp
{
public:
    void Run(int width, int height, const std::string& title);
};
