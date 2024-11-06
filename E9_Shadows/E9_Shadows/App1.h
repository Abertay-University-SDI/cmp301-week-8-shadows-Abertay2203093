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
	SphereMesh* sphere;
	SphereMesh* lightVisualSphere;


	Light* light;
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	RenderTexture* shadowRT;

	ShadowMap* shadowMap;


	float inputLightWidth = 50;
	float inputLightLength = 50;

	bool showShadowDebug = false;

	float heightOffset = 0;
	float widthOffset = 0;
	float lengthOffset = 0;

	float lightPosition[3] = {0, 0, -10};
	float lightDirection[3] = { 0.0f, -0.7f, 0.7f };
};

#endif