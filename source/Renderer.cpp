#include "pch.h"
#include "Renderer.h"
#include "Processor.h"
#include "ProcessorGPU.h"
#include "ProcessorCPU.h"
#include "Utils.h"
#include "Mesh.h"
#include "EffectOpaque.h"
#include "EffectTransparent.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height); //TODO check if this is necessary

		m_AspectRatio = static_cast<float>(m_Width) / m_Height;
		m_Camera.Initialize(45.f, { 0.f,0.f, 0.f }, m_AspectRatio);

		//Create Device & DeviceContext
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		ID3D11Device* pDevice;
		ID3D11DeviceContext* pDeviceContext;
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION,
			&pDevice, nullptr, &pDeviceContext);

		if (result == S_OK)
		{
			m_IsInitialized = true;
			InitMeshes(pDevice);
			std::cout << "Device and DeviceContext are initialized and ready!\n";
		}
		else
		{
			std::cout << "Device or DeviceContext initialization failed!\n";
		}

		m_pProcessorGPU = new ProcessorGPU(pDevice, pDeviceContext, pWindow);
		m_pProcessorCPU = new ProcessorCPU(pWindow);
		m_pRenderProcessor = m_pProcessorCPU;		
	}

	Renderer::~Renderer()
	{
		for (Mesh* pMesh : m_Meshes)
		{
			delete pMesh;
		}
		m_Meshes.clear();

		delete m_pProcessorGPU;
		m_pProcessorGPU = nullptr;

		delete m_pProcessorCPU;
		m_pProcessorCPU = nullptr;

	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		if (m_ShouldRotate)
		{
			for (Mesh* pMesh : m_Meshes)
			{
				pMesh->RotateY(pTimer->GetElapsed() * m_RotationSpeed);
				pMesh->SetMatrices(m_Camera.viewMatrix * m_Camera.projectionMatrix, m_Camera.invViewMatrix);
			}
		}
	}


	void Renderer::Render()
	{
		if (!m_IsInitialized) return;
		m_pRenderProcessor->Render(m_Meshes, &m_Camera);
	}

	void Renderer::ToggleProcessor()
	{
		switch (m_ProcessorType)
		{
		case ProcessorType::CPU:
			m_ProcessorType = ProcessorType::GPU;
			m_pRenderProcessor = m_pProcessorGPU;
			break;
		case ProcessorType::GPU:
			m_ProcessorType = ProcessorType::CPU;
			m_pRenderProcessor = m_pProcessorCPU;
			break;
		}
	}

	void Renderer::ToggleRotation()
	{
		m_ShouldRotate = !m_ShouldRotate;
	}

	void Renderer::InitMeshes(ID3D11Device* pDevice)
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		const Vector3 translation{ 0.f, 0.f, 50.f };
		const Vector3 rotation{ 0.f, 0.f, 0.f };

		//Vehicle
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		std::wstring fxPath{ L"Resources/EffectOpaque.fx" };
		std::string diffusePath{ "Resources/vehicle_diffuse.png" };
		const std::string normalPath{ "Resources/vehicle_normal.png" };
		const std::string specularPath{ "Resources/vehicle_specular.png" };
		const std::string glossinessPath{ "Resources/vehicle_gloss.png" };

		EffectOpaque* pVehicleEffect{ EffectOpaque::CreateEffect(pDevice, fxPath, diffusePath, normalPath, specularPath, glossinessPath) };

		m_Meshes.push_back(new Mesh(pDevice, vertices, indices, pVehicleEffect, rotation, translation));

		//Fire
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);

		fxPath = L"Resources/EffectTransparent.fx";
		diffusePath = "Resources/fireFX_diffuse.png";

		EffectTransparent* pFireEffect{EffectTransparent::CreateEffect(pDevice, fxPath, diffusePath)};
		m_Meshes.push_back(new Mesh(pDevice, vertices, indices, pFireEffect, rotation, translation));
	}
}
