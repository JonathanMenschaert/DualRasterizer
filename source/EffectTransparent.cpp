#include "pch.h"
#include "EffectTransparent.h"
#include "Texture.h"

namespace dae
{
	EffectTransparent::EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile)
		:Effect(pDevice, assetFile)
	{
		//connect the different matrices to the hlsl file
		m_pMatWorldViewProjVar = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVar->IsValid())
		{
			std::wcout << L"m_pMatViewProjVar is not valid!\n";
		}

		//connect the texture maps to the hlsl file
		m_pDiffuseMapVar = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVar->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVar not valid\n";
		}
	}

	EffectTransparent::~EffectTransparent()
	{
		//Release resources
		if (m_pDiffuseMapVar) m_pDiffuseMapVar->Release();
	}

	//Diffuse map should be set at effect initialisation
	void EffectTransparent::SetDiffuseMap(Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVar)
		{
			m_pDiffuseMapVar->SetResource(pDiffuseTexture->GetSRV());
		}
	}

	EffectTransparent* EffectTransparent::CreateEffect(ID3D11Device* pDevice, const std::wstring& fxPath, const string& diffusePath)
	{
		//Initialize effect
		EffectTransparent* pEffect{ new EffectTransparent(pDevice, fxPath) };

		//Add texture to the respective variable if valid
		Texture* pDiffuseTexture{ nullptr };

		if (!diffusePath.empty()) pDiffuseTexture = Texture::LoadFromFile(pDevice, diffusePath);

		//Add textures to the effect
		pEffect->SetDiffuseMap(pDiffuseTexture);

		//Delete the textures as they are not necessary anymore
		delete pDiffuseTexture;
		
		return pEffect;
	}

	ID3D11InputLayout* EffectTransparent::CreateInputLayout(ID3D11Device* pDevice) const
	{
		//Create Vertex Layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TEXCOORD";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 12;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "NORMAL";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 20;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "TANGENT";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 32;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//Create Input Layout
		D3DX11_PASS_DESC passDesc{};
		GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

		ID3D11InputLayout* pInputLayout{ nullptr };

		HRESULT result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&pInputLayout
		);

		if (FAILED(result))
		{
			std::wcout << L"Input Layout creation failed!\n";
			return nullptr;
		}

		return pInputLayout;
	}
}
