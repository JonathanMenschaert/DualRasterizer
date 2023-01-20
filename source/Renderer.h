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

	private:

		void InitMeshes(ID3D11Device* pDevice);

		enum class ProcessorType
		{
			CPU,
			GPU
		};
		ProcessorType m_ProcessorType{ ProcessorType::CPU };

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		float m_AspectRatio{};

		bool m_IsInitialized{ false };
		bool m_ShouldRotate{ true };
		const float m_RotationSpeed{ TO_RADIANS * 45.f };

		std::vector<Mesh*> m_Meshes{};

		Camera m_Camera{};
		
		//Processors
		Processor* m_pRenderProcessor;
		ProcessorGPU* m_pProcessorGPU;
		ProcessorCPU* m_pProcessorCPU;
	};
}
