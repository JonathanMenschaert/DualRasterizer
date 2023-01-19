#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Processor;
	class ProcessorGPU;
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
		void Render() const;

	private:

		void InitMeshes();

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		//bool m_IsInitialized{ false };

		std::vector<Mesh*> m_Meshes{};
		
		//Processors
		Processor* m_pRenderProcessor;
		ProcessorGPU* m_pProcessorGPU;

	};
}
