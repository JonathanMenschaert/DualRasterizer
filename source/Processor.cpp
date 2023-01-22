#include "pch.h"
#include "Processor.h"

namespace dae
{
	const ColorRGB Processor::m_UniformColor{ 0.1f, 0.1f, 0.1f };

	Processor::Processor(SDL_Window* pWindow)
		:m_pWindow{pWindow}
	{
		//Initialize window
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	}
}
