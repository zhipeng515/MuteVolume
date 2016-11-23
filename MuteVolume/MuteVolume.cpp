#include "stdafx.h"

#include "MuteVolume.h"
#include "../../CommonFunction/DetoursWrapper.h"
#include "CCoreAudioVolume.h"
#include <dsound.h>
#include "../../CommonFunction/Utility.h"
#include <MMSystem.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "dsound.lib")

bool MuteVolume_XP::MUTE = FALSE;

DWORD  RealFuncPtr_CreateSoudBuffer;
DWORD* HookFuncPtr_CreateSoundBuffer = NULL;
DWORD  RealFuncPtr_DirectSoundBuffer_UnLock;
DWORD* HookFuncPtr_DirectSoundBuffer_UnLock = NULL;

typedef MMRESULT (WINAPI *FuncDefine_waveOutWrite)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
FuncDefine_waveOutWrite Real_waveOutWrite = waveOutWrite;
MMRESULT WINAPI Hook_waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
	if (MuteVolume_XP::MUTE)
		memset(pwh->lpData, 0, pwh->dwBufferLength);
	return  Real_waveOutWrite(hwo, pwh, cbwh);
}


typedef MMRESULT (WINAPI *FuncDefine_midiStreamOut)(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr);
FuncDefine_midiStreamOut Real_midiStreamOut = midiStreamOut;
MMRESULT WINAPI Hook_midiStreamOut(HMIDISTRM hMidiStream, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr)
{
	if (MuteVolume_XP::MUTE)
		memset(lpMidiHdr->lpData, 0, lpMidiHdr->dwBufferLength);
	return  Real_midiStreamOut(hMidiStream, lpMidiHdr, cbMidiHdr);
}


typedef HRESULT (WINAPI *FuncDefine_DirectSoundCreate)(__in_opt LPCGUID pcGuidDevice, __deref_out LPDIRECTSOUND *ppDS, __null LPUNKNOWN pUnkOuter);
FuncDefine_DirectSoundCreate Real_DirectSoundCreate = DirectSoundCreate;

HRESULT WINAPI Hook_CreateSoundBuffer(LPVOID pDS, LPCDSBUFFERDESC lpDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDirectSoundBuffer, LPUNKNOWN pUnkOuter);
HRESULT WINAPI Hook_DirectSoundBuffer_Unlock(LPVOID pDirectSoundBuffer, LPVOID lp1, DWORD dw1, LPVOID lp2, DWORD dw2);
HRESULT WINAPI Hook_DirectSoundCreate(__in_opt LPCGUID pcGuidDevice, __deref_out LPDIRECTSOUND *ppDS, __null LPUNKNOWN pUnkOuter)
{
	HRESULT hr = Real_DirectSoundCreate(pcGuidDevice, ppDS, pUnkOuter);

	if (HookFuncPtr_CreateSoundBuffer == NULL)
	{
		const int nFuncAddressIndex = 3;
		HookFuncPtr_CreateSoundBuffer = (DWORD*)(*((DWORD*)&ppDS) + 4 * nFuncAddressIndex);

		DWORD dwRet;
		VirtualProtectEx((HANDLE)-1, HookFuncPtr_CreateSoundBuffer, 4, 64, &dwRet);
		RealFuncPtr_CreateSoudBuffer = *HookFuncPtr_CreateSoundBuffer;
		*HookFuncPtr_CreateSoundBuffer = (DWORD)Hook_CreateSoundBuffer;
	}

	return hr;
}

HRESULT WINAPI Hook_CreateSoundBuffer(LPVOID pDS, LPCDSBUFFERDESC lpDSBufferDesc, LPDIRECTSOUNDBUFFER* ppDirectSoundBuffer, LPUNKNOWN pUnkOuter)
{
	HRESULT hr;

	DWORD dwpDirectSoundBuffer = (DWORD)ppDirectSoundBuffer;
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

	if (HookFuncPtr_DirectSoundBuffer_UnLock == NULL)
	{
		const int nFuncAddressIndex = 19;
		HookFuncPtr_DirectSoundBuffer_UnLock = (DWORD*)(*((DWORD*)(&ppDirectSoundBuffer)) + 4 * nFuncAddressIndex);

		DWORD dwRet;
		VirtualProtectEx((HANDLE)-1, HookFuncPtr_DirectSoundBuffer_UnLock, 4, 64, &dwRet);
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
	Detours::Instance()->Attach(Real_waveOutWrite, Hook_waveOutWrite);
	Detours::Instance()->Attach(Real_midiStreamOut, Hook_midiStreamOut);
	Detours::Instance()->Attach(Real_DirectSoundCreate, Hook_DirectSoundCreate);
}

void MuteVolume_XP::Uninit()
{
	Detours::Instance()->Detach(Real_waveOutWrite, Hook_waveOutWrite);
	Detours::Instance()->Detach(Real_midiStreamOut, Hook_midiStreamOut);
	Detours::Instance()->Detach(Real_DirectSoundCreate, Hook_DirectSoundCreate);
}

void MuteVolume_XP::Mute(bool bMute)
{
	MUTE = bMute;
}

bool MuteVolume_XP::isMuted()
{
	return MUTE;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void MuteVolume_WinVista::Init()
{
	CCoreAudioVolume::Initlialize(TRUE);
}

void MuteVolume_WinVista::Uninit()
{
	CCoreAudioVolume::Uninitialize();
}

void MuteVolume_WinVista::Mute(bool bMute)
{
	CCoreAudioVolume::EnableSound(!bMute);
}


bool MuteVolume_WinVista::isMuted()
{
	return !CCoreAudioVolume::IsEnableSound();
}


MuteVolumeManager::MuteVolumeManager()
{
	if (Utility::IsWinXP())
		mMuteVolume = new MuteVolume_XP();
	else
		mMuteVolume = new MuteVolume_WinVista();
	mMuteVolume->Init();
}

MuteVolumeManager::~MuteVolumeManager()
{
	mMuteVolume->Uninit();
	delete mMuteVolume;
}

void MuteVolumeManager::Mute(bool bMute)
{
	mMuteVolume->Mute(bMute);
}

bool MuteVolumeManager::isMuted()
{
	return mMuteVolume->isMuted();
}