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

		virtual ColorRGB ShadePixel(const VertexOut& out, ShadingMode shadingMode, bool renderNormals) = 0;
		virtual void CycleSamplerState(ID3D11Device* pDevice);
		virtual void CycleCullMode(ID3D11Device* pDevice);

		CullMode GetCullMode() const;

	protected:
		ID3DX11Effect* m_pEffect{ nullptr };
		ID3DX11EffectTechnique* m_pTechnique{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVar{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatWorldVar{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatViewInverseVar{ nullptr };

		
		ID3DX11EffectSamplerVariable* m_pSamplerEffect;
		ID3DX11EffectRasterizerVariable* m_pRasterizerEffect;
		

		CullMode m_CullMode{CullMode::None};
		SampleState m_SamplerState{ SampleState::Point};
	};
}
