#include "pch.h"
#include "Effect.h"

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
		:m_pEffect{ LoadEffect(pDevice, assetFile) }
	{
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pRasterizerEffect = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();
		if (!m_pRasterizerEffect->IsValid())
		{
			std::wcout << L"Rasterizer not valid\n";
		}

		m_pSamplerEffect = m_pEffect->GetVariableByName("gSampleState")->AsSampler();
		if (!m_pSamplerEffect->IsValid())
		{
			std::wcout << L"Rasterizer not valid\n";
		}
	}

	Effect::~Effect()
	{
		//Release the matrices
		if (m_pMatWorldViewProjVar)
		{
			m_pMatWorldViewProjVar->Release();
			m_pMatWorldViewProjVar = nullptr;
		}
		if (m_pMatWorldVar)
		{
			m_pMatWorldVar->Release();
			m_pMatWorldVar = nullptr;
		}
		if (m_pMatViewInverseVar)
		{
			m_pMatViewInverseVar->Release();
			m_pMatViewInverseVar = nullptr;
		}

		if (m_pRasterizerEffect)
		{
			m_pRasterizerEffect->Release();
			m_pRasterizerEffect = nullptr;
		}

		if (m_pSamplerEffect)
		{
			m_pSamplerEffect->Release();
			m_pSamplerEffect = nullptr;
		}

		//Release the technique & effect
		if (m_pTechnique)
		{
			m_pTechnique->Release();
			m_pTechnique = nullptr;
		}
		if (m_pEffect)
		{
			m_pEffect->Release();
			m_pEffect = nullptr;
		}
	}

	ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags{ 0 };
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		//Try to compile the fx file
		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		//Handle & Report compilation errors
		if (FAILED(result))
		{
			if (pErrorBlob)
			{
				const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}
				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
			}
			return nullptr;
		}

		return pEffect;
	}

	ID3DX11Effect* Effect::GetEffect() const
	{
		return m_pEffect;
	}

	ID3DX11EffectTechnique* Effect::GetTechnique() const
	{
		return m_pTechnique;
	}

	void Effect::SetViewProjectionMatrix(const Matrix& matrix)
	{
		if (!m_pMatWorldViewProjVar) return;
		m_pMatWorldViewProjVar->SetMatrix(reinterpret_cast<const float*>(&matrix));
	}

	void Effect::SetWorldMatrix(const Matrix& matrix)
	{
		if (!m_pMatWorldVar) return;
		m_pMatWorldVar->SetMatrix(reinterpret_cast<const float*>(&matrix));
	}
	void Effect::SetViewInverseMatrix(const Matrix& matrix)
	{
		if (!m_pMatViewInverseVar) return;
		m_pMatViewInverseVar->SetMatrix(reinterpret_cast<const float*>(&matrix));
	}

	//SamplerState desc: https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc
	void Effect::CycleSamplerState(ID3D11Device* pDevice)
	{
		int state{ static_cast<int>(m_SamplerState) + 1 };
		int count{ static_cast<int>(SamplerState::COUNT) };
		m_SamplerState = static_cast<SamplerState>(state % count);

		D3D11_SAMPLER_DESC sampleDesc{};
		sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampleDesc.MipLODBias = 0;
		sampleDesc.MinLOD = 0;
		sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
		sampleDesc.MaxAnisotropy = 16;

		switch (m_SamplerState)
		{
		case SamplerState::Point:
			sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case SamplerState::Linear:
			sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case SamplerState::Anisotropic:
			sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			break;
		default:
			break;
		}
		ID3D11SamplerState* pSamplerState{ nullptr };
		HRESULT result{ pDevice->CreateSamplerState(&sampleDesc, &pSamplerState) };
		if (FAILED(result))
		{
			if (pSamplerState) pSamplerState->Release();
			std::wcout << L"Failed to create new Sampler State!\n";
			return;
		}

		result = m_pSamplerEffect->SetSampler(0, pSamplerState);

		if (FAILED(result))
		{
			std::wcout << L"Failed to update Sampler State\n";
		}
		if (pSamplerState) pSamplerState->Release();
	}

	//Rasterizer desc: https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_rasterizer_desc
	void Effect::CycleCullMode(ID3D11Device* pDevice)
	{
		int state{ static_cast<int>(m_CullMode) + 1 };
		int count{ static_cast<int>(CullMode::COUNT) };
		m_CullMode = static_cast<CullMode>(state % count);

		D3D11_RASTERIZER_DESC rastDesc{};
		
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.DepthBias = 0;
		rastDesc.SlopeScaledDepthBias = 0.f;
		rastDesc.DepthBiasClamp = 0.f; 
		rastDesc.DepthClipEnable = true;
		rastDesc.ScissorEnable = false;
		rastDesc.MultisampleEnable = false;
		rastDesc.AntialiasedLineEnable = false;
		

		switch (m_CullMode)
		{
		case CullMode::None:
			rastDesc.CullMode = D3D11_CULL_NONE;
			break;
		case CullMode::Back:
			rastDesc.CullMode = D3D11_CULL_BACK;
			break;
		case CullMode::Front:
			rastDesc.CullMode = D3D11_CULL_FRONT;
			break;
		default:
			break;
		}
		ID3D11RasterizerState* pRasterizerState{ nullptr };
		HRESULT result{ pDevice->CreateRasterizerState(&rastDesc, &pRasterizerState) };
		if (FAILED(result))
		{
			if (pRasterizerState) pRasterizerState->Release();
			std::wcout << L"Failed to create new Rasterizer State!\n";
			return;
		}
		result = m_pRasterizerEffect->SetRasterizerState(0, pRasterizerState);
		if (FAILED(result))
		{
			std::wcout << L"Failed to update Rasterizer State!\n";
		}
		if (pRasterizerState) pRasterizerState->Release();
	}

	bool Effect::UseDepthBuffer() const
	{
		return true;
	}

	bool Effect::UseMultiThreading() const
	{
		return true;
	}

	CullMode Effect::GetCullMode() const
	{
		return m_CullMode;
	}
	SamplerState Effect::GetSamplerState() const
	{
		return m_SamplerState;
	}
}
