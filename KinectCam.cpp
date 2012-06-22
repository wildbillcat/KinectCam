#include "KinectCam.h"
#include "stdio.h"
#include <shlobj.h>

bool g_flipImage = false;

HRESULT KinectCam::Nui_Init()
{
    HRESULT hr;

	m_hNextVideoFrameEvent = NULL;
    m_pVideoStreamHandle = NULL;
    m_hNextVideoFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
    if( FAILED( hr ) )
    {
        return hr;
    }

	// Set camera angle
	char iniFile[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, iniFile);
	strcat(iniFile, "\\KinectCam");
	CreateDirectory(iniFile, NULL);
	strcat(iniFile, "\\config.ini");
	if (_access(iniFile, 0) != 0)
	{
		FILE *f = fopen(iniFile, "w");
		if (f)
		{
			fprintf(f, "[KinectCam]\n");
			fprintf(f, "CameraElevationAngle=0\n");
			fprintf(f, "CameraShutdownAngle=0\n");
			fprintf(f, "FlipImage=0\n");
			fclose(f);
		}
	}

	char val[256];
	GetPrivateProfileString("KinectCam", "CameraElevationAngle", "999", val, 256, iniFile);
	int angle = atoi(val);
	if (angle != 999)
		NuiCameraElevationSetAngle(angle);
	
	GetPrivateProfileString("KinectCam", "FlipImage", "0", val, 256, iniFile);
	g_flipImage = (atoi(val) != 0);

	NUI_IMAGE_RESOLUTION resolution;
	resolution = NUI_IMAGE_RESOLUTION_640x480;

    hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
		resolution,
        0,
        2,
        m_hNextVideoFrameEvent,
        &m_pVideoStreamHandle );

	return hr;
}



void KinectCam::Nui_UnInit()
{
	// Reset camera angle
	char iniFile[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, iniFile);
	strcat(iniFile, "\\KinectCam");
	strcat(iniFile, "\\config.ini");

	char val[256];
	GetPrivateProfileString("KinectCam", "CameraShutdownAngle", "999", val, 256, iniFile);
	int angle = atoi(val);
	if (angle != 999)
		NuiCameraElevationSetAngle(angle);

	NuiShutdown( );

	if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextVideoFrameEvent );
        m_hNextVideoFrameEvent = NULL;
    }
}


void KinectCam::Nui_GetCamFrame(BYTE *frameBuffer, int frameSize)
{
    const NUI_IMAGE_FRAME *pImageFrame = NULL;

	WaitForSingleObject(m_hNextVideoFrameEvent, INFINITE);

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pVideoStreamHandle,
        0,
        &pImageFrame );
    if( FAILED( hr ) )
    {
        return;
    }

    INuiFrameTexture *pTexture = pImageFrame->pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 )
    {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;
		memcpy(frameBuffer, pBuffer, frameSize);
    }

    NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}
