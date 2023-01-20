#pragma once
#include "pch.h"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
	};

	struct VertexExt final : public Vertex
	{
		Vector3 viewDirection{};
	};

	struct VertexOut final
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};


	enum class PrimitiveTopology
	{
		TriangleList = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		TriangleStrip = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	};
}

