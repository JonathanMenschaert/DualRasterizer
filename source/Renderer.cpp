#include "pch.h"
#include "Renderer.h"
#include "Processor.h"
#include "ProcessorGPU.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height); //TODO check if this is necessary
		InitMeshes();
	}

	Renderer::~Renderer()
	{
		
	}

	void Renderer::Update(const Timer* pTimer)
	{

	}


	void Renderer::Render() const
	{
		//m_pRenderProcessor->Render(m_Meshes)

	}

	void Renderer::InitMeshes()
	{

	}
}
