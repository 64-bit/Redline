#pragma once
#include "Utilities/Multithreading/IThreadpoolJob.h"

namespace Redline
{
	class FrameRenderer;

	class RenderPendingQueueJob : public IThreadpoolJob
	{
	public:
		void Execute() override;
		FrameRenderer* Renderer;
	};
}
