#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;
	class EffectTransparant : public Effect
	{
	public:

		EffectTransparant(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectTransparant();
		EffectTransparant(const EffectTransparant&) = delete;
		EffectTransparant(EffectTransparant&&) noexcept = delete;
		EffectTransparant& operator=(const EffectTransparant&) = delete;
		EffectTransparant& operator=(EffectTransparant&&) noexcept = delete;

		void SetDiffuseMap(Texture* pDiffuseTexture);

		virtual ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) const;

	private:
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar{ nullptr };
	};
}

