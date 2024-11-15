// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "DXF.h"
#include "vector"

#define MAX_LIGHTS 8

using namespace std;
using namespace DirectX;


class ShadowShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView[MAX_LIGHTS];
		XMMATRIX lightProjection[MAX_LIGHTS];
	};

	struct CameraBufferType {
		XMFLOAT3 cameraPosition;
		float padding;
	};

	struct DirectionalLight
	{
		XMFLOAT4 diffuse;
		XMFLOAT4 ambient;

		XMFLOAT3 direction;
		float padding;

		XMFLOAT3 position;
		float padding2;
	};

	struct PointLight
	{
		XMFLOAT4 diffuse;
		XMFLOAT4 ambient;

		XMFLOAT3 direction;
		float padding;

		XMFLOAT3 position;
		float padding2;
	};

	struct SpotLight
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;

		XMFLOAT3 position;
		float range;

		XMFLOAT3 direction;
		float spot;

		XMFLOAT3 attenuation;
		float materialSpecularPower;

		XMFLOAT4 color;
		XMFLOAT4 emissive;


	};

	struct LightType
	{
		bool lightEnabled;
		XMFLOAT3 padding;

		DirectionalLight directionalLight;
		PointLight pointLight;
		SpotLight spotLight;
	};

	struct LightBufferType {
		LightType lights[MAX_LIGHTS];
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, 
		const XMMATRIX &projection, ID3D11ShaderResourceView* texture, std::vector <ShadowMap*> sceneMaps, std::vector<Light*> lights);

	CameraBufferType cameraInput;

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;

};

#endif