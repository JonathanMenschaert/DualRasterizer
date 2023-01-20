#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;
	using std::string;
	class EffectTransparent : public Effect
	{
	public:

		EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectTransparent();
		EffectTransparent(const EffectTransparent&) = delete;
		EffectTransparent(EffectTransparent&&) noexcept = delete;
		EffectTransparent& operator=(const EffectTransparent&) = delete;
		EffectTransparent& operator=(EffectTransparent&&) noexcept = delete;

		void SetDiffuseMap(Texture* pDiffuseTexture);

		static EffectTransparent* CreateEffect(ID3D11Device* pDevice, const std::wstring& fxPath, const string& diffusePath);

		virtual ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) const override;
		virtual ColorRGB ShadePixel(const VertexOut& out, ShadingMode shadingMode, bool renderNormals) override;
		virtual void CycleCullMode(ID3D11Device* pDevice) override;

	private:
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar{ nullptr };

		Texture* m_pDiffuseTexture{ nullptr };
	};
}

