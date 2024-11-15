// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();

	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;
	OrthoMesh* orthoMesh;

	CubeMesh* cube;
	PlaneMesh* plane;
	SphereMesh* sphere;
	SphereMesh* lightVisualSphere;


	Light* light;
	Light* light2;

	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	RenderTexture* shadowRT;

	ShadowMap* shadowMap;
	ShadowMap* shadowMap2;


	float inputLightWidth = 50;
	float inputLightLength = 50;

	bool showShadowDebug = false;
	bool shouldMove = false;

	bool light1Enabled = false;
	bool light2Enabled = false;

	float heightOffset = 0;
	float widthOffset = 0;
	float lengthOffset = 0;

	float lightPosition[3] = { -35.0f, 16.0f, 14.0f };
	float lightDirection[3] = { 180.0f, -180.0f, 0.0f };

	float light2Position[3] = {35.0f, 16.0f, 14.0f };
	float light2Direction[3] = { -180.0f, -180.0f, 0.0f };

	float inputtedConeSize = 0.5f;
	float inputtedMatSpecPower = 0.0f;
	XMFLOAT4 inputtedSpotlightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 inputtedSpotlightSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 inputtedSpotlightEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	float inputtedSpotlightRange = 100.0f;
	XMFLOAT3 inputtedSpotlightAttenuation = XMFLOAT3(1.0f, 0.09f, 0.032f);
	XMFLOAT4 inputtedAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	XMFLOAT4 inputtedDiffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

};

#endif