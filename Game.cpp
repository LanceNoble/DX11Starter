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
using namespace Microsoft::WRL;

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
	ent6Dir = 1;
	ent4Dir = 1;
	shadowMapResolution = 2048;
	blurRadius = 5;
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

/// <summary>
/// Destroys and re-creates the post process render targets whenever the window size changes
/// </summary>
/// <param name="rtv">the render target to reset</param>
/// <param name="srv">the render target's associated shader resource view to reset</param>
void Game::ResetRenderTarget()
{
	ppRTV.Reset();
	ppSRV.Reset();

	// Describe the texture we're creating for post process
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = windowWidth;
	textureDesc.Height = windowHeight;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View for post process
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());
	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());


}

void Game::AddTextures(std::shared_ptr<Material> mat, const wchar_t* albedo, const wchar_t* normal, const wchar_t* roughness, const wchar_t* metalness)
{
	ComPtr<ID3D11ShaderResourceView> SRVs[4];
	CreateWICTextureFromFile(device.Get(), context.Get(), albedo, nullptr, SRVs[0].GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), normal, nullptr, SRVs[1].GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), roughness, nullptr, SRVs[2].GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), metalness, nullptr, SRVs[3].GetAddressOf());
	mat->AddTextureSRV("Albedo", SRVs[0]);
	mat->AddTextureSRV("NormalMap", SRVs[1]);
	mat->AddTextureSRV("RoughnessMap", SRVs[2]);
	mat->AddTextureSRV("MetalnessMap", SRVs[3]);
	mat->AddTextureSRV("ShadowMap", shadowSRV);
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


	// Initialize lights before loading shaders
	XMFLOAT3 dirOri = XMFLOAT3(0, -1, -0.5f);
	XMVECTOR dirOriMath = XMLoadFloat3(&dirOri);
	XMVector3Normalize(dirOriMath);
	XMStoreFloat3(&dirOri, dirOriMath);
	dir = MakeDir(dirOri, XMFLOAT3(1,1,1), 1);
	
	vs = make_shared<SimpleVertexShader>(device, context, FixPath(L"VertexShader.cso").c_str());
	ps = make_shared<SimplePixelShader>(device, context, FixPath(L"PixelShader.cso").c_str());
	unsigned int lightSize = sizeof(Light);
	ps->SetData("dir", &dir, lightSize);
	ps->SetData("pt", &pt, lightSize);
	ps->SetData("spot", &spot, lightSize);
	skyVS = make_shared<SimpleVertexShader>(device, context, FixPath(L"SkyVS.cso").c_str());
	skyPS = make_shared<SimplePixelShader>(device, context, FixPath(L"SkyPS.cso").c_str());
	shadowVS = make_shared<SimpleVertexShader>(device, context, FixPath(L"Shadow.cso").c_str());

	ppVS = make_shared<SimpleVertexShader>(device, context, FixPath(L"ppVS.cso").c_str());
	ppPS = make_shared<SimplePixelShader>(device, context, FixPath(L"ppPS.cso").c_str());

	meshes.insert({ "sphere", make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context) });
	meshes.insert({ "cube", make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context) });
	meshes.insert({ "helix", make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context) });

	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	ResetRenderTarget();

	// CODE: Shadow stuff
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; 
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; 
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution; 
	shadowDesc.Height = shadowMapResolution;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, shadowDSV.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowSRV.GetAddressOf());

	XMVECTOR dirLightDirMath = XMLoadFloat3(&dir.Direction);
	XMMATRIX lightView = XMMatrixLookToLH(-dirLightDirMath * 20, dirLightDirMath, XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowViewMatrix, lightView);

	float lightProjectionSize = 15.0f; // If shadows get cut off, increase this variable
	XMMATRIX lightProjection = XMMatrixOrthographicLH(lightProjectionSize, lightProjectionSize, 1.0f, 100.0f);
	XMStoreFloat4x4(&shadowProjectionMatrix, lightProjection);

	// CODE END


	D3D11_SAMPLER_DESC ssDesc;
	ssDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	ssDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	ssDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	for (int i = 0; i < 4; i++) ssDesc.BorderColor[i] = 0.0f;
	ssDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	ssDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	ssDesc.MaxAnisotropy = 8;
	ssDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ssDesc.MinLOD = 0.0f;
	ssDesc.MipLODBias = 0.0f;
	device->CreateSamplerState(&ssDesc, ss.GetAddressOf());

	for (int i = 0; i < 7; i++)
	{
		mats.push_back(make_shared<Material>(XMFLOAT4(1, 1, 1, 1), vs, ps));
		mats[i]->AddSampler("Sampler", ss);
		mats[i]->AddSampler("ShadowSampler", shadowSampler);
		if (i % 3 == 0) ents.push_back(Ent(meshes["sphere"], mats[i]));
		else if (i % 2 == 0) ents.push_back(Ent(meshes["cube"], mats[i]));
		else ents.push_back(Ent(meshes["helix"], mats[i]));
		ents[i].GetTf()->SetPosition((float)i, 1, 0);
		ents[i].GetTf()->SetScale(0.25f,0.25f,0.25f);
	}

	int rows = sizeof(floor) / sizeof(floor[0]);
	int cols = sizeof(floor[0]) / sizeof(Ent);
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			floor[i][j] = Ent(meshes["cube"], mats[6]);
			floor[i][j].GetTf()->SetPosition((i * 2.0f) - 10, -2.0f, (j * 2.0f) - 10);
			floor[i][j].GetTf()->UpdateMatrices();
		}
	}

	AddTextures(mats[0], FixPath(L"../../Assets/Textures/AlbedoMaps/bronze_albedo.png").c_str(), 
		FixPath(L"../../Assets/Textures/NormalMaps/bronze_normals.png").c_str(), 
		FixPath(L"../../Assets/Textures/RoughnessMaps/bronze_roughness.png").c_str(), 
		FixPath(L"../../Assets/Textures/MetalMaps/bronze_metal.png").c_str());

	AddTextures(mats[1], FixPath(L"../../Assets/Textures/AlbedoMaps/cobblestone_albedo.png").c_str(), 
		FixPath(L"../../Assets/Textures/NormalMaps/cobblestone_normals.png").c_str(), 
		FixPath(L"../../Assets/Textures/RoughnessMaps/cobblestone_roughness.png").c_str(), 
		FixPath(L"../../Assets/Textures/MetalMaps/cobblestone_metal.png").c_str());

	AddTextures(mats[2], FixPath(L"../../Assets/Textures/AlbedoMaps/floor_albedo.png").c_str(), 
		FixPath(L"../../Assets/Textures/NormalMaps/floor_normals.png").c_str(), 
		FixPath(L"../../Assets/Textures/RoughnessMaps/floor_roughness.png").c_str(), 
		FixPath(L"../../Assets/Textures/MetalMaps/floor_metal.png").c_str());

	AddTextures(mats[3], FixPath(L"../../Assets/Textures/AlbedoMaps/paint_albedo.png").c_str(), 
		FixPath(L"../../Assets/Textures/NormalMaps/paint_normals.png").c_str(), 
		FixPath(L"../../Assets/Textures/RoughnessMaps/paint_roughness.png").c_str(), 
		FixPath(L"../../Assets/Textures/MetalMaps/paint_metals.png").c_str());

	AddTextures(mats[4], FixPath(L"../../Assets/Textures/AlbedoMaps/rough_albedo.png").c_str(), 
		FixPath(L"../../Assets/Textures/NormalMaps/rough_normals.png").c_str(), 
		FixPath(L"../../Assets/Textures/RoughnessMaps/rough_roughness.png").c_str(), 
		FixPath(L"../../Assets/Textures/MetalMaps/rough_metal.png").c_str());

	AddTextures(mats[5], FixPath(L"../../Assets/Textures/AlbedoMaps/scratched_albedo.png").c_str(), 
		FixPath(L"../../Assets/Textures/NormalMaps/scratched_normals.png").c_str(), 
		FixPath(L"../../Assets/Textures/RoughnessMaps/scratched_roughness.png").c_str(), 
		FixPath(L"../../Assets/Textures/MetalMaps/scratched_metal.png").c_str());

	AddTextures(mats[6], FixPath(L"../../Assets/Textures/AlbedoMaps/wood_albedo.png").c_str(), 
		FixPath(L"../../Assets/Textures/NormalMaps/wood_normals.png").c_str(), 
		FixPath(L"../../Assets/Textures/RoughnessMaps/wood_roughness.png").c_str(), 
		FixPath(L"../../Assets/Textures/MetalMaps/wood_metal.png").c_str());

	cams.push_back(make_shared<Cam>((float)windowWidth / windowHeight, XMFLOAT3(3, 0, 5), XMFLOAT3(0, XM_PI, 0), 70.0f));
	cams.push_back(make_shared<Cam>((float)windowWidth / windowHeight, XMFLOAT3(3, 0, 5), XMFLOAT3(0, XM_PI, 0)));

	// CODE: Initialize skybox
	D3D11_RASTERIZER_DESC skyRSDesc = {};
	skyRSDesc.CullMode = D3D11_CULL_FRONT; // Draw inside instead of outside
	skyRSDesc.FillMode = D3D11_FILL_SOLID;
	skyRSDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&skyRSDesc, skyRS.GetAddressOf());

	// Depth state so we accept pixels with a depth <= 1
	D3D11_DEPTH_STENCIL_DESC skyDSSDesc = {};
	skyDSSDesc.DepthEnable = true;
	skyDSSDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	skyDSSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	device->CreateDepthStencilState(&skyDSSDesc, skyDSS.GetAddressOf());

	// Can just reuse the sampler state for the textures (unless you want different settings)
	sky = Sky(ss, skySRV, skyDSS, skyRS, meshes["cube"], skyVS, skyPS, device);
	sky.CreateCubemap(FixPath(L"../../Assets/Textures/Sky/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Sky/back.png").c_str(),
		device,
		context);
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
	ResetRenderTarget();
	// Handle base-level DX resize stuff
	for (int i = 0; i < cams.size(); i++)
		cams[i]->UpdateProj((float)this->windowWidth / this->windowHeight);
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

	ImGui::Begin("Basic Scene Info");
	ImGui::Text("Window: %f x %f", ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
	ImGui::DragInt("Active Cam", &activeCam, 1.0f, 0, 1);
	ImGui::Text("Cam Pos: %f, %f, %f", cams[activeCam]->GetPos().x, cams[activeCam]->GetPos().y, cams[activeCam]->GetPos().z);
	ImGui::Text("Cam FOV: %f", cams[activeCam]->GetFOV());
	ImGui::Text("Cam Aspect Ratio: %f", cams[activeCam]->GetAspRat());
	ImGui::SliderInt("Blur Radius: %f", &blurRadius, 0, 10);
	ImGui::Image(shadowSRV.Get(), ImVec2(512, 512));

	if (CollapsingHeader("Inspector"))
	{
		if (TreeNode("Lights")) {
			LightNode("Directional Light", &dir);
			TreePop();
		}

	}

	ImGui::End();


	if (ents[6].GetTf()->GetPosition().z > 2) ent6Dir = -1;
	else if (ents[6].GetTf()->GetPosition().z < -2) ent6Dir = 1;
	ents[6].GetTf()->MoveAbsolute(0, 0, ent6Dir * deltaTime);

	ents[5].GetTf()->Rotate(0, deltaTime, 0);

	if (ents[4].GetTf()->GetPosition().y > 2) ent4Dir = -1;
	else if (ents[4].GetTf()->GetPosition().y < 0) ent4Dir = 1;
	ents[4].GetTf()->MoveAbsolute(0, ent4Dir * deltaTime, 0);
	


	unsigned int lightSize = sizeof(Light);
	ps->SetData("dir", &dir, lightSize);
	ps->SetData("pt", &pt, lightSize);
	ps->SetData("spot", &spot, lightSize);

	for (int i = 0; i < ents.size(); i++) ents[i].GetTf()->UpdateMatrices();

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

	// CODE: Render fresh info to the shadow map
	context->RSSetState(shadowRasterizer.Get());

	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());
	context->PSSetShader(0, 0, 0);
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjectionMatrix);
	

	for (auto& e : ents)
	{
		shadowVS->SetMatrix4x4("world", e.GetTf()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		e.GetMesh()->Draw();
	}

	for (int i = 0; i < sizeof(floor) / sizeof(floor[0]); i++)
	{
		for (int j = 0; j < sizeof(floor[0]) / sizeof(Ent); j++)
		{
			shadowVS->SetMatrix4x4("world", floor[i][j].GetTf()->GetWorldMatrix());
			shadowVS->CopyAllBufferData();
			floor[i][j].GetMesh()->Draw();
		}
	}

	// change rendering pipeline settings back to normal
	context->RSSetState(0);
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	// CODE END

	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// clear extra render targets
		context->ClearRenderTargetView(ppRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), depthBufferDSV.Get());

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		for (int i = 0; i < ents.size(); i++) {
			
			vs->SetMatrix4x4("lightView", shadowViewMatrix);
			vs->SetMatrix4x4("lightProjection", shadowProjectionMatrix);
			// set shader before drawing entity since most likely each entity will want to be drawn via a different shader instead of the same global one
			ents[i].GetMat()->GetVertexShader()->SetShader();
			ents[i].GetMat()->GetPixelShader()->SetShader();
			ents[i].Draw(cams[activeCam]);
		}

		for (int i = 0; i < sizeof(floor) / sizeof(floor[0]); i++)
		{
			for (int j = 0; j < sizeof(floor[0]) / sizeof(Ent); j++)
			{
				floor[i][j].GetMat()->GetVertexShader()->SetShader();
				floor[i][j].GetMat()->GetPixelShader()->SetShader();
				floor[i][j].Draw(cams[activeCam]);
			}
		}
	}

	// Draw sky last so pixelshader doesn't have to draw the part of the sky we can't see
	sky.Draw(context, cams, activeCam);

	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);

	// Activate shaders and bind resources
	// Also set any required cbuffer data (not shown)
	ppVS->SetShader();
	ppPS->SetShader();
	ppPS->SetShaderResourceView("Pixels", ppSRV.Get());
	ppPS->SetSamplerState("ClampSampler", ppSampler.Get());
	ppPS->SetInt("blurRadius", blurRadius);
	ppPS->SetFloat("pixelWidth", 1.0f / windowWidth);
	ppPS->SetFloat("pixelHeight", 1.0f / windowHeight);
	ppPS->CopyAllBufferData();
	context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)

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

		ID3D11ShaderResourceView* nullSRVs[128] = {};
		context->PSSetShaderResources(0, 128, nullSRVs);
	}

}