#pragma once
#include "DataTypes.h"
namespace dae
{
	class Effect
	{
	public:		
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect();
		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		ID3DX11Effect* GetEffect() const;
		ID3DX11EffectTechnique* GetTechnique() const;

		virtual ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) const = 0;

		void SetViewProjectionMatrix(const Matrix& matrix);
		void SetWorldMatrix(const Matrix& matrix);
		void SetViewInverseMatrix(const Matrix& matrix);

		//Shade pixel made virtual so each effect can shade the pixel based on it's properties
		//Also the Pixel Shading stage
		virtual ColorRGB ShadePixel(const VertexOut& out, ShadingMode shadingMode, const uint32_t currentColor, bool renderNormals) = 0;
		virtual void CycleSamplerState(ID3D11Device* pDevice);
		virtual void CycleCullMode(ID3D11Device* pDevice);
		virtual bool UseDepthBuffer() const;
		virtual bool UseMultiThreading() const;
		CullMode GetCullMode() const;
		SamplerState GetSamplerState() const;

	protected:

		//DirectX variables
		ID3DX11Effect* m_pEffect{ nullptr };
		ID3DX11EffectTechnique* m_pTechnique{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVar{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatWorldVar{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatViewInverseVar{ nullptr };

		
		ID3DX11EffectSamplerVariable* m_pSamplerEffect;
		ID3DX11EffectRasterizerVariable* m_pRasterizerEffect;
		
		//Enum variables
		CullMode m_CullMode{CullMode::None};
		SamplerState m_SamplerState{ SamplerState::Point};
	};
}
