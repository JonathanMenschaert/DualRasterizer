#pragma once
#include "Mesh.h"

namespace dae
{
	class Processor
	{
	public:
		Processor() = default;
		virtual ~Processor() = default;

		Processor(const Processor& processor) = delete;
		Processor(Processor&& processor) noexcept = delete;
		Processor& operator=(const Processor& processor) = delete;
		Processor& operator=(Processor&& processor) noexcept = delete;

		void Render(Mesh& mesh);

	private:

	};
}

