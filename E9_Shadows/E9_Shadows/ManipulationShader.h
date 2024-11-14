#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class ManipulationShader : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

	struct TimeBufferType
	{
		float time;
		float amplitude;
		float frequency;
		float speed;

		XMFLOAT4 colourTexture;

		int isWaveManipulation;
		int isHeightMapManipulation;
		XMFLOAT2 padding;
	};

public:
	ManipulationShader(ID3D11Device* device, HWND hwnd);
	~ManipulationShader();

	void setActiveTimer(Timer* time) {
		currentTimer = time;
	}

	TimeBufferType inputtedTimeBuffer;

	ID3D11ShaderResourceView* colourMapTexture;

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, Light* light);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;

	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* timeBuffer;


	Timer* currentTimer;
};

