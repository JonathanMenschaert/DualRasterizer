#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;
	using std::string;

    class EffectOpaque final : public Effect
    {
	public:
		EffectOpaque(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectOpaque();
		EffectOpaque(const EffectOpaque&) = delete;
		EffectOpaque(EffectOpaque&&) noexcept = delete;
		EffectOpaque& operator=(const EffectOpaque&) = delete;
		EffectOpaque& operator=(EffectOpaque&&) noexcept = delete;

		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);

		static EffectOpaque* CreateEffect(ID3D11Device* pDevice, const std::wstring& fxPath, const string& diffusePath, 
			const string& normalPath, const string& specularPath, const string& glossinessPath);

		virtual ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) const override;
		virtual ColorRGB ShadePixel(const VertexOut& out, ShadingMode shadingMode, const uint32_t currentColor, bool renderNormals) override;

	private:
		//DirectX variables
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVar{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVar{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVar{ nullptr };

		//Textures
		Texture* m_pDiffuseTexture{ nullptr };
		Texture* m_pNormalTexture{ nullptr };
		Texture* m_pSpecularTexture{ nullptr };
		Texture* m_pGlossinessTexture{ nullptr };

		//Light calculation 
		const float m_LightIntensity{ 7.f };
		const float m_Kd{ 1.f };
		const float m_Shininess{ 25.f };
		const ColorRGB m_AmbientLight{ 0.025f, 0.025f, 0.025f };
		const Vector3 m_LightDirection{ 0.577f, -0.577f, 0.577f };
    };
}
