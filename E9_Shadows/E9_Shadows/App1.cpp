// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	lightVisualSphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 1024*4;
	int shadowmapHeight = 1024*4;
	int sceneWidth = 50;
	int sceneHeight = 50;

	// This is your shadow map
	shadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	shadowRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Configure directional light
	light = new Light();
	light->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(0.0f, -0.7f, 0.7f);
	light->setPosition(0.f, 0.f, -10.f);
	light->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	light2 = new Light();
	light2->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	light2->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light2->setDirection(0.0f, -0.7f, 0.7f);
	light2->setPosition(0.f, 0.f, -10.f);
	light2->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	shadowRT->setRenderTarget(renderer->getDeviceContext());
	shadowRT->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);
	
	shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	light->generateViewMatrix();
	light2->generateViewMatrix();

	XMMATRIX lightViewMatrix = light->getViewMatrix();
	XMMATRIX lightProjectionMatrix = light->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f + widthOffset, 7.f + heightOffset, 5.f + lengthOffset);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);

	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Render sphere
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f + widthOffset, 7.f + heightOffset, 15.f + lengthOffset);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Render cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f + widthOffset, 7.f + heightOffset, 10.f + lengthOffset);
	cube->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	
	std::vector<Light*> sceneLights = {};

	if (light1Enabled) {
		sceneLights.push_back(light);
	}
	if (light2Enabled) {
		sceneLights.push_back(light2);
	}
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, 
		textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), sceneLights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f + widthOffset, 7.f + heightOffset, 5.f + lengthOffset);

	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), sceneLights);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Render sphere
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f + widthOffset, 7.f + heightOffset, 15.f + lengthOffset);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), sceneLights);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	// Render cube
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f + widthOffset, 7.f + heightOffset, 10.f + lengthOffset);
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), sceneLights);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	// Render light sphere
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix *= 
		XMMatrixRotationRollPitchYaw(light2Direction[0], light2Direction[1], light2Direction[2]) 
		* XMMatrixTranslation(light2Position[0], light2Position[1], light2Position[2]);

	lightVisualSphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap->getDepthMapSRV(), sceneLights);
	shadowShader->render(renderer->getDeviceContext(), lightVisualSphere->getIndexCount());

	// Render ortho mesh
	if (showShadowDebug) {
		renderer->setZBuffer(false);

		float width = 600;
		float pixels_free = 600 - shadowRT->getTextureWidth();
		worldMatrix = renderer->getWorldMatrix();

		XMMATRIX orthoWorldMatrix = worldMatrix;
		//orthoWorldMatrix *= XMMatrixTranslation(pixels_free/2, 0, 0);

		XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
		orthoMesh->sendData(renderer->getDeviceContext());
		shadowShader->setShaderParameters(renderer->getDeviceContext(), orthoWorldMatrix, orthoViewMatrix, orthoMatrix, shadowRT->getShaderResourceView(), shadowMap->getDepthMapSRV(), sceneLights);
		shadowShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

		renderer->setZBuffer(true);
	}
	
	gui();
	renderer->endScene();
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::Checkbox("Shadow debug mode", &showShadowDebug);
	ImGui::Checkbox("Should scene move", &shouldMove);

	ImGui::Checkbox("Light 1 Enabled", &light1Enabled);
	ImGui::Checkbox("Light 2 Enabled", &light2Enabled);

	ImGui::SliderFloat("Light Width", &inputLightWidth, 5, 100);
	ImGui::SliderFloat("Light Length", &inputLightLength, 5, 100);

	ImGui::SliderFloat3("Light 1 Position", lightPosition, -100, 100);
	ImGui::SliderFloat3("Light 1 Direction", lightDirection, -180, 180);

	ImGui::SliderFloat3("Light 2 Position", light2Position, -100, 100);
	ImGui::SliderFloat3("Light 2 Direction", light2Direction, -180, 180);

	// Update settings
	light->generateOrthoMatrix((float)inputLightWidth, (float)inputLightLength, 0.1f, 100.f);
	light->setPosition(lightPosition[0], lightPosition[1], lightPosition[2]);
	light->setDirection(lightDirection[0]* 0.0174533, lightDirection[1]* 0.0174533, lightDirection[2]* 0.0174533);

	light2->generateOrthoMatrix((float)inputLightWidth, (float)inputLightLength, 0.1f, 100.f);
	light2->setPosition(light2Position[0], light2Position[1], light2Position[2]);
	light2->setDirection(light2Direction[0] * 0.0174533, light2Direction[1] * 0.0174533, light2Direction[2] * 0.0174533);

	if (shouldMove) {
		heightOffset += 0.01;
		widthOffset += 0.01;
		lengthOffset += 0.01;
	}
	
	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

