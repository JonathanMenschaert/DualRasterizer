#include "pch.h"
#include "Processor.h"

namespace dae
{
	dae::Processor::Processor(SDL_Window* pWindow)
		:m_pWindow{pWindow}
	{
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	}
}
