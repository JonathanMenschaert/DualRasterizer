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
		void ToggleBoundingBoxes();
		void CycleRenderMode();
		void CycleShadingMode();

	private:

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};

		void ProjectMesh(std::vector<Mesh*>& meshes, const Camera* camera);
		void RasterizeTriangle(Mesh* pMesh, const std::vector<Vector2>& screenVertices, uint32_t vertIdx, bool swapVertices = false);
		void VertexTransformationFunction(Mesh* pMesh, const Camera* camera) const;
		bool IsValidPixelForCullMode(CullMode mode, float areaV0V1, float areaV1V2, float areaV2V0) const;

		const ColorRGB m_SoftwareColor{ 0.39f, 0.39f, 0.39f };
		const float m_ColorModifier{ 1.f / 255.f };
		bool m_ShouldRenderNormals{ true };
		bool m_ShouldRenderBoundingBoxes{ false };
		RenderMode m_RenderMode{ RenderMode::FinalColor };
		ShadingMode m_ShadingMode{ ShadingMode::Combined };
	};
}

