#pragma once
#include "DataTypes.h"
#include <vector>
namespace dae
{
	
	class Effect;
	class Mesh final
	{
	public:
		Mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Effect* pEffect,
			const Vector3& rotation, const Vector3& translation);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void UpdateSampleState(ID3D11SamplerState* pSampleState);
		void RotateY(float angle);
		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void SetMatrices(const Matrix& viewProjMatrix, const Matrix& inverseViewMatrix);

		const std::vector<VertexExt>& GetVertices() const;
		std::vector<VertexOut>& GetVerticesOut();
		const std::vector<uint32_t>& GetIndices() const;
		const Matrix& GetWorldMatrix() const;
		PrimitiveTopology GetPrimitiveTopology() const;

	private:
		Effect* m_pEffect{ nullptr };
		ID3D11Buffer* m_pVertexBuffer{ nullptr };
		ID3D11Buffer* m_pIndexBuffer{ nullptr };
		ID3D11InputLayout* m_pInputLayout{ nullptr };

		uint32_t m_NumIndices{};
		
		Matrix m_TranslationMatrix{};
		Matrix m_RotationMatrix{};
		Matrix m_WorldMatrix;

		PrimitiveTopology m_Topology{ PrimitiveTopology::TriangleList };

		std::vector<VertexExt> m_Vertices{};
		std::vector<VertexOut> m_VerticesOut{};
		std::vector<uint32_t> m_Indices{};
	};
}

