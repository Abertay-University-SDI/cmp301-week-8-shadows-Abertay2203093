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


	float inputLightWidth = 50;
	float inputLightLength = 50;

	bool showShadowDebug = false;
	bool shouldMove = false;

	bool light1Enabled = false;
	bool light2Enabled = false;

	float heightOffset = 0;
	float widthOffset = 0;
	float lengthOffset = 0;

	float lightDirection[3] = {-16, -41, 75};
	float lightPosition[3] = { -15.0f, 16.0f, -7.7f };

	float light2Direction[3] = { -16, -41, 75 };
	float light2Position[3] = { 15.0f, 16.0f, -7.7f };
};

#endif