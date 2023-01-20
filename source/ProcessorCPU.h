#pragma once
#include "Processor.h"

namespace dae
{



	class ProcessorCPU final : public Processor
	{
	public:
		ProcessorCPU(SDL_Window* pWindow);
		virtual ~ProcessorCPU();

		ProcessorCPU(const ProcessorCPU& processor) = delete;
		ProcessorCPU(ProcessorCPU&& processor) noexcept = delete;
		ProcessorCPU& operator=(const ProcessorCPU& processor) = delete;
		ProcessorCPU& operator=(ProcessorCPU&& processor) noexcept = delete;


		virtual void Render(std::vector<Mesh*>& meshes, const Camera* camera) override;

	private:

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};

		void RenderMesh(std::vector<Mesh*>& meshes, const Camera* camera);
		void RenderMeshTriangle(Mesh* pMesh, const std::vector<Vector2>& screenVertices, uint32_t vertIdx, bool swapVertices = false);
		void VertexTransformationFunction(Mesh* pMesh, const Camera* camera) const;

		enum class RenderMode
		{
			FinalColor,
			DepthBuffer,

			//Declare modes above
			COUNT
		};

		enum class ShadingMode
		{
			Combined,
			ObservedArea,
			Diffuse,
			Specular,
			//Declare modes above
			COUNT
		};


		RenderMode m_RenderMode{ RenderMode::FinalColor };
		ShadingMode m_ShadingMode{ ShadingMode::Combined };
	};
}

