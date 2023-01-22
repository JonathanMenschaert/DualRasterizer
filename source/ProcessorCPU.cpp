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
		std::fill_n(m_pDepthBufferPixels, nrPixels, 1.f);

		m_BackgroundColor = m_SoftwareColor * 255.f; //Multiply color to fit the FillRect function
	}

	ProcessorCPU::~ProcessorCPU()
	{
		//Free resources
		delete[] m_pDepthBufferPixels;
		m_pDepthBufferPixels = nullptr;
	}

	void ProcessorCPU::Render(std::vector<Mesh*>& meshes, const Camera* camera)
	{
		//Lock Backbuffer
		SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 
			static_cast<Uint8>(m_BackgroundColor.r),
			static_cast<Uint8>(m_BackgroundColor.g),
			static_cast<Uint8>(m_BackgroundColor.b)
		));
		const int nrPixels{ m_Width * m_Height };
		std::fill_n(m_pDepthBufferPixels, nrPixels, 1.f);
		SDL_LockSurface(m_pBackBuffer);

		//Projection Stage
		ProjectMesh(meshes, camera);

		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void ProcessorCPU::ToggleBackgroundColor(bool useUniformBg)
	{
		m_BackgroundColor = (useUniformBg ? m_UniformColor : m_SoftwareColor) * 255.f; //Multiply color to fit the FillRect function
	}

	void dae::ProcessorCPU::ToggleNormalMap()
	{
		m_ShouldRenderNormals = !m_ShouldRenderNormals;
		std::wcout << "\033[35m" << "**(SOFTWARE) NormalMap " << (m_ShouldRenderNormals ? "ON" : "OFF") << "\033[0m" << "\n";
	}

	void ProcessorCPU::ToggleBoundingBoxes()
	{
		m_ShouldRenderBoundingBoxes = !m_ShouldRenderBoundingBoxes;
		std::wcout << "\033[35m" << "**(SOFTWARE) BoundingBox Visualization " << (m_ShouldRenderBoundingBoxes ? "ON" : "OFF") << "\033[0m" << "\n";
	}

	void ProcessorCPU::CycleRenderMode()
	{
		//Cycle through the rendermodes
		int count{ static_cast<int>(RenderMode::COUNT) };
		int currentMode{ static_cast<int>(m_RenderMode) };
		m_RenderMode = static_cast<RenderMode>((currentMode + 1) % count);

		std::wcout << "\033[35m" << "**(SOFTWARE) DepthBuffer Visualization " 
			<< (m_RenderMode == RenderMode::FinalColor ? "ON" : "OFF") << "\033[0m" << "\n";
	}

	void ProcessorCPU::CycleShadingMode()
	{
		//Cycle through the shading modes
		int count{ static_cast<int>(ShadingMode::COUNT) };
		int currentMode{ static_cast<int>(m_ShadingMode) };
		m_ShadingMode = static_cast<ShadingMode>((currentMode + 1) % count);

		switch (m_ShadingMode)
		{
		case ShadingMode::Combined:
			std::wcout << "\033[35m" << "**(SOFTWARE) Shading Mode = COMBINED" << "\033[0m" << "\n";
			break;
		case ShadingMode::ObservedArea:
			std::wcout << "\033[35m" << "**(SOFTWARE) Shading Mode = OBSERVED_AREA" << "\033[0m" << "\n";
			break;
		case ShadingMode::Diffuse:
			std::wcout << "\033[35m" << "**(SOFTWARE) Shading Mode = DIFFUSE" << "\033[0m" << "\n";
			break;
		case ShadingMode::Specular:
			std::wcout << "\033[35m" << "**(SOFTWARE) Shading Mode = SPECULAR" << "\033[0m" << "\n";
			break;
		default:
			break;
		}
	}


	void ProcessorCPU::VertexTransformationFunction(Mesh* pMesh, const Camera* camera) const
	{
		pMesh->GetVerticesOut().clear();
		pMesh->GetVerticesOut().reserve(pMesh->GetVertices().size());
		const Matrix worldViewProjectionMatrix{ pMesh->GetWorldMatrix() * camera->GetViewMatrix() * camera->GetProjectionMatrix()};
		
		for (const auto& vertexIn : pMesh->GetVertices())
		{
			VertexOut vertexOut{ Vector4{ vertexIn.position, 1.f}, vertexIn.uv, vertexIn.normal, 
				vertexIn.tangent, vertexIn.viewDirection };

			//Transform position based on worldviewproj matrix
			vertexOut.position = worldViewProjectionMatrix.TransformPoint(vertexOut.position);

			//Perspective Divide
			const float perspectiveDiv{ 1.f / vertexOut.position.w };
			vertexOut.position.x *= perspectiveDiv;
			vertexOut.position.y *= perspectiveDiv;
			vertexOut.position.z *= perspectiveDiv;

			//Transform properties based on the mesh worldmatrix
			vertexOut.normal = pMesh->GetWorldMatrix().TransformVector(vertexIn.normal);
			vertexOut.tangent = pMesh->GetWorldMatrix().TransformVector(vertexIn.tangent);
			vertexOut.viewDirection = (pMesh->GetWorldMatrix().TransformPoint(vertexIn.position) - camera->origin);

			//Add the vertex to the output
			pMesh->GetVerticesOut().emplace_back(vertexOut);
		}
	}

	inline bool ProcessorCPU::IsValidPixelForCullMode(CullMode mode, float areaV0V1, float areaV1V2, float areaV2V0) const
	{
		//Calculate sign of each area with true = >=0 and false = <0
		const bool isAreaV0V1Pos{ areaV0V1 >= 0.f };
		const bool isAreaV1V2Pos{ areaV1V2 >= 0.f };
		const bool isAreaV2V0Pos{ areaV2V0 >= 0.f };

		//Check if all area signs are either false or true
		const bool isPointInFront{ isAreaV0V1Pos && isAreaV1V2Pos && isAreaV2V0Pos };
		const bool isPointInBack{ !isAreaV0V1Pos && !isAreaV1V2Pos && !isAreaV2V0Pos };

		//Check the cullmode
		const bool isCullModeNoneValid{ (isPointInFront || isPointInBack)  && mode == CullMode::None };
		const bool isCullModeFrontValid{ isPointInBack && mode == CullMode::Front};
		const bool isCullModeBackValid{ isPointInFront && mode == CullMode::Back };


		return isCullModeNoneValid || isCullModeFrontValid || isCullModeBackValid;
	}

	void ProcessorCPU::ProjectMesh(std::vector<Mesh*>& meshes, const Camera* camera)
	{
		for (Mesh* pMesh : meshes)
		{
			//Used to not render the fireFX when turned off
			if (!pMesh->ShouldRender()) continue;
			
			//Transform the mesh vertices
			VertexTransformationFunction(pMesh, camera);

			//Create the screen
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

			//Check the mesh topology
			switch (pMesh->GetPrimitiveTopology())
			{
				case PrimitiveTopology::TriangleStrip:
				{
					//Check if the mesh wants to use multithreading
					//Cannot be used with transparent objects due to variable processing time
					if (pMesh->UseMultiThreading())
					{
						const uint32_t numIndices{ static_cast<uint32_t>(pMesh->GetIndices().size() - 2) };
						//Go over the indices one by one for the strip topology
						concurrency::parallel_for(0u, numIndices, [=, this](uint32_t vertIdx)
						{
							RasterizeTriangle(pMesh, screenVertices, vertIdx, vertIdx & 1);
						});
					}
					else
					{
						//Go over the indices one by one for the strip topology
						for (uint32_t vertIdx{}; vertIdx < pMesh->GetIndices().size() - 2; ++vertIdx)
						{
							RasterizeTriangle(pMesh, screenVertices, vertIdx, vertIdx % 2);
						}
					}
					break;
				}
				case PrimitiveTopology::TriangleList:
				{
					//Check if the mesh wants to use multithreading
					//Cannot be used with transparent objects due to variable processing time
					if (pMesh->UseMultiThreading())
					{
						//Go over the indices in steps of 3 for the list topology
						const uint32_t numTriangles{ static_cast<uint32_t>(pMesh->GetIndices().size() - 2) / 3 };
						concurrency::parallel_for(0u, numTriangles, [=, this](uint32_t vertIdx)
						{
								RasterizeTriangle(pMesh, screenVertices, vertIdx * 3);
						});
					}
					else
					{
						//Go over the indices in steps of 3 for the list topology
						for (uint32_t vertIdx{}; vertIdx < pMesh->GetIndices().size() - 2; vertIdx += 3)
						{
							RasterizeTriangle(pMesh, screenVertices, vertIdx);
						}
					}
					break;
				}
			
			}
		}
	}

	void ProcessorCPU::RasterizeTriangle(Mesh* pMesh, const std::vector<Vector2>& screenVertices, uint32_t vertIdx, bool swapVertices)
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

		//Cache cull mode
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
				//Check if the bounding box should be rendered. 
				//If so, skip the rest of code in the loop and continue to the next
				if (m_ShouldRenderBoundingBoxes)
				{
					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255));
					continue;
				}

				//Check if pixel is in triangle
				const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
				float signedAreaV0V1, signedAreaV1V2, signedAreaV2V0;

				if (GeometryUtils::IsPointInTriangle(screenVertices[vertIdx0], screenVertices[vertIdx1],
					screenVertices[vertIdx2], pixelCoordinates, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0))
				{
					//Check if the pixel can be rendered in the current cullmode
					if (!IsValidPixelForCullMode(meshCullMode, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0)) continue;
					

					//Calculate interpolated depth
					const float triangleArea{ 1.f / (Vector2::Cross(screenVertices[vertIdx1] - screenVertices[vertIdx0],
						screenVertices[vertIdx2] - screenVertices[vertIdx0])) };

					const float weightV0{ signedAreaV1V2 * triangleArea };
					const float weightV1{ signedAreaV2V0 * triangleArea };
					const float weightV2{ signedAreaV0V1 * triangleArea };

					//Calculate z depth interpolated
					const float depthInterpolated
					{
						1.f / (weightV0 / pMesh->GetVerticesOut()[vertIdx0].position.z +
						weightV1 / pMesh->GetVerticesOut()[vertIdx1].position.z +
						weightV2 / pMesh->GetVerticesOut()[vertIdx2].position.z )
					};

					//Compare calculated depth to the depth already stored in the depthbuffer. 
					//The depthtest is passed when the calculated depth is smaller.
					const int pixelIdx{ px + py * m_Width };
					if (m_pDepthBufferPixels[pixelIdx] <= depthInterpolated || depthInterpolated < 0.f || depthInterpolated > 1.f) continue;
					if (pMesh->UseDepthBuffer()) m_pDepthBufferPixels[pixelIdx] = depthInterpolated;


					ColorRGB finalColor{};

					//Calculate final color based on the rendermode
					switch (m_RenderMode)
					{
					default:
					case RenderMode::FinalColor:
					{
						//Calculate w depth interpolated
						const float inv0PosW{ 1.f / pMesh->GetVerticesOut()[vertIdx0].position.w };
						const float inv1PosW{ 1.f / pMesh->GetVerticesOut()[vertIdx1].position.w };
						const float inv2PosW{ 1.f / pMesh->GetVerticesOut()[vertIdx2].position.w };

						const float viewDepthInterpolated
						{
							1.f / (inv0PosW * weightV0 +
							inv1PosW * weightV1 +
							inv2PosW * weightV2)
						};

						Vector2 pixelUV
						{
							(pMesh->GetVerticesOut()[vertIdx0].uv * inv0PosW * weightV0 +
							pMesh->GetVerticesOut()[vertIdx1].uv * inv1PosW * weightV1 +
							pMesh->GetVerticesOut()[vertIdx2].uv * inv2PosW * weightV2) * viewDepthInterpolated
						};
						//Clamping uv to mitigate rounding errors from the calculations
						pixelUV.x = std::min(1.f, std::max(pixelUV.x, 0.f));
						pixelUV.y = std::min(1.f, std::max(pixelUV.y, 0.f));

						//interpolate the various properties of the vertex
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

						VertexOut interpolatedVertex{};
						interpolatedVertex.uv = pixelUV;
						interpolatedVertex.normal = normal.Normalized();
						interpolatedVertex.tangent = tangent.Normalized();
						interpolatedVertex.viewDirection = viewDirection.Normalized();

						//Pixelshading stage is done in the effect stored in the mesh
						finalColor = pMesh->ShadePixel(interpolatedVertex, m_ShadingMode, m_pBackBufferPixels[px + (py * m_Width)], m_ShouldRenderNormals);
					}
					break;
					case RenderMode::DepthBuffer:
					{
						//Calculate the depthColor
						const float depthRemapped{ DepthRemap(depthInterpolated, 0.997f, 1.f) };
						const ColorRGB depthViewColor{ depthRemapped, depthRemapped, depthRemapped };
						const bool useDepthColor{ pMesh->UseDepthBuffer() };

						//Retrieve the background color
						uint8_t red{}, green{}, blue{};
						SDL_GetRGB(m_pBackBufferPixels[px + (py * m_Width)], m_pBackBuffer->format, &red, &green, &blue);
						const ColorRGB currentColor{ 
							static_cast<float>(red) * m_ColorModifier, 
							static_cast<float>(green) * m_ColorModifier,
							static_cast<float>(blue) * m_ColorModifier,
						};

						//Check if the depthcolor should be used instead of the background color
						finalColor = useDepthColor * depthViewColor  + !useDepthColor * currentColor;
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
}