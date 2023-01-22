#include "pch.h"
#include "EffectOpaque.h"
#include "Texture.h"
#include "Utils.h"

namespace dae
{
	EffectOpaque::EffectOpaque(ID3D11Device* pDevice, const std::wstring& assetFile)
		: Effect(pDevice, assetFile)
	{
		//connect the different matrices to the hlsl file
		m_pMatWorldViewProjVar = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVar->IsValid())
		{
			std::wcout << L"m_pMatViewProjVar is not valid!\n";
		}

		m_pMatWorldVar = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
		if (!m_pMatWorldVar->IsValid())
		{
			std::wcout << L"m_pMatWorldVar is not valid!\n";
		}
		m_pMatViewInverseVar = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pMatViewInverseVar->IsValid())
		{
			std::wcout << L"m_pMatViewInverseVar is not valid!\n";
		}

		//connect the texture maps to the hlsl file
		m_pDiffuseMapVar = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVar->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVar not valid\n";
		}

		m_pNormalMapVar = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVar->IsValid())
		{
			std::wcout << L"m_pNormalMapVar not valid\n";
		}

		m_pSpecularMapVar = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVar->IsValid())
		{
			std::wcout << L"m_pSpecularMapVar not valid\n";
		}

		m_pGlossinessMapVar = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossinessMapVar->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVar not valid\n";
		}

		//Set Cullmode
		m_CullMode = CullMode::Back;
	}

	EffectOpaque::~EffectOpaque()
	{
		//Release resources
		if (m_pDiffuseMapVar) m_pDiffuseMapVar->Release();
		if (m_pNormalMapVar) m_pNormalMapVar->Release();
		if (m_pSpecularMapVar) m_pSpecularMapVar->Release();
		if (m_pGlossinessMapVar) m_pGlossinessMapVar->Release();

		delete m_pDiffuseTexture;
		delete m_pNormalTexture;
		delete m_pSpecularTexture;
		delete m_pGlossinessTexture;
	}

	//Diffuse map should be set at effect initialisation
	void EffectOpaque::SetDiffuseMap(Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVar && pDiffuseTexture)
		{
			m_pDiffuseTexture = pDiffuseTexture;
			m_pDiffuseMapVar->SetResource(pDiffuseTexture->GetSRV());
		}
	}

	//Normal map should be set at effect initialisation
	void EffectOpaque::SetNormalMap(Texture* pNormalTexture)
	{
		if (m_pNormalMapVar && pNormalTexture)
		{
			m_pNormalTexture = pNormalTexture;
			m_pNormalMapVar->SetResource(pNormalTexture->GetSRV());
		}
	}

	//Specular map should be set at effect initialisation
	void EffectOpaque::SetSpecularMap(Texture* pSpecularTexture)
	{
		if (m_pSpecularMapVar && pSpecularTexture)
		{
			m_pSpecularTexture = pSpecularTexture;
			m_pSpecularMapVar->SetResource(pSpecularTexture->GetSRV());
		}
	}

	//Glossiness map should be set at effect initialisation
	void EffectOpaque::SetGlossinessMap(Texture* pGlossinessTexture)
	{
		if (m_pGlossinessMapVar && pGlossinessTexture)
		{
			m_pGlossinessTexture = pGlossinessTexture;
			m_pGlossinessMapVar->SetResource(pGlossinessTexture->GetSRV());
		}
	}

	//Factory function for the opaque effect
	EffectOpaque* EffectOpaque::CreateEffect(ID3D11Device* pDevice, const std::wstring& fxPath, const string& diffusePath, 
		const string& normalPath, const string& specularPath, const string& glossinessPath)
	{
		//Initialize effect
		EffectOpaque* pEffect{ new EffectOpaque(pDevice, fxPath) };

		//Add texture to the respective variable if valid
		Texture* pDiffuseTexture{ nullptr };
		Texture* pNormalTexture{ nullptr };
		Texture* pSpecularTexture{ nullptr };
		Texture* pGlossinessTexture{ nullptr };

		if (!diffusePath.empty()) pDiffuseTexture = Texture::LoadFromFile(pDevice, diffusePath);
		if (!normalPath.empty()) pNormalTexture = Texture::LoadFromFile(pDevice, normalPath);
		if (!specularPath.empty()) pSpecularTexture = Texture::LoadFromFile(pDevice, specularPath);
		if (!glossinessPath.empty()) pGlossinessTexture = Texture::LoadFromFile(pDevice, glossinessPath);

		//Add textures to the effect

		pEffect->SetDiffuseMap(pDiffuseTexture);
		pEffect->SetNormalMap(pNormalTexture);
		pEffect->SetSpecularMap(pSpecularTexture);
		pEffect->SetGlossinessMap(pGlossinessTexture);

		return pEffect;
	}

	ID3D11InputLayout* EffectOpaque::CreateInputLayout(ID3D11Device* pDevice) const
	{
		//Create Vertex Layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "NORMAL";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 20;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 32;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

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

		//Return nullptr if initialisation failed, 
		//otherwise return the ptr to the initialised layout
		if (FAILED(result))
		{
			std::wcout << L"Input Layout creation failed!\n";
			return nullptr;
		}

		return pInputLayout;
	}

	ColorRGB EffectOpaque::ShadePixel(const VertexOut& out, ShadingMode shadingMode, const uint32_t currentColor, bool renderNormals)
	{
		Vector3 sampledNormal{ out.normal };

		if (renderNormals)
		{
			const Vector3 binormal{ Vector3::Cross(out.normal, out.tangent) };
			const Matrix tangentSpaceAxis{ out.tangent, binormal.Normalized(), out.normal, Vector3{0.f, 0.f, 0.f} };
			sampledNormal = m_pNormalTexture->SampleNormal(out.uv);
			sampledNormal = (2.f * sampledNormal) - Vector3{ 1.f, 1.f, 1.f };
			sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);
			sampledNormal.Normalize();
		}

		const float observedArea{ std::max(Vector3::Dot(sampledNormal, -m_LightDirection), 0.f) };
		const ColorRGB observedAreaColor{ observedArea, observedArea, observedArea };

		switch (shadingMode)
		{
		case ShadingMode::Combined:
		{
			const ColorRGB diffuse{ dae::BRDF::Lambert(m_Kd, m_pDiffuseTexture->Sample(out.uv)) * m_LightIntensity };
			const ColorRGB specular{ BRDF::Phong(m_pSpecularTexture->Sample(out.uv), 1.f, m_pGlossinessTexture->Sample(out.uv).r * m_Shininess,
				m_LightDirection, -out.viewDirection, sampledNormal) };
			return diffuse * observedArea + specular + m_AmbientLight;
		}
		case ShadingMode::ObservedArea:
		{
			return observedAreaColor;
		}
		case ShadingMode::Diffuse:
		{
			const ColorRGB diffuse{ BRDF::Lambert(m_Kd, m_pDiffuseTexture->Sample(out.uv) * m_LightIntensity) };
			return diffuse * observedAreaColor;
		}
		case ShadingMode::Specular:
		{
			const ColorRGB specular{ BRDF::Phong(m_pSpecularTexture->Sample(out.uv), 1.f, m_pGlossinessTexture->Sample(out.uv).r * m_Shininess,
				m_LightDirection, -out.viewDirection, sampledNormal) };
			return specular;
		}
		default:
			return ColorRGB{};
		}
	}
	
}