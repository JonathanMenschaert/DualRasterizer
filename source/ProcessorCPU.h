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
		virtual void ToggleBackgroundColor(bool useUniformBg) override;
		void ToggleNormalMap();
		void CycleShadingMode();

	private:

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};

		void RenderMesh(std::vector<Mesh*>& meshes, const Camera* camera);
		void RenderMeshTriangle(Mesh* pMesh, const std::vector<Vector2>& screenVertices, uint32_t vertIdx, bool swapVertices = false);
		void VertexTransformationFunction(Mesh* pMesh, const Camera* camera) const;
		bool IsValidForCullMode(CullMode mode, float areaV0V1, float areaV1V2, float areaV2V0) const;

		enum class RenderMode
		{
			FinalColor,
			DepthBuffer,

			//Declare modes above
			COUNT
		};

		const ColorRGB m_SoftwareColor{ 0.39f, 0.39f, 0.39f };
		bool m_ShouldRenderNormals{ true };
		RenderMode m_RenderMode{ RenderMode::FinalColor };
		ShadingMode m_ShadingMode{ ShadingMode::Combined };
	};
}

