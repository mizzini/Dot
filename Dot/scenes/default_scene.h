#pragma once

#include "../dot.h"

class BlueCube : public SpatialNode
{
public:
    void Update(float dt) override
    {
        rotation.y += dt;

        SpatialNode::Update(dt);
    }

    void Draw() override
    {
        //Matrix transform = GetTransformMatrix();
        Matrix transform = GetWorldTransform();

        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(transform));

        DrawCube(Vector3Zero(), 2.0f, 2.0f, 2.0f, BLUE);
        DrawCubeWires(Vector3Zero(), 2.0f, 2.0f, 2.0f, RAYWHITE);

        rlPopMatrix();

        SpatialNode::Draw();
    }
};

class RedCube : public SpatialNode
{
public:
    void Update(float dt) override
    {
        SpatialNode::Update(dt);
    }

    void Draw() override
    {
        //Matrix transform = GetTransformMatrix();
        Matrix transform = GetWorldTransform();

        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(transform));

        DrawCube({ 2.0f, 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f, RED);
        DrawCubeWires({ 2.0f, 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f, RAYWHITE);

        rlPopMatrix();

        SpatialNode::Draw();
    }
};

class DefaultScene : public Scene
{
public:
    DefaultScene() : Scene("Default") {}

    void Start() override
    {
        // Input
        InputManager::Get().BindAction("Change Scene", KEY_SPACE);

        CameraNode* camera = new CameraNode();
        camera->position = { 0.0f, 4.0f, -10.0f };
        camera->rotation = { 0.0f, 0.0f, 0.0f }; // Looking straight forward...
        GetRoot()->AddChild(camera);
        
        SetCamera(camera);

        BlueCube* cube = new BlueCube();
        RedCube* redCube = new RedCube();
        cube->AddChild(redCube);
        cube->position = { 0.0f, 1.0f, 0.0f };
        camera->SetTarget(cube);
        GetRoot()->AddChild(cube);
    }

    void Update(float dt) override
    {
        if (InputManager::Get().IsActionPressed("Change Scene"))
        {
            std::cout << "Space pressed" << std::endl;
            SceneManager::Get().ChangeSceneByName("Next");
        }

        Scene::Update(dt);
    }
};

class NextScene : public Scene
{
public:
    NextScene() : Scene("Next") {}

    void Start() override
    {
        // Input
        InputManager::Get().BindAction("Change Scene", KEY_SPACE);
        
        CameraNode* camera = new CameraNode();
        camera->position = { 0.0f, 4.0f, -10.0f };
        camera->rotation = { 0.0f, 0.0f, 0.0f }; // Looking straight forward...
        GetRoot()->AddChild(camera);

        SetCamera(camera);

        BlueCube* cube = new BlueCube();
       
        cube->position = { 0.0f, 1.0f, 0.0f };
        camera->SetTarget(cube);
        GetRoot()->AddChild(cube);
    }

    void Update(float dt) override
    {
        if (InputManager::Get().IsActionPressed("Change Scene"))
        {
            std::cout << "Space pressed" << std::endl;
            SceneManager::Get().ChangeSceneByName("Default");
        }

        Scene::Update(dt);
    }
};
