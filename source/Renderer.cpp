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

		//Create Device & DeviceContext to initialize the meshes
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION,
			&m_pDevice, nullptr, &m_pDeviceContext);

		if (result == S_OK)
		{
			m_IsInitialized = true;
			InitMeshes(m_pDevice);
		}
		else
		{
			std::wcout << "Device or DeviceContext initialization failed!\n";
		}

		//Initialize the two renderers, default GPU
		m_pProcessorGPU = new ProcessorGPU(m_pDevice, m_pDeviceContext, pWindow);
		m_pProcessorCPU = new ProcessorCPU(pWindow);
		m_pRenderProcessor = m_pProcessorGPU;		

		//Print application header
		PrintHeader();
	}

	Renderer::~Renderer()
	{
		//Release and delete resources
		for (Mesh* pMesh : m_Meshes)
		{
			delete pMesh;
		}
		m_Meshes.clear();

		delete m_pProcessorGPU;
		m_pProcessorGPU = nullptr;

		delete m_pProcessorCPU;
		m_pProcessorCPU = nullptr;

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		
		for (Mesh* pMesh : m_Meshes)
		{
			//only rotate when enabled
			if (m_ShouldRotate)
			{
				pMesh->RotateY(pTimer->GetElapsed() * m_RotationSpeed);
			}

			//Update matrices
			pMesh->SetMatrices(m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix(), m_Camera.GetInvViewMatrix());
		}
	}


	void Renderer::Render()
	{
		if (!m_IsInitialized) return;
		m_pRenderProcessor->Render(m_Meshes, &m_Camera);
	}

	void Renderer::ToggleProcessor()
	{
		//Switch between the render processor
		switch (m_ProcessorType)
		{
		case ProcessorType::CPU:
			m_ProcessorType = ProcessorType::GPU;
			m_pRenderProcessor = m_pProcessorGPU;
			std::wcout << "\033[33m" << "**(Shared) Rasterizer Mode = HARDWARE" << "\033[0m" << "\n";
			break;
		case ProcessorType::GPU:
			m_ProcessorType = ProcessorType::CPU;
			m_pRenderProcessor = m_pProcessorCPU;
			std::wcout << "\033[33m" << "**(Shared) Rasterizer Mode = SOFTWARE" << "\033[0m" << "\n";
			break;
		}
	}

	void Renderer::ToggleRotation()
	{
		m_ShouldRotate = !m_ShouldRotate;

		std::wcout << "\033[33m" << "**(Shared) Vehicle Rotation " << (m_ShouldRotate ? "ON" : "OFF") << "\033[0m" << "\n";
	}

	void Renderer::ToggleFireFx()
	{
		m_Meshes[1]->ToggleRender();
	}

	void Renderer::CycleSamplerState()
	{
		//Only cycle the shading mode when the current processor is the gpu
		if (m_ProcessorType != ProcessorType::GPU) return;
		for (Mesh* pMesh : m_Meshes)
		{
			pMesh->CycleSamplerState(m_pDevice);
		}

		switch (m_Meshes[0]->GetSamplerState())
		{
		case SamplerState::Point:
			std::wcout << "\033[32m" << "**(HARDWARE) Sampler Filter = POINT" << "\033[0m" << "\n";
			break;
		case SamplerState::Linear:
			std::wcout << "\033[32m" << "**(HARDWARE) Sampler Filter = LINEAR" << "\033[0m" << "\n";
			break;
		case SamplerState::Anisotropic:
			std::wcout << "\033[32m" << "**(HARDWARE) Sampler Filter = ANISOTROPIC" << "\033[0m" << "\n";
			break;
		default:
			break;
		}
	}

	void Renderer::CycleCullMode()
	{
		for (Mesh* pMesh : m_Meshes)
		{
			pMesh->CycleCullMode(m_pDevice);
		}

		switch (m_Meshes[0]->GetCullMode())
		{
		case CullMode::None:
			std::wcout << "\033[33m" << "**(Shared) CullMode = NONE" << "\033[0m" << "\n";
			break;
		case CullMode::Back:
			std::wcout << "\033[33m" << "**(Shared) CullMode = BACK" << "\033[0m" << "\n";
			break;
		case CullMode::Front:
			std::wcout << "\033[33m" << "**(Shared) CullMode = FRONT" << "\033[0m" << "\n";
			break;
		default:
			break;
		}
	}

	void Renderer::ToggleNormalMap()
	{
		//Only cycle the shading mode when the current processor is the cpu
		if (m_ProcessorType != ProcessorType::CPU) return;
		m_pProcessorCPU->ToggleNormalMap();
	}

	void Renderer::CycleShadingMode()
	{
		//Only cycle the shading mode when the current processor is the cpu
		if (m_ProcessorType != ProcessorType::CPU) return;
		m_pProcessorCPU->CycleShadingMode();
	}

	void Renderer::ToggleUniformColor()
	{
		//Toggle the background color of both processors
		m_IsBackgroundUniform = !m_IsBackgroundUniform;
		m_pProcessorCPU->ToggleBackgroundColor(m_IsBackgroundUniform);
		m_pProcessorGPU->ToggleBackgroundColor(m_IsBackgroundUniform);
		std::wcout << "\033[33m" << "**(Shared) Uniform ClearColor " << (m_IsBackgroundUniform ? "ON" : "OFF") << "\033[0m" << "\n";
	}

	void Renderer::ToggleBoundingBoxes()
	{
		if (m_ProcessorType != ProcessorType::CPU) return;
		m_pProcessorCPU->ToggleBoundingBoxes();
	}

	void Renderer::CycleRenderMode()
	{
		if (m_ProcessorType != ProcessorType::CPU) return;
		m_pProcessorCPU->CycleRenderMode();
	}

	void Renderer::InitMeshes(ID3D11Device* pDevice)
	{
		//Initialize mesh variables
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		const Vector3 translation{ 0.f, 0.f, 50.f };
		const Vector3 rotation{ 0.f, 0.f, 0.f };

		//Vehicle resources
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		std::wstring fxPath{ L"Resources/EffectOpaque.fx" };
		std::string diffusePath{ "Resources/vehicle_diffuse.png" };
		const std::string normalPath{ "Resources/vehicle_normal.png" };
		const std::string specularPath{ "Resources/vehicle_specular.png" };
		const std::string glossinessPath{ "Resources/vehicle_gloss.png" };

		EffectOpaque* pVehicleEffect{ EffectOpaque::CreateEffect(pDevice, fxPath, diffusePath, normalPath, specularPath, glossinessPath) };

		m_Meshes.push_back(new Mesh(pDevice, vertices, indices, pVehicleEffect, rotation, translation));

		//FireFX resources
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);

		fxPath = L"Resources/EffectTransparent.fx";
		diffusePath = "Resources/fireFX_diffuse.png";

		EffectTransparent* pFireEffect{EffectTransparent::CreateEffect(pDevice, fxPath, diffusePath)};
		m_Meshes.push_back(new Mesh(pDevice, vertices, indices, pFireEffect, rotation, translation));
	}


	void Renderer::PrintHeader() const
	{
		//Shared Keybindings
		std::wcout << "\033[33m" << "[Key Bindings - SHARED]\n";
		std::wcout << "\t[F1]\tToggle Rasterizer Mode (HARDWARE/SOFTWARE)\n";
		std::wcout << "\t[F2]\tToggle Vehicle Rotation (ON/OFF)\n";
		std::wcout << "\t[F3]\tToggle FireFX (ON/OFF)\n";
		std::wcout << "\t[F9]\tCycle CullMode (BACK/FRONT/NONE)\n";
		std::wcout << "\t[F10]\tUniform ClearColor (ON/OFF)\n";
		std::wcout << "\t[F11]\tToggle Print FPS (ON/OFF)" << "\033[0m" << "\n\n";

		//Hardware Keybindings
		std::wcout << "\033[32m" << "[Key Bindings - HARDWARE]\n";
		std::wcout << "\t[F4]\tCycle Sampler State (POINT/LINEAR/ANISOTROPIC)" << "\033[0m" << "\n\n";

		//Software Keybindings
		std::wcout << "\033[35m" << "[Key Bindings - SOFTWARE]\n";
		std::wcout << "\t[F5]\tToggle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n";
		std::wcout << "\t[F6]\tToggle NormalMap (ON/OFF)\n";
		std::wcout << "\t[F7]\tToggle DepthBuffer Visualization (ON/OFF)\n";
		std::wcout << "\t[F8]\tToggle BoundingBox Visualization (ON/OFF)" << "\033[0m" << "\n\n";

		//Software Extras
		std::wcout << "\033[36m" << "[Extras - SOFTWARE]\n";
		std::wcout << "\t[+]\tMultithreading\n";
		std::wcout << "\t[+]\tFireFX" << "\033[0m" << "\n\n";
	}
}
