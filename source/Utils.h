#pragma once
#include <fstream>
#include "Math.h"
#include <vector>
#include "Mesh.h"

namespace dae
{
	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			vertices.clear();
			indices.clear();

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// Vertex TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// Vertex Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 vertices, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					Vertex vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.uv = UVs[iTexCoord - 1];
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = uint32_t(vertices.size()) - 1;
						//indices.push_back(uint32_t(vertices.size()) - 1);
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding)
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[size_t(i) + 1];
				uint32_t index2 = indices[size_t(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].uv;
				const Vector2& uv1 = vertices[index1].uv;
				const Vector2& uv2 = vertices[index2].uv;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Create the Tangents (reject)
			for (auto& v : vertices)
			{
				v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

				if (flipAxisAndWinding)
				{
					v.position.z *= -1.f;
					v.normal.z *= -1.f;
					v.tangent.z *= -1.f;
				}

			}

			return true;
		}
#pragma warning(pop)
	}

	namespace GeometryUtils
	{
		inline bool IsPointInTriangle(const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& pixel,
			float& signedAreaOutV0V1, float& signedAreaOutV1V2, float& signedAreaOutV2V0)
		{	
			//Calculate signed areas of triangle
			signedAreaOutV0V1 = Vector2::Cross(v1 - v0, pixel - v0);
			bool areaV0V1Pos{ signedAreaOutV0V1 >= 0.f };			

			signedAreaOutV1V2 = Vector2::Cross(v2 - v1, pixel - v1);
			bool areaV1V2Pos{ signedAreaOutV1V2 >= 0.f };

			signedAreaOutV2V0 = Vector2::Cross(v0 - v2, pixel - v2);
			bool areaV2V0Pos{ signedAreaOutV2V0 >= 0.f };

			//If all areas have the same sign, they are inside the triangle
			bool haveAreasSameSign{ areaV0V1Pos == areaV1V2Pos && areaV0V1Pos == areaV2V0Pos };

			return haveAreasSameSign;
		}

		inline bool IsVertexInFrustrum(const Vector4& vertex, float min = -1.f, float max = 1.f)
		{
			//Check if the vertex is iside the appropriate boundaries
			return vertex.x >= min && vertex.x <= max && vertex.y >= min && vertex.y <= max && vertex.z >= 0.f && vertex.z <= max;
		}

	}

	namespace BRDF
	{
		inline ColorRGB ObservedArea(const Vector3& normal, const Vector3& lightDir)
		{
			const float observedArea{ std::max(Vector3::Dot(normal, lightDir), 0.f)};
			return observedArea * ColorRGB{ 1, 1, 1 };
		}

		inline ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			return (cd * kd) / PI;
		}

		inline ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return (cd * kd) / PI;
		}

		inline ColorRGB Phong(const ColorRGB specularColor, float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{

			const Vector3 reflect{ Vector3::Reflect(l, n) };
			//Used std::max to prevent the result of the dotproduct going under 0
			const float cosa = std::max(Vector3::Dot(reflect, v), 0.f);
			const float specularReflection{ ks * powf(cosa, exp) };
			return specularColor * specularReflection;
		}
	}
}