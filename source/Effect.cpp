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
}
