#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "Texture.h"

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Effect* pEffect)
		: m_pEffect{pEffect}
	{
		m_RotationMatrix = Matrix::CreateRotation(Vector3{ 0.f, 0.f, 0.f });

		m_pInputLayout = m_pEffect->CreateInputLayout(pDevice);

		//Create Vertex Buffer
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices.data();

		HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
		if (FAILED(result))
		{
			std::wcout << L"Vertex Buffer creation failed!\n";
			return;
		}

		//Create Index Buffer
		m_NumIndices = static_cast<uint32_t>(indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		initData.pSysMem = indices.data();
		
		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
		if (FAILED(result))
		{
			std::wcout << L"Index Buffer creation failed!\n";
			return;
		}
	}


	Mesh::~Mesh()
	{
		if (m_pEffect) delete m_pEffect;
		if (m_pIndexBuffer) m_pIndexBuffer->Release();		
		if (m_pVertexBuffer) m_pVertexBuffer->Release();		
		if (m_pInputLayout) m_pInputLayout->Release();		
	}


	void Mesh::UpdateSampleState(ID3D11SamplerState* pSampleState)
	{
		ID3DX11EffectSamplerVariable* pSamplerEffect{ m_pEffect->GetEffect()->GetVariableByName("gSampleState")->AsSampler() };
		HRESULT result{ pSamplerEffect->SetSampler(0, pSampleState) };
		if (FAILED(result))
		{
			std::wcout << L"Failed to update mesh sampler state\n";
			return;
		}
	}


	void Mesh::RotateY(float angle)
	{
		m_RotationMatrix = Matrix::CreateRotationY(angle) * m_RotationMatrix;
	}


	void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
	{
		//1. Set Primitive Topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2. Set Input Layout
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		//3. Set Vertex Buffer
		constexpr UINT stride = sizeof(Vertex);
		constexpr UINT offset = 0;
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		//4. Set IndexBuffer
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//5. Draw
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pEffect->GetTechnique()->GetDesc(&techDesc);

		for (UINT p{ 0 }; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}
	}

	void Mesh::SetMatrices(const Matrix& viewProjMatrix, const Matrix& inverseViewMatrix)
	{
		m_pEffect->SetViewProjectionMatrix(m_RotationMatrix * viewProjMatrix);
		m_pEffect->SetWorldMatrix(m_RotationMatrix);
		m_pEffect->SetViewInverseMatrix(inverseViewMatrix);
	}
}