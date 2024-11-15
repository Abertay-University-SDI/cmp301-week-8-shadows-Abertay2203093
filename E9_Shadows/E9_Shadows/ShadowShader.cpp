// texture shader.cpp
#include "shadowshader.h"


ShadowShader::ShadowShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"shadow_vs.cso", L"shadow_ps.cso");
}


ShadowShader::~ShadowShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (layout)
	{
		layout->Release();
		layout = 0;
	}
	if (lightBuffer)
	{	
		lightBuffer->Release();
		lightBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void ShadowShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Sampler for shadow map sampling.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	renderer->CreateSamplerState(&samplerDesc, &sampleStateShadow);

	// Setup light buffer
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

}


void ShadowShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, 
	const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture, std::vector<ShadowMap*> sceneMaps, std::vector<Light*> lights)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;
	CameraBufferType* cameraDataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);
	
	// Lock the constant buffer so it can be written to.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;

	// Add view for each light
	for (int i = 0; i < MAX_LIGHTS; i++) {
		if (i < lights.size()) {
			Light* light = lights.at(i);
			XMMATRIX tLightViewMatrix = XMMatrixTranspose(light->getViewMatrix());
			XMMATRIX tLightProjectionMatrix = XMMatrixTranspose(light->getOrthoMatrix());

			//XMMATRIX tLightViewMatrix = XMMatrixTranspose(light->getViewMatrix());
			//XMMATRIX tLightProjectionMatrix = XMMatrixTranspose(light->getProjectionMatrix());

			dataPtr->lightView[i] = tLightViewMatrix;
			dataPtr->lightProjection[i] = tLightProjectionMatrix;
		}		
	}
	
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Camera buffer
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraDataPtr = (CameraBufferType*)mappedResource.pData;
	cameraDataPtr->cameraPosition = cameraInput.cameraPosition;
	cameraDataPtr->padding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &cameraBuffer);

	//Additional
	// Send light data to pixel shader
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;

	// Add view for each light
	for (int i = 0; i < MAX_LIGHTS; i++) {
		if (i < lights.size()) {
			Light* light = lights.at(i);
			bool isSpotlight = false;

			if (light != nullptr) {

				if (isSpotlight) {
					lightPtr->lights[i].spotLight.ambient = light->getAmbientColour(); // XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //
					lightPtr->lights[i].spotLight.diffuse = light->getDiffuseColour(); // XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); //
					lightPtr->lights[i].spotLight.position = light->getPosition();
					lightPtr->lights[i].spotLight.specular = light->getSpecularColour(); //XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
					lightPtr->lights[i].spotLight.direction = light->getDirection();
					lightPtr->lights[i].spotLight.spot = 96.0f;// light->getSpotlightCone(); //96.0f;
					lightPtr->lights[i].spotLight.range = 400.0f; //light->getSpotlightRange(); //  400.0f;//
					lightPtr->lights[i].spotLight.attenuation = XMFLOAT3(1.0f, 0.0f, 0.0f); //light->getSpotlightAttenuation(); // XMFLOAT3(1.0f, 0.0f, 0.0f);  //
					lightPtr->lights[i].spotLight.materialSpecularPower = 1.0f; //light->getMaterialSpecularPower(); //1.0f;
					lightPtr->lights[i].spotLight.color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); //light->getSpotlightColor(); //XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
					lightPtr->lights[i].spotLight.emissive = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); //light->getSpotlightEmissive(); // XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
				}
				else {
					lightPtr->lights[i].directionalLight.ambient = light->getAmbientColour();
					lightPtr->lights[i].directionalLight.diffuse = light->getDiffuseColour();
					lightPtr->lights[i].directionalLight.direction = light->getDirection();
					lightPtr->lights[i].directionalLight.position = light->getPosition();

					lightPtr->lights[i].lightEnabled = true;
				}
				
			}
		}
		else {
			lightPtr->lights[i].lightEnabled = false;
		}
	}

	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	// Load shadow maps
	for (int i = 0; i < sceneMaps.size(); i++) {
		ID3D11ShaderResourceView* depthMap = sceneMaps.at(i)->getDepthMapSRV();
		deviceContext->PSSetShaderResources(1+i, 1, &depthMap);
	}

	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleStateShadow);
}

