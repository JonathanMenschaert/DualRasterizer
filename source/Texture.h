#pragma once

namespace dae
{
	class Texture
	{
	public:
		Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);
		~Texture();
		Texture(const Texture&) = delete;
		Texture(Texture&&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

		ID3D11ShaderResourceView* GetSRV() const;

		ColorRGB Sample(const Vector2& uv) const;
		Vector4 SampleTransparency(const Vector2& uv) const;
		Vector3 SampleNormal(const Vector2& uv) const;

		static Texture* LoadFromFile(ID3D11Device* pDevice, const std::string& path);

	private:
		ID3D11Texture2D* m_pResource{ nullptr };
		ID3D11ShaderResourceView* m_pSRV{ nullptr };

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
		const float m_ColorModifier{ 1.f / 255.f };
	};
}
