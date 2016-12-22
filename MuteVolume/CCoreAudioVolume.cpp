#include "StdAfx.h"
#include ".\CCoreAudioVolume.h"

#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
				{ (punk)->Release(); (punk) = NULL; }

BOOL					CCoreAudioVolume::m_bEnableSound = FALSE;
IMMDeviceEnumerator*	CCoreAudioVolume::m_pEnumerator = NULL;
IMMDeviceCollection*	CCoreAudioVolume::m_pCollection = NULL;
UINT					CCoreAudioVolume::m_nDevCount = 0;
std::vector<IMMDevice*>	CCoreAudioVolume::m_vArrayDevice;


//得到设备硬件ID (设备管理器可以看到的硬件ID)
bool CCoreAudioVolume::GetDeviceDsc(IMMDevice *pDevice, wchar_t* DeviceDsc, size_t nDeviceScsSize)
{
	HRESULT hr;
	IPropertyStore *pStore;
	hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
	if (SUCCEEDED(hr))
	{
		PROPERTYKEY Drvidkey = { 0xb3f8fa53, 0x0004, 0x438e, 0x90, 0x03, 0x51, 0xa4, 0x6e, 0x13, 0x9b, 0xfc, 2 };
		PROPVARIANT pDrvidkey;
		PropVariantInit(&pDrvidkey);
		hr = pStore->GetValue(Drvidkey, &pDrvidkey);
		if (SUCCEEDED(hr))
		{
			wcscpy_s(DeviceDsc, nDeviceScsSize, pDrvidkey.pwszVal);
			PropVariantClear(&pDrvidkey);
			pStore->Release();
			return true;
		}
		pStore->Release();
	}
	return false;
}

// 验证设备是否指定设备
bool CCoreAudioVolume::VerifyDev(IMMDevice *pDevice, EDataFlow dataFlow)
{
	wchar_t DeviceDsc[255];
	if (GetDeviceDsc(pDevice, DeviceDsc, 255))
	{
		// 这里省略判断具体设备的 匹配硬件　如 HDAUDIO\FUNC_01&VEN_10EC&DEV_0888&SUBSYS_14627514&REV_1000
		return true;
	}
	return false;
}

// 获取设备音量
int CCoreAudioVolume::GetDevicePlayVol(void)
{
	IMMDeviceEnumerator* pEnumerator;
	IMMDeviceCollection* pCollection = NULL;
	IMMDevice *pDevice = NULL;
	IAudioEndpointVolume *pVolumeAPI = NULL;
	UINT deviceCount = 0;
	HRESULT hr;
	float fVolume = -1;

//	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	//实例化 MMDeviceEnumerator 枚举器
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
	if (hr != S_OK)
	{
		goto FreeEnumerator;
	}
	// 枚举 设备到设备容器 eRander：放音设备，DEVICE_STATE_ACTIVE 为当前已激活的设备，禁用和无连接的用其他状态参数
	hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
	if (hr != S_OK)
	{
		goto FreeCollection;
	}
	// 设备容器里的总数
	hr = pCollection->GetCount(&deviceCount);
	if (hr != S_OK)
	{
		goto FreeCollection;
	}

	for (UINT dev = 0; dev<deviceCount; dev++)
	{
		pDevice = NULL;
		hr = pCollection->Item(dev, &pDevice);
		if (hr == S_OK)
		{
			if (VerifyDev(pDevice, eRender))
			{    // 用 pDevice 的 Activate 方法初始一个 IAudioEndpointVolume 接口
				hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void **)(&pVolumeAPI));
				// 使用 IAudioEndpintVolume 的方法获取音量，设置音量，设置静音等 
				hr = pVolumeAPI->GetMasterVolumeLevelScalar(&fVolume);
				break;
			}
		}
	}

FreeCollection:
	SAFE_RELEASE(pCollection);
FreeEnumerator:
	SAFE_RELEASE(pEnumerator);
//	CoUninitialize();
	if (fVolume > 0)
		return (int)(fVolume * 100);
	else
		return (int)fVolume;

	return 0;
}

//设置应用程序静音
bool CCoreAudioVolume::SetMute(BOOL bMute)
{
	//IAudioSessionControl *pAudioSection = NULL;
	ISimpleAudioVolume	 *pAudioVolume = NULL;
	IAudioSessionManager *pManager = NULL;
	IMMDevice			 *pDevice = NULL;
	HRESULT hr;

	std::vector<IMMDevice*>::iterator iter = m_vArrayDevice.begin();
	for (; iter != m_vArrayDevice.end(); iter++)
	{
		pDevice = *iter;
		ATLASSERT(pDevice != NULL);

		hr = pDevice->Activate(__uuidof(IAudioSessionManager),
			CLSCTX_INPROC_SERVER, NULL,
			(void**)&pManager);

		if (FAILED(hr)) continue;

		//hr = pManager->GetAudioSessionControl(NULL, 0, &pAudioSection);

		hr = pManager->GetSimpleAudioVolume(NULL, 0, &pAudioVolume);

		if (SUCCEEDED(hr))
		{
			pAudioVolume->SetMute(bMute, &GUID_NULL);
		}
		else
		{
			ATLASSERT(FALSE);
		}
	}

	SAFE_RELEASE(pManager);
	SAFE_RELEASE(pAudioVolume);

	return true;
}

HRESULT CCoreAudioVolume::EnableSound(BOOL bEnable)
{
	m_bEnableSound = bEnable;

	bool bResult = CCoreAudioVolume::SetMute(!bEnable);

	return bResult ? S_OK : E_FAIL;
}

HRESULT CCoreAudioVolume::Initlialize(BOOL bEnableSound)
{
	m_bEnableSound = bEnableSound;

//	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	HRESULT hr;
	//实例化 MMDeviceEnumerator 枚举器
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	// 枚举 设备到设备容器 eRander：放音设备，DEVICE_STATE_ACTIVE 为当前已激活的设备，禁用和无连接的用其他状态参数
	hr = m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &m_pCollection);
	if (hr != S_OK)
	{
		return E_FAIL;
	}

	// 设备容器里的总数
	hr = m_pCollection->GetCount(&m_nDevCount);
	if (hr != S_OK)
	{
		return E_FAIL;
	}

	IMMDevice* pDevice = NULL;

	for (UINT dev = 0; dev<m_nDevCount; dev++)
	{
		pDevice = NULL;
		hr = m_pCollection->Item(dev, &pDevice);
		if (hr == S_OK)
		{
			if (VerifyDev(pDevice, eRender))
			{
				m_vArrayDevice.push_back(pDevice);
			}
		}
	}

	return S_OK;
}

HRESULT CCoreAudioVolume::Uninitialize()
{
	std::vector<IMMDevice*>::iterator iter = m_vArrayDevice.begin();
	for (; iter != m_vArrayDevice.end(); iter++)
	{
		SAFE_RELEASE(*iter);
	}
	SAFE_RELEASE(m_pCollection);
	SAFE_RELEASE(m_pEnumerator);

//	CoUninitialize();
	m_vArrayDevice.clear();

	return S_OK;
}