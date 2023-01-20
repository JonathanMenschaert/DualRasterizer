#pragma once
#include "Processor.h"

namespace dae
{
	struct Camera;
	class ProcessorGPU final : public Processor
	{
	public:
		ProcessorGPU(ID3D11Device* pDevice, ID3D11DeviceContext* m_pDeviceContext, SDL_Window* pWindow);
		virtual ~ProcessorGPU();

		ProcessorGPU(const ProcessorGPU& processor) = delete;
		ProcessorGPU(ProcessorGPU&& processor) noexcept = delete;
		ProcessorGPU& operator=(const ProcessorGPU& processor) = delete;
		ProcessorGPU& operator=(ProcessorGPU&& processor) noexcept = delete;

		virtual void Render(std::vector<Mesh*>& meshes, const Camera* camera) override;
		virtual void ToggleBackgroundColor(bool useUniformBg) override;

	private:
		
		//DirectX
		//Member functions
		HRESULT InitializeDirectX();


		//Member variables
		bool m_IsInitialized{ false };

		const ColorRGB m_HardwareColor{ 0.39f, 0.59f, 0.93f };

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;

		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;

		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		enum class SampleState
		{
			POINT,
			LINEAR,
			ANISOTROPIC,
			//Define samplestates above

			COUNT
		};

		SampleState m_SamplerState{ SampleState::POINT };
	};
}

