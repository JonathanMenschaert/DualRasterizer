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

		void RotateY(float angle);
		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void SetMatrices(const Matrix& viewProjMatrix, const Matrix& inverseViewMatrix);

		const std::vector<VertexExt>& GetVertices() const;
		std::vector<VertexOut>& GetVerticesOut();
		const std::vector<uint32_t>& GetIndices() const;
		const Matrix& GetWorldMatrix() const;
		PrimitiveTopology GetPrimitiveTopology() const;

		void ToggleRender();
		void CycleCullMode(ID3D11Device* pDevice);
		void CycleSamplerState(ID3D11Device* pDevice);
		bool ShouldRender() const;

		CullMode GetCullMode() const;
		SamplerState GetSamplerState() const;
		ColorRGB ShadePixel(const VertexOut& out, ShadingMode shadingMode, const uint32_t currentColor, bool renderNormals);
		bool UseDepthBuffer() const;
		bool UseMultiThreading() const;

	private:
		//DirectX variables
		Effect* m_pEffect{ nullptr };
		ID3D11Buffer* m_pVertexBuffer{ nullptr };
		ID3D11Buffer* m_pIndexBuffer{ nullptr };
		ID3D11InputLayout* m_pInputLayout{ nullptr };

		//Matricces
		Matrix m_TranslationMatrix{};
		Matrix m_RotationMatrix{};
		Matrix m_WorldMatrix;

		//Topology
		PrimitiveTopology m_Topology{ PrimitiveTopology::TriangleList };

		//Data variables
		uint32_t m_NumIndices{};
		std::vector<VertexExt> m_Vertices{};
		std::vector<VertexOut> m_VerticesOut{};
		std::vector<uint32_t> m_Indices{};

		bool m_ShouldRender{ true };
	};
}

