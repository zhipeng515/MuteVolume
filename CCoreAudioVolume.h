#pragma once

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT	0x0600
#endif

#include <mmdeviceapi.h>
#include <Endpointvolume.h>
#include <Audioclient.h>
#include <KsMedia.h>
#include <Audiopolicy.h>
#include <vector>

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT	0x0501
#endif

class CCoreAudioVolume
{
public:
	static HRESULT EnableSound(BOOL bEnable);

	static HRESULT Initlialize(BOOL bEnableSound);

	static HRESULT Uninitialize();

	static BOOL IsEnableSound() { return m_bEnableSound; }

private:
	static bool GetDeviceDsc(IMMDevice *pDevice, wchar_t* DeviceDsc, size_t nDeviceScsSize);

	static bool VerifyDev(IMMDevice *pDevice, EDataFlow dataFlow);

	static int GetDevicePlayVol(void);

	static bool SetMute(BOOL bMute);

private:
	static BOOL m_bEnableSound;
	static IMMDeviceEnumerator*		m_pEnumerator;
	static IMMDeviceCollection*		m_pCollection;
	//	static IAudioEndpointVolume*	pVolumeAPI=NULL;  
	static UINT						m_nDevCount;
	static std::vector<IMMDevice*>	m_vArrayDevice;
};
