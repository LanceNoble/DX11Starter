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
	// Initialize Game members so VS stops yelling
	activeCam = 0;
	dir = {};
	pt = {};
	spot = {};
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
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	activeCam = 0;

	// Make sure to initialize the lights before loading the shaders
	dir = MakeDir(XMFLOAT3(0,0,-1), XMFLOAT3(.93f,.69f,.38f), 1);
	pt = MakePoint(3, XMFLOAT3(0,-1,0), 1, XMFLOAT3(1,1,1));
	spot = MakeSpot(XMFLOAT3(0,-1,0), 10, XMFLOAT3(0,5,0), 1, XMFLOAT3(.46f,.36f,1), XMConvertToRadians(30));

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	

	// Load textures and create sampler state before creating materials
	// Saul Goodman surface texture from https://knowyourmeme.com/memes/3d-saul-goodman
	// Saul Goodman normal map from https://steamcommunity.com/sharedfiles/filedetails/?id=2818203002
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/Surface/rock.png").c_str(), nullptr, srvSurf0.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/Surface/saulSurf.png").c_str(), nullptr, srvSurf1.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/Normal/rock_normals.png").c_str(), nullptr, srvNorm0.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Textures/Normal/saulNorm.png").c_str(), nullptr, srvNorm1.GetAddressOf());
	
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT; // Draw inside instead of outside
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rastDesc, skyRS.GetAddressOf());

	// Depth state so we accept pixels with a depth <= 1
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	device->CreateDepthStencilState(&depthDesc, skyDSS.GetAddressOf());

	D3D11_SAMPLER_DESC skySampDesc = {};
	skySampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	skySampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	skySampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	for (int i = 0; i < 4; i++) {
		skySampDesc.BorderColor[i] = 0.0f;
	}
	skySampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	skySampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	skySampDesc.MaxAnisotropy = 8;
	skySampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	skySampDesc.MinLOD = 0.0f;
	skySampDesc.MipLODBias = 0.0f;

	device->CreateSamplerState(&skySampDesc, skySS.GetAddressOf());
	
	sky = Sky(skySS, skySRV, skyDSS, skyRS, skyMesh, skyVS, skyPS, device);
	sky.CreateCubemap(FixPath(L"../../Assets/Textures/Sky/CloudsPink/right.png").c_str(), 
		FixPath(L"../../Assets/Textures/Sky/CloudsPink/left.png").c_str(), 
		FixPath(L"../../Assets/Textures/Sky/CloudsPink/up.png").c_str(), 
		FixPath(L"../../Assets/Textures/Sky/CloudsPink/down.png").c_str(), 
		FixPath(L"../../Assets/Textures/Sky/CloudsPink/front.png").c_str(), 
		FixPath(L"../../Assets/Textures/Sky/CloudsPink/back.png").c_str(), 
		device,
		context);
	
	

	D3D11_SAMPLER_DESC sampDesc;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	for (int i = 0; i < 4; i++) {
		sampDesc.BorderColor[i] = 0.0f;
	}
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 8;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.MinLOD = 0.0f;
	sampDesc.MipLODBias = 0.0f;

	device->CreateSamplerState(&sampDesc, sampState.GetAddressOf());


	XMFLOAT3 ambientColor = XMFLOAT3(0.1f, 0.1f, 0.25f);

	mats.push_back(make_shared<Material>(XMFLOAT4(0.44f, 0.31f, 0.22f, 1.0f), vs, ps, 0.0f, ambientColor));
	mats.push_back(make_shared<Material>(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), vs, ps, 0.0f, ambientColor));
	mats.push_back(make_shared<Material>(XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f), vs, ps, 0.0f, ambientColor));
	
	// After creating materials, add the SRVs and samp states to them
	for (int i = 0; i < 3; i++) {
		mats[i]->AddSampler("BasicSampler", sampState);
	}
	
	// we only have one Texture2D and one SamplerState in the pixel shader,
	// so we can only add one Texture2D and one SamplerState to a material
	mats[0]->AddTextureSRV("SurfaceTexture", srvSurf0);
	mats[0]->AddTextureSRV("NormalMap", srvNorm0);
	mats[1]->AddTextureSRV("SurfaceTexture", srvSurf1);
	mats[1]->AddTextureSRV("NormalMap", srvNorm1);
	mats[2]->AddTextureSRV("SurfaceTexture", srvSurf0);
	mats[2]->AddTextureSRV("NormalMap", srvNorm0);
	
	ents.push_back(Ent(meshes[0], mats[0]));
	ents.push_back(Ent(meshes[0], mats[1]));
	ents.push_back(Ent(meshes[0], mats[2]));
	ents.push_back(Ent(meshes[1], mats[0]));
	ents.push_back(Ent(meshes[1], mats[1]));
	ents.push_back(Ent(meshes[1], mats[2]));
	ents.push_back(Ent(meshes[2], mats[0]));
	ents.push_back(Ent(meshes[2], mats[1]));
	ents.push_back(Ent(meshes[2], mats[2]));

	for (int i = 0; i < ents.size(); i++) {
		ents[i].GetTf()->SetScale(0.25, 0.25, 0.25);
	}

	ents[0].GetTf()->SetPosition(-4, 0, 0);
	ents[1].GetTf()->SetPosition(-3, 0, 0);
	ents[2].GetTf()->SetPosition(-2, 0, 0);
	ents[3].GetTf()->SetPosition(-1, 0, 0);
	ents[4].GetTf()->SetPosition(0, 0, 0);
	ents[5].GetTf()->SetPosition(1, 0, 0);
	ents[6].GetTf()->SetPosition(2, 0, 0);
	ents[7].GetTf()->SetPosition(3, 0, 0);
	ents[8].GetTf()->SetPosition(4, 0, 0);

	cams.push_back(make_shared<Cam>((float)windowWidth / windowHeight, XMFLOAT3(0, 1, -6), XMFLOAT3(), 70.0f));
	cams.push_back(make_shared<Cam>((float)windowWidth / windowHeight, XMFLOAT3(0, 0, -4), XMFLOAT3()));

	
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
	vs = make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	ps = make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());

	unsigned int lightSize = sizeof(Light);
	ps->SetData("dir", &dir, lightSize);
	ps->SetData("pt", &pt, lightSize);
	ps->SetData("spot", &spot, lightSize);

	// Load sky shaders
	skyVS = make_shared<SimpleVertexShader>(device, context, FixPath(L"SkyVS.cso").c_str());
	skyPS = make_shared<SimplePixelShader>(device, context, FixPath(L"SkyPS.cso").c_str());

}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	skyMesh = make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context);

	meshes.push_back(make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context));
	meshes.push_back(make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context));
	meshes.push_back(make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));
}

Light Game::MakeDir(XMFLOAT3 dir, XMFLOAT3 color, float intensity)
{
	Light light = {};
	light.Type = LIGHT_TYPE_DIRECTIONAL;
	light.Direction = dir;
	light.Color = color;
	light.Intensity = intensity;
	return light;
}

Light Game::MakePoint(float range, DirectX::XMFLOAT3 pos, float intensity, DirectX::XMFLOAT3 color)
{
	Light light = {};
	light.Type = LIGHT_TYPE_POINT;
	light.Range = range;
	light.Position = pos;
	light.Intensity = intensity;
	light.Color = color;
	return light;
}

Light Game::MakeSpot(XMFLOAT3 dir, float range, XMFLOAT3 pos, float intensity, XMFLOAT3 color, float spotFalloff)
{
	Light light = {};
	light.Type = LIGHT_TYPE_SPOT;
	light.Direction = dir;
	light.Range = range;
	light.Position = pos;
	light.Intensity = intensity;
	light.Color = color;
	light.SpotFalloff = spotFalloff;
	return light;
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
		cams[i]->UpdateProj((float)this->windowWidth / this->windowHeight);
	}
	DXCore::OnResize();
}

/// <summary>
/// Helper function for creating dropdown tree nodes
/// </summary>
/// <param name="label">tree node name</param>
/// <param name="object">object to showcase in tree node</param>
void Game::Node(const char* label, Ent* object)
{
	if (TreeNode(label))
	{
		float position[3] = { object->GetTf()->GetPosition().x, object->GetTf()->GetPosition().y, object->GetTf()->GetPosition().z };
		float scale[3] = { object->GetTf()->GetScale().x, object->GetTf()->GetScale().y, object->GetTf()->GetScale().z };
		if (DragFloat3("Position", position, 0.01f)) {
			object->GetTf()->SetPosition(position[0], position[1], position[2]);
		}
		if (DragFloat3("Scale", scale, 0.01f)) {
			object->GetTf()->SetScale(scale[0], scale[1], scale[2]);
		}
		TreePop();
	}
}

// Helper function for creating imgui controls for lights
void Game::LightNode(const char* label, Light* light) {
	if (TreeNode(label)) {
		float direction[3] = { light->Direction.x, light->Direction.y, light->Direction.z };
		float range = light->Range;
		float position[3] = { light->Position.x, light->Position.y, light->Position.z };
		float intensity = light->Intensity;
		float color[3] = { light->Color.x, light->Color.y, light->Color.z };
		float falloff = light->SpotFalloff;
		Text("Type: %f", light->Type);
		DragFloat3("Direction", direction, 0.01f);
		DragFloat("Range", &range, 0.01f, 0.0f);
		DragFloat3("Position", position, 0.01f);
		DragFloat("Intensity", &intensity, 0.01f, 0.0f, 1.0f);
		DragFloat3("Color", color, 0.01f, 0.0f, 1.0f);
		DragFloat("Falloff", &falloff, 0.01f, 0.0f);
		light->Direction = XMFLOAT3(direction[0], direction[1], direction[2]);
		light->Range = range;
		light->Position = XMFLOAT3(position[0], position[1], position[2]);
		light->Intensity = intensity;
		light->Color = XMFLOAT3(color[0], color[1], color[2]);
		light->SpotFalloff = falloff;
		TreePop();
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{ 
	for (int i = 0; i < ents.size(); i++)
	{
		//ents[i].GetTransform()->Rotate(.5 * deltaTime,.5 * deltaTime, .5 * deltaTime);
		ents[i].GetTf()->Rotate(0, .25f * deltaTime, 0);
	}
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
	ImGui::Text("Window: %f x %f", ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
	ImGui::DragInt("Active Cam", &activeCam, 1.0f, 0, 1);
	ImGui::Text("Cam Pos: %f, %f, %f", cams[activeCam]->GetPos().x, cams[activeCam]->GetPos().y, cams[activeCam]->GetPos().z);
	ImGui::Text("Cam FOV: %f", cams[activeCam]->GetFOV());
	ImGui::Text("Cam Aspect Ratio: %f", cams[activeCam]->GetAspRat());

	if (CollapsingHeader("Inspector"))
	{
		if (TreeNode("Scene Entities"))
		{
			Node("Entity 0", &ents[0]);
			Node("Entity 1", &ents[1]);
			Node("Entity 2", &ents[2]);
			Node("Entity 3", &ents[3]);
			Node("Entity 4", &ents[4]);
			Node("Entity 5", &ents[5]);
			Node("Entity 6", &ents[6]);
			Node("Entity 7", &ents[7]);
			Node("Entity 8", &ents[8]);
			TreePop();
		}
		if (TreeNode("Lights")) {
			LightNode("Light 0", &dir);
			LightNode("Light 1", &pt);
			LightNode("Light 2", &spot);
			TreePop();
		}

	}

	ImGui::End();


	unsigned int lightSize = sizeof(Light);
	ps->SetData("dir", &dir, lightSize);
	ps->SetData("pt", &pt, lightSize);
	ps->SetData("spot", &spot, lightSize);

	for (int i = 0; i < ents.size(); i++) {
		ents[i].GetTf()->UpdateMatrices();
	}

	cams[activeCam]->Move(deltaTime);
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
	{
		for (int i = 0; i < ents.size(); i++) {
			// set shader before drawing entity since most likely each entity will want to be drawn via a different shader instead of the same global one
			ents[i].GetMat()->GetVertexShader()->SetShader();
			ents[i].GetMat()->GetPixelShader()->SetShader();
			ents[i].Draw(cams[activeCam]);
		}
	}

	sky.Draw(context, cams, activeCam);

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