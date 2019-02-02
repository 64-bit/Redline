#include "stdafx.h"
#include "RenderPendingQueueJob.h"
#include "FrameRenderer.h"

void Redline::RenderPendingQueueJob::Execute()
{
	Renderer->ProcessPendingTileJob();
}
