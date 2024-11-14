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

	struct LightType
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		bool lightEnabled;
	};

	struct LightBufferType {
		LightType lights[MAX_LIGHTS];
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, 
		const XMMATRIX &projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView*depthMap, std::vector<Light*> lights);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
};

#endif