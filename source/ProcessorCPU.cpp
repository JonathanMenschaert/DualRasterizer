#include "pch.h"
#include "ProcessorCPU.h"
#include "Utils.h"
#include "Camera.h"


//Multithreading includes
#include <thread>
#include <ppl.h>

namespace dae
{
	ProcessorCPU::ProcessorCPU(SDL_Window* pWindow)
		:Processor{ pWindow }
	{
		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		const int nrPixels{ m_Width * m_Height };
		m_pDepthBufferPixels = new float[nrPixels];
		std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);
	}

	ProcessorCPU::~ProcessorCPU()
	{
		delete[] m_pDepthBufferPixels;
		m_pDepthBufferPixels = nullptr;
	}

	void ProcessorCPU::Render(std::vector<Mesh*>& meshes, const Camera* camera)
	{
		//Lock Backbuffer
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
		const int nrPixels{ m_Width * m_Height };
		std::fill_n(m_pDepthBufferPixels, nrPixels, 1.f);
		SDL_LockSurface(m_pBackBuffer);

		RenderMesh(meshes, camera);

		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void ProcessorCPU::VertexTransformationFunction(Mesh* pMesh, const Camera* camera) const
	{
		pMesh->GetVerticesOut().clear();
		pMesh->GetVerticesOut().reserve(pMesh->GetVertices().size());
		const Matrix worldViewProjectionMatrix{ pMesh->GetWorldMatrix() * camera->viewMatrix * camera->projectionMatrix };
		for (const auto& vertexIn : pMesh->GetVertices())
		{
			VertexOut vertexOut{ Vector4{ vertexIn.position, 1.f}, vertexIn.uv, vertexIn.normal, 
				vertexIn.tangent, vertexIn.viewDirection };
			vertexOut.position = worldViewProjectionMatrix.TransformPoint(vertexOut.position);
			const float perspectiveDiv{ 1.f / vertexOut.position.w };
			vertexOut.position.x *= perspectiveDiv;
			vertexOut.position.y *= perspectiveDiv;
			vertexOut.position.z *= perspectiveDiv;
			vertexOut.normal = pMesh->GetWorldMatrix().TransformVector(vertexIn.normal);
			vertexOut.tangent = pMesh->GetWorldMatrix().TransformVector(vertexIn.tangent);
			vertexOut.viewDirection = (pMesh->GetWorldMatrix().TransformPoint(vertexIn.position) - camera->origin);
			pMesh->GetVerticesOut().emplace_back(vertexOut);
		}
	}

	void ProcessorCPU::RenderMesh(std::vector<Mesh*>& meshes, const Camera* camera)
	{
		for (Mesh* pMesh : meshes)
		{
			if (!pMesh->ShouldRender()) continue;
			//Check this later
			VertexTransformationFunction(pMesh, camera);
			std::vector<Vector2> screenVertices;
			screenVertices.reserve(pMesh->GetVerticesOut().size());
			for (const auto& vertexNdc : pMesh->GetVerticesOut())
			{
				screenVertices.emplace_back(
					Vector2{
						(vertexNdc.position.x + 1) * 0.5f * m_Width,
						(1 - vertexNdc.position.y) * 0.5f * m_Height
					}
				);
			}

			switch (pMesh->GetPrimitiveTopology())
			{
			case PrimitiveTopology::TriangleStrip:
			{
				const uint32_t numIndices{ static_cast<uint32_t>(pMesh->GetIndices().size() - 2)};
				concurrency::parallel_for(0u, numIndices, [=, this](uint32_t vertIdx)
					{
						RenderMeshTriangle(pMesh, screenVertices, vertIdx, vertIdx & 1);
					}
				);
				break;
			}
			case PrimitiveTopology::TriangleList:
				const uint32_t numTriangles{ static_cast<uint32_t>(pMesh->GetIndices().size() - 2) / 3 };
				concurrency::parallel_for(0u, numTriangles, [=, this](uint32_t vertIdx)
					{
						RenderMeshTriangle(pMesh, screenVertices, vertIdx * 3);
					}
				);
				break;
			}
		}
	}

	void ProcessorCPU::RenderMeshTriangle(Mesh* pMesh, const std::vector<Vector2>& screenVertices, uint32_t vertIdx, bool swapVertices)
	{
		//Get vertex from the index vector.
		//The vertices will be swapped when vertIdx is uneven and the TriangleStrip primitive topology is set
		const uint32_t vertIdx0{ pMesh->GetIndices()[vertIdx + swapVertices * 2] };
		const uint32_t vertIdx1{ pMesh->GetIndices()[vertIdx + 1] };
		const uint32_t vertIdx2{ pMesh->GetIndices()[vertIdx + !swapVertices * 2] };

		//Check If the same vertex is retrieved twice. This is used for the TriangleStrip topology.
		if (vertIdx0 == vertIdx1 || vertIdx1 == vertIdx2 || vertIdx2 == vertIdx0) return;

		//Check if all of the retrieved vertices are within the frustrum.
		if (!GeometryUtils::IsVertexInFrustrum(pMesh->GetVerticesOut()[vertIdx0].position)
			|| !GeometryUtils::IsVertexInFrustrum(pMesh->GetVerticesOut()[vertIdx1].position)
			|| !GeometryUtils::IsVertexInFrustrum(pMesh->GetVerticesOut()[vertIdx2].position))
		{
			return;
		}

		CullMode meshCullMode{ pMesh->GetCullMode() };

		//Create boundingbox around triangle
		Vector2 boundingBoxMin{ Vector2::Min(screenVertices[vertIdx0], Vector2::Min(screenVertices[vertIdx1], screenVertices[vertIdx2])) };
		Vector2 boundingBoxMax{ Vector2::Max(screenVertices[vertIdx0], Vector2::Max(screenVertices[vertIdx1], screenVertices[vertIdx2])) };

		//Clip boundingbox to frustrum
		const Vector2 screenVector{ static_cast<float>(m_Width), static_cast<float>(m_Height) };
		boundingBoxMin = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMin, screenVector));
		boundingBoxMax = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMax, screenVector));

		//Loop over the pixels in the area defined by the boundingbox
		for (int px{ static_cast<int>(boundingBoxMin.x) }; px < boundingBoxMax.x; ++px)
		{
			for (int py{ static_cast<int>(boundingBoxMin.y) }; py < boundingBoxMax.y; ++py)
			{
				//Check if pixel is in triangle
				const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
				float signedAreaV0V1, signedAreaV1V2, signedAreaV2V0;

				if (GeometryUtils::IsPointInTriangle(screenVertices[vertIdx0], screenVertices[vertIdx1],
					screenVertices[vertIdx2], pixelCoordinates, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0))
				{
					const bool isAreaV0V1Pos{ signedAreaV0V1 >= 0.f };
					const bool isAreaV1V2Pos{ signedAreaV1V2 >= 0.f };
					const bool isAreaV2V0Pos{ signedAreaV2V0 >= 0.f };

					

					//Calculate interpolated depth
					const float triangleArea{ 1.f / (Vector2::Cross(screenVertices[vertIdx1] - screenVertices[vertIdx0],
						screenVertices[vertIdx2] - screenVertices[vertIdx0])) };

					const float weightV0{ signedAreaV1V2 * triangleArea };
					const float weightV1{ signedAreaV2V0 * triangleArea };
					const float weightV2{ signedAreaV0V1 * triangleArea };

					const float depthInterpolated
					{
						1.f / (1.f / pMesh->GetVerticesOut()[vertIdx0].position.z * weightV0 +
						1.f / pMesh->GetVerticesOut()[vertIdx1].position.z * weightV1 +
						1.f / pMesh->GetVerticesOut()[vertIdx2].position.z * weightV2)
					};

					//Compare calculated depth to the depth already stored in the depthbuffer. 
					//The depthtest is passed when the calculated depth is smaller.
					const int pixelIdx{ px + py * m_Width };
					if (m_pDepthBufferPixels[pixelIdx] <= depthInterpolated || depthInterpolated < 0.f || depthInterpolated > 1.f) continue;
					m_pDepthBufferPixels[pixelIdx] = depthInterpolated;


					ColorRGB finalColor{};
					switch (m_RenderMode)
					{
					default:
					case RenderMode::FinalColor:
					{

						const float inv0PosW{ 1.f / pMesh->GetVerticesOut()[vertIdx0].position.w };
						const float inv1PosW{ 1.f / pMesh->GetVerticesOut()[vertIdx1].position.w };
						const float inv2PosW{ 1.f / pMesh->GetVerticesOut()[vertIdx2].position.w };

						const float viewDepthInterpolated
						{
							1.f / (1.f * inv0PosW * weightV0 +
							1.f * inv1PosW * weightV1 +
							1.f * inv2PosW * weightV2)
						};

						const Vector2 pixelUV
						{
							(pMesh->GetVerticesOut()[vertIdx0].uv * inv0PosW * weightV0 +
							pMesh->GetVerticesOut()[vertIdx1].uv * inv1PosW * weightV1 +
							pMesh->GetVerticesOut()[vertIdx2].uv * inv2PosW * weightV2) * viewDepthInterpolated
						};

						const Vector3 normal
						{
							(pMesh->GetVerticesOut()[vertIdx0].normal * inv0PosW * weightV0 +
							pMesh->GetVerticesOut()[vertIdx1].normal * inv1PosW * weightV1 +
							pMesh->GetVerticesOut()[vertIdx2].normal * inv2PosW * weightV2) * viewDepthInterpolated
						};

						const Vector3 tangent
						{
							(pMesh->GetVerticesOut()[vertIdx0].tangent * inv0PosW * weightV0 +
							pMesh->GetVerticesOut()[vertIdx1].tangent * inv1PosW * weightV1 +
							pMesh->GetVerticesOut()[vertIdx2].tangent * inv2PosW * weightV2) * viewDepthInterpolated
						};

						const Vector3 viewDirection
						{
							(pMesh->GetVerticesOut()[vertIdx0].viewDirection * inv0PosW * weightV0 +
							pMesh->GetVerticesOut()[vertIdx1].viewDirection * inv1PosW * weightV1 +
							pMesh->GetVerticesOut()[vertIdx2].viewDirection * inv2PosW * weightV2) * viewDepthInterpolated
						};

						VertexExt interpolatedVertex{};
						interpolatedVertex.uv = pixelUV;
						interpolatedVertex.normal = normal.Normalized();
						interpolatedVertex.tangent = tangent.Normalized();
						interpolatedVertex.viewDirection = viewDirection.Normalized();
						finalColor = ColorRGB{};//PixelShading(interpolatedVertex);
					}
					break;
					case RenderMode::DepthBuffer:
					{
						const float depthRemapped{ DepthRemap(depthInterpolated, 0.997f, 1.f) };
						finalColor = ColorRGB{ depthRemapped, depthRemapped, depthRemapped };
					}
					}


					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}

	/*ColorRGB dae::Renderer::PixelShading(const Vertex_Out& v)
	{
		const float lightIntensity{ 7.f };
		const float kd{ 1.f };
		const float shininess{ 25.f };
		Vector3 sampledNormal{ v.normal };
		const ColorRGB ambient{ 0.025f, 0.025f, 0.025f };

		if (m_ShouldRenderNormals)
		{
			const Vector3 binormal{ Vector3::Cross(v.normal, v.tangent) };
			const Matrix tangentSpaceAxis{ v.tangent, binormal.Normalized(), v.normal, Vector3{0.f, 0.f, 0.f} };
			sampledNormal = m_pNormalTexture->SampleNormal(v.uv);
			sampledNormal = (2.f * sampledNormal) - Vector3{ 1.f, 1.f, 1.f };
			sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);
			sampledNormal.Normalize();
		}

		const float observedArea{ std::max(Vector3::Dot(sampledNormal, -m_LightDirection), 0.f) };
		const ColorRGB observedAreaColor{ observedArea, observedArea, observedArea };
		switch (m_ShadingMode)
		{
		case ShadingMode::Combined:
		{
			const ColorRGB diffuse{ dae::BRDF::Lambert(kd, m_pDiffuseTexture->Sample(v.uv)) * lightIntensity };
			const ColorRGB specular{ BRDF::Phong(m_pSpecularTexture->Sample(v.uv), 1.f, m_pGlossinessTexture->Sample(v.uv).r * shininess,
				m_LightDirection, -v.viewDirection, sampledNormal) };
			return (diffuse + specular + ambient) * observedArea;
		}
		case ShadingMode::ObservedArea:
		{
			return observedAreaColor;
		}
		case ShadingMode::Diffuse:
		{
			const ColorRGB diffuse{ BRDF::Lambert(kd, m_pDiffuseTexture->Sample(v.uv) * lightIntensity) };
			return diffuse * observedAreaColor;
		}
		case ShadingMode::Specular:
		{
			const ColorRGB specular{ BRDF::Phong(m_pSpecularTexture->Sample(v.uv), 1.f, m_pGlossinessTexture->Sample(v.uv).r * shininess,
				m_LightDirection, -v.viewDirection, sampledNormal) };
			return specular * observedAreaColor;
		}
		default:
			return ColorRGB{};
		}
	}*/
}