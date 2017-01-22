#include "stdafx.h"

#include "MuteVolume.h"
#include "CCoreAudioVolume.h"

#include "../../CommonFunction/DetoursWrapper.h"
#include "../../CommonFunction/Utility.h"
#include "../../CommonFunction/StdLog.h"

#include <dsound.h>
#include <MMSystem.h>
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "Winmm.lib")

bool MuteVolume_XP::MUTE = FALSE;

DETOURS_FUNC_DECLARE(MMRESULT, WINAPI, waveOutWrite, HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
	if (MuteVolume_XP::MUTE)
		memset(pwh->lpData, 0, pwh->dwBufferLength);
	return  DETOURS_FUNC_CALLREAL(waveOutWrite, hwo, pwh, cbwh);
}


DETOURS_FUNC_DECLARE(MMRESULT, WINAPI, midiStreamOut, HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
{
	if (MuteVolume_XP::MUTE)
		memset(lpMidiHdr->lpData, 0, lpMidiHdr->dwBufferLength);
	return  DETOURS_FUNC_CALLREAL(midiStreamOut, hMidiStream, lpMidiHdr, cbMidiHdr);
}

DWORD  RealFuncPtr_CreateSoudBuffer;
DWORD* HookFuncPtr_CreateSoundBuffer = NULL;
DWORD  RealFuncPtr_DirectSoundBuffer_UnLock;
DWORD* HookFuncPtr_DirectSoundBuffer_UnLock = NULL;

typedef HRESULT (WINAPI *FuncDefine_DirectSoundCreate)(__in_opt LPCGUID pcGuidDevice, __deref_out LPDIRECTSOUND *ppDS, __null LPUNKNOWN pUnkOuter);
FuncDefine_DirectSoundCreate Real_DirectSoundCreate;
typedef HRESULT(WINAPI *FuncDefine_DirectSoundCreate8)(__in_opt LPCGUID pcGuidDevice, __deref_out LPDIRECTSOUND8 *ppDS, __null LPUNKNOWN pUnkOuter);
FuncDefine_DirectSoundCreate8 Real_DirectSoundCreate8;

HRESULT WINAPI Hook_CreateSoundBuffer(LPVOID pDS, LPCDSBUFFERDESC lpDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDirectSoundBuffer, LPUNKNOWN pUnkOuter);
HRESULT WINAPI Hook_DirectSoundBuffer_Unlock(LPVOID pDirectSoundBuffer, LPVOID lp1, DWORD dw1, LPVOID lp2, DWORD dw2);
HRESULT WINAPI Hook_DirectSoundCreate(__in_opt LPCGUID pcGuidDevice, __deref_out LPDIRECTSOUND *ppDS, __null LPUNKNOWN pUnkOuter)
{
	HRESULT hr = Real_DirectSoundCreate(pcGuidDevice, ppDS, pUnkOuter);

	if (HookFuncPtr_CreateSoundBuffer == NULL)
	{
		const int nFuncAddressIndex = 3;
		HookFuncPtr_CreateSoundBuffer = (DWORD*)(*((DWORD*)*ppDS) + 4 * nFuncAddressIndex);

		DWORD dwRet;
		HANDLE hProcess = ::OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		if (NULL == hProcess)
		{
			return S_FALSE;
		}
		VirtualProtectEx(hProcess, HookFuncPtr_CreateSoundBuffer, 4, PAGE_EXECUTE_READWRITE, &dwRet);
		RealFuncPtr_CreateSoudBuffer = *HookFuncPtr_CreateSoundBuffer;
		*HookFuncPtr_CreateSoundBuffer = (DWORD)Hook_CreateSoundBuffer;
	}

	return hr;
}

HRESULT WINAPI Hook_CreateSoundBuffer(LPVOID pDS, LPCDSBUFFERDESC lpDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDirectSoundBuffer, LPUNKNOWN pUnkOuter)
{
	HRESULT hr;

	IDirectSoundBuffer *pDirectSoundBuffer;
	DWORD dwpDirectSoundBuffer = (DWORD)&pDirectSoundBuffer;
	__asm
	{
		push pUnkOuter
		push dwpDirectSoundBuffer
		push lpDSBufferDesc
		push pDS
		mov eax, RealFuncPtr_CreateSoudBuffer
		call eax
		mov hr, eax
	}
	*ppDirectSoundBuffer = pDirectSoundBuffer;

	if (HookFuncPtr_DirectSoundBuffer_UnLock == NULL)
	{
		const int nFuncAddressIndex = 19;
		HookFuncPtr_DirectSoundBuffer_UnLock = (DWORD*)(*((DWORD*)*ppDirectSoundBuffer) + 4 * nFuncAddressIndex);

		DWORD dwRet;
		HANDLE hProcess = ::OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, GetCurrentProcessId());
		if (NULL == hProcess)
		{
			return S_FALSE;
		}

		VirtualProtectEx(hProcess, HookFuncPtr_DirectSoundBuffer_UnLock, 4, PAGE_EXECUTE_READWRITE, &dwRet);
		RealFuncPtr_DirectSoundBuffer_UnLock = *HookFuncPtr_DirectSoundBuffer_UnLock;
		*HookFuncPtr_DirectSoundBuffer_UnLock = (DWORD)Hook_DirectSoundBuffer_Unlock;
	}

	return hr;
}

HRESULT WINAPI Hook_DirectSoundBuffer_Unlock(LPVOID pDirectSoundBuffer, LPVOID lp1, DWORD dw1, LPVOID lp2, DWORD dw2)
{
	HRESULT hr;
	if (MuteVolume_XP::MUTE)
	{
		memset(lp1, 0, dw1);
		memset(lp2, 0, dw2);
	}
	__asm
	{
		push dw2
		push lp2
		push dw1
		push lp1
		push pDirectSoundBuffer
		mov eax, RealFuncPtr_DirectSoundBuffer_UnLock
		call eax
		mov hr, eax
	}

	return hr;
}

void MuteVolume_XP::Init()
{
	Real_DirectSoundCreate = (FuncDefine_DirectSoundCreate)Detours::Instance()->Find("dsound.dll", "DirectSoundCreate");
	Real_DirectSoundCreate8 = (FuncDefine_DirectSoundCreate8)Detours::Instance()->Find("dsound.dll", "DirectSoundCreate8");
	
	DETOURS_FUNC_ATTACH(waveOutWrite);
	DETOURS_FUNC_ATTACH(midiStreamOut);
	DETOURS_FUNC_ATTACH(DirectSoundCreate);

	Detours::Instance()->Attach(Real_DirectSoundCreate8, Hook_DirectSoundCreate);
}

void MuteVolume_XP::Uninit()
{
	DETOURS_FUNC_DETACH(waveOutWrite);
	DETOURS_FUNC_DETACH(midiStreamOut);
	DETOURS_FUNC_DETACH(DirectSoundCreate);

	Detours::Instance()->Detach(Real_DirectSoundCreate8, Hook_DirectSoundCreate);
}

void MuteVolume_XP::Mute(bool bMute)
{
	MuteVolume_XP::MUTE = bMute;
}

bool MuteVolume_XP::IsMuted()
{
	return MuteVolume_XP::MUTE;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void MuteVolume_WinVista::Init()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CCoreAudioVolume::Initlialize(TRUE);
}

void MuteVolume_WinVista::Uninit()
{
	CCoreAudioVolume::Uninitialize();
	CoUninitialize();
}

void MuteVolume_WinVista::Mute(bool bMute)
{
	CCoreAudioVolume::EnableSound(!bMute);
}


bool MuteVolume_WinVista::IsMuted()
{
	return !CCoreAudioVolume::IsEnableSound();
}

MuteVolumeManager::MuteVolumeManager() : mMuteVolume(NULL)
{
}

bool MuteVolumeManager::Init()
{
	if (Utility::IsWinXP())
		mMuteVolume = new MuteVolume_XP();
	else
		mMuteVolume = new MuteVolume_WinVista();
	mMuteVolume->Init();

	return true;
}

void MuteVolumeManager::Uninit()
{
	if (mMuteVolume)
	{
		mMuteVolume->Uninit();
		delete mMuteVolume;
	}
}

void MuteVolumeManager::Mute(bool bMute)
{
	mMuteVolume->Mute(bMute);
}

bool MuteVolumeManager::IsMuted()
{
	return mMuteVolume->IsMuted();
}