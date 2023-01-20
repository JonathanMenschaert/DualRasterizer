#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "Texture.h"

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Effect* pEffect,
		const Vector3& rotation, const Vector3& translation)
		: m_pEffect{pEffect}
	{
		m_RotationMatrix = Matrix::CreateRotation(rotation);
		m_TranslationMatrix = Matrix::CreateTranslation(translation);

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

		m_Indices = indices;
		for (const Vertex& vertex : vertices)
		{
			VertexExt out{};
			out.position = vertex.position;
			out.uv = vertex.uv;
			out.normal = vertex.normal;
			out.tangent = vertex.tangent;

			m_Vertices.emplace_back(out);
		}
	}


	Mesh::~Mesh()
	{
		if (m_pEffect) delete m_pEffect;
		if (m_pIndexBuffer) m_pIndexBuffer->Release();		
		if (m_pVertexBuffer) m_pVertexBuffer->Release();		
		if (m_pInputLayout) m_pInputLayout->Release();		
	}


	void Mesh::RotateY(float angle)
	{
		m_RotationMatrix = Matrix::CreateRotationY(angle) * m_RotationMatrix;
	}


	void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
	{
		//1. Set Primitive Topology
		pDeviceContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(m_Topology));
		
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
		m_WorldMatrix = m_RotationMatrix * m_TranslationMatrix;
		m_pEffect->SetViewProjectionMatrix(m_WorldMatrix * viewProjMatrix);
		m_pEffect->SetWorldMatrix(m_WorldMatrix);
		m_pEffect->SetViewInverseMatrix(inverseViewMatrix);
	}
	const std::vector<VertexExt>& Mesh::GetVertices() const
	{
		return m_Vertices;
	}
	std::vector<VertexOut>& Mesh::GetVerticesOut()
	{
		return m_VerticesOut;
	}
	const std::vector<uint32_t>& Mesh::GetIndices() const
	{
		return m_Indices;
	}
	const Matrix& Mesh::GetWorldMatrix() const
	{
		return m_WorldMatrix;
	}
	PrimitiveTopology Mesh::GetPrimitiveTopology() const
	{
		return m_Topology;
	}
	void Mesh::ToggleRender()
	{
		m_ShouldRender = !m_ShouldRender;
	}
	void Mesh::CycleCullMode(ID3D11Device* pDevice)
	{
		m_pEffect->CycleCullMode(pDevice);
	}

	void Mesh::CycleSamplerState(ID3D11Device* pDevice)
	{
		m_pEffect->CycleSamplerState(pDevice);
	}

	bool Mesh::ShouldRender() const
	{
		return m_ShouldRender;
	}
	CullMode Mesh::GetCullMode() const
	{
		return CullMode();
	}
	ColorRGB Mesh::ShadePixel(const VertexOut& out, ShadingMode shadingMode, bool renderNormals)
	{
		return m_pEffect->ShadePixel(out, shadingMode, renderNormals);
	}
}