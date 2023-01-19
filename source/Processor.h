#pragma once
#include "Mesh.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Processor
	{
	public:
		Processor(SDL_Window* pWindow);
		virtual ~Processor() = default;

		Processor(const Processor& processor) = delete;
		Processor(Processor&& processor) noexcept = delete;
		Processor& operator=(const Processor& processor) = delete;
		Processor& operator=(Processor&& processor) noexcept = delete;

		virtual void Render(Mesh& mesh) const = 0;

	protected:
		SDL_Window* m_pWindow{nullptr};
		int m_Width{};
		int m_Height{};
	};
}

