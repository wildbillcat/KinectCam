#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include "NuiApi.h"

class KinectCam
{
public:
    HRESULT Nui_Init();
    void Nui_UnInit();
    void Nui_GetCamFrame(BYTE *frameBuffer, int frameSize);

private:
	HANDLE m_hNextVideoFrameEvent;
    HANDLE m_pVideoStreamHandle;
};
