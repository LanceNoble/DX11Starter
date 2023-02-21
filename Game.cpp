#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include <memory>
#include <iostream>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;
using namespace ImGui;
using namespace std;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}						 

// -----------------------Entity(triangle1);---------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've	
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	// Game class destructor needs to clean up the ImGui library and free its memory
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		//context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		//context->VSSetShader(vertexShader.Get(), 0, 0);
		//context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	mat0 = make_shared<Material>(XMFLOAT4(1.0f, 0.8f, 0.6f, 1.0f), vertexShader, pixelShader);
	mat1 = make_shared<Material>(XMFLOAT4(0.25f, 0.5f, 0.75f, 1.0f), vertexShader, pixelShader);
	mat2 = make_shared<Material>(XMFLOAT4(0.20f, 0.20f, 0.70f, 1.0f), vertexShader, pixelShader);

	cams.push_back(make_shared<Camera>((float)windowWidth / windowHeight, XMFLOAT3(0, 0, -5), 70.0f));
	cams.push_back(make_shared<Camera>((float)windowWidth / windowHeight, XMFLOAT3(0, 2, -1), 90.0f));
	
	activeCam = 0;

	entities[0] = Entity(mesh0, mat0);
	entities[1] = Entity(mesh1, mat1);
	entities[2] = Entity(mesh2, mat2);
	entities[3] = Entity(mesh0, mat0);
	entities[4] = Entity(mesh1, mat2);
	entityCount = sizeof(entities) / sizeof(Entity);

	timer = 1.0f;
	fps = 0.0f;
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	mesh0 = make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context);
	mesh1 = make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context);
	mesh2 = make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context);	
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	for (int i = 0; i < cams.size(); i++)
	{
		cams[i]->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	}
	DXCore::OnResize();
}

/// <summary>
/// Helper function for creating dropdown tree nodes
/// </summary>
/// <param name="label">tree node name</param>
/// <param name="object">object to showcase in tree node</param>
void Node(const char* label, Entity* object)
{
	if (TreeNode(label))
	{
		float position[3] = { object->GetTransform()->GetPosition().x, object->GetTransform()->GetPosition().y, object->GetTransform()->GetPosition().z };
		float orientation[4] = { object->GetTransform()->GetOrientation().x, object->GetTransform()->GetOrientation().y, object->GetTransform()->GetOrientation().z, object->GetTransform()->GetOrientation().w };
		float scale[3] = { object->GetTransform()->GetScale().x, object->GetTransform()->GetScale().y, object->GetTransform()->GetScale().z };
		DragFloat3("Position", position, 0.01f);
		DragFloat4("Rotation (Radians)", orientation, 0.01f, -1, 1);
		DragFloat3("Scale", scale, 0.01f);
		TreePop();
		object->GetTransform()->SetPosition(position[0], position[1], position[2]);
		object->GetTransform()->SetOrientation(XMFLOAT4(orientation[0], orientation[1], orientation[2], orientation[3]));
		object->GetTransform()->SetScale(scale[0], scale[1], scale[2]);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{ 
	entities[0].GetTransform()->MoveAbsolute(0.25f * deltaTime, 0, 0);
	entities[1].GetTransform()->MoveAbsolute(0.0f, -0.25f * deltaTime, 0.0f);
	entities[2].GetTransform()->Scale(0.0f, 0.25f * deltaTime, 0.0f);
	entities[3].GetTransform()->Rotate(0, 0, 0.25f * deltaTime);
	entities[4].GetTransform()->MoveAbsolute(0.25f * deltaTime, 0.0f, 0.0f);
	entities[4].GetTransform()->Scale(0, 0.25f * deltaTime, 0.0f);
	entities[4].GetTransform()->Rotate(0, 0, -0.25f * deltaTime);
	
	// The imgui stuff needs to be done first
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	ImGui::ShowDemoWindow();

	// imgui UI code happens after imgui frame setup

	// I had to add two new fields to the Game class for this to work: timer and fps
	// The timer and fps helps to control the ImGui::GetIO().Framerate
	// So the fps gets updated less frequently and we don't get a bunch of flashing new number updates every 0.00001 seconds
	ImGui::Begin("Basic Scene Info");
	timer -= ImGui::GetIO().DeltaTime;
	if (timer <= 0.0f) {
		fps = ImGui::GetIO().Framerate;
		timer = 1.0f;
	}
	ImGui::Text("Framerate: %f", fps);
	ImGui::Text("Window: %f x %f", ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
	ImGui::DragInt("Active Cam", &activeCam, 1.0f, 0, 1);
	ImGui::Text("Cam Pos: %f, %f, %f", cams[activeCam]->transform.GetPosition().x, cams[activeCam]->transform.GetPosition().y, cams[activeCam]->transform.GetPosition().z);
	ImGui::Text("Cam FOV: %f", cams[activeCam]->fov);
	ImGui::Text("Cam Aspect Ratio: %f", cams[activeCam]->aspRat);

	if (CollapsingHeader("Inspector"))
	{
		if (TreeNode("Scene Entities"))
		{
			Node("Object 0", &entities[0]);
			Node("Object 1", &entities[1]);
			Node("Object 2", &entities[2]);
			Node("Object 3", &entities[3]); 
			Node("Object 4", &entities[4]);
			TreePop();
		}


	}

	ImGui::End();
	
	for (int i = 0; i < 5; i++) {
		entities[i].GetTransform()->UpdateMatrices();
	}

	cams[activeCam]->Update(deltaTime);
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	{
		for (int i = 0; i < 5; i++) {
			// set shader before drawing entity since most likely each entity will want to be drawn via a different shader instead of the same global one
			entities[i].GetMaterial()->GetVertexShader()->SetShader();
			entities[i].GetMaterial()->GetPixelShader()->SetShader();
			entities[i].Draw(device, context, cams[activeCam]);
		}
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;

		// the imgui stuff should be drawn last here, right before the swap chain presents
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}