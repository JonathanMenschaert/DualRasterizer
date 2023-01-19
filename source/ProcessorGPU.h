#pragma once
#include "Processor.h"

namespace dae
{
	struct Camera;
	class ProcessorGPU final : public Processor
	{
	public:
		ProcessorGPU(SDL_Window* pWindow);
		virtual ~ProcessorGPU() = default;

		ProcessorGPU(const ProcessorGPU& processor) = delete;
		ProcessorGPU(ProcessorGPU&& processor) noexcept = delete;
		ProcessorGPU& operator=(const ProcessorGPU& processor) = delete;
		ProcessorGPU& operator=(ProcessorGPU&& processor) noexcept = delete;


		virtual void Render(std::vector<Mesh*> meshes, const Camera* camera) const override;
	
	private:
		
		//DirectX
		//Member functions
		HRESULT InitializeDirectX();


		//Member variables
		bool m_IsInitialized{ false };

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;

		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;

		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		ID3D11SamplerState* m_pSamplerState{ nullptr };

	};
}

