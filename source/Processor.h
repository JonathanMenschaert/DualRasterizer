#pragma once
#include "Mesh.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	struct Camera;
	class Processor
	{
	public:
		Processor(SDL_Window* pWindow);
		virtual ~Processor() = default;

		Processor(const Processor& processor) = delete;
		Processor(Processor&& processor) noexcept = delete;
		Processor& operator=(const Processor& processor) = delete;
		Processor& operator=(Processor&& processor) noexcept = delete;

		virtual void Render(std::vector<Mesh*>& meshes, const Camera* camera) = 0;

	protected:
		SDL_Window* m_pWindow{nullptr};
		int m_Width{};
		int m_Height{};
	};
}

