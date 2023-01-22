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
		//Assign the value of the directx primitive topologies to the local topologies
		//This way a single variable can be used for both the hardware and the software rasterizer
		TriangleList = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		TriangleStrip = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	};

	enum class ShadingMode
	{
		Combined,
		ObservedArea,
		Diffuse,
		Specular,
		//Define modes above
		COUNT
	};

	enum class CullMode
	{
		Back,
		Front,
		None,

		//Define mdoes above
		COUNT
	};

	enum class SamplerState
	{
		Point,
		Linear,
		Anisotropic,
		//Define samplestates above

		COUNT
	};

	enum class RenderMode
	{
		FinalColor,
		DepthBuffer,

		//Declare modes above
		COUNT
	};
}

