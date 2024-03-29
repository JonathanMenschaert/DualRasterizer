#pragma once
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Processor;
	class ProcessorGPU;
	class ProcessorCPU;
	class Mesh;
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render();

		void ToggleProcessor();
		void ToggleRotation();
		void ToggleFireFx();
		void CycleSamplerState();
		void CycleCullMode();
		void ToggleNormalMap();
		void CycleShadingMode();
		void ToggleUniformColor();
		void ToggleBoundingBoxes();
		void CycleRenderMode();

	private:

		void PrintHeader() const;

		//Member variables
		enum class ProcessorType
		{
			CPU,
			GPU
		};
		ProcessorType m_ProcessorType{ ProcessorType::GPU };

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		float m_AspectRatio{};

		bool m_IsInitialized{ false };
		bool m_ShouldRotate{ true };
		const float m_RotationSpeed{ TO_RADIANS * 45.f };

		std::vector<Mesh*> m_Meshes{};
		Camera m_Camera{};

		//Background color
		bool m_IsBackgroundUniform{ false };

		//DirectX
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		void InitMeshes(ID3D11Device* pDevice);

		
		//Processors
		Processor* m_pRenderProcessor;
		ProcessorGPU* m_pProcessorGPU;
		ProcessorCPU* m_pProcessorCPU;
	};
}
