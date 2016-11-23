#pragma once

#include "../../CommonFunction/Singleton.h"

#ifndef _LIB
#ifdef _DEBUG
#pragma comment (lib, "../Debug/MuteVolume.lib")
#else
#pragma comment (lib, "../Release/MuteVolume.lib")
#endif
#endif

class MuteVolume
{
public:
	virtual void Init() = 0;
	virtual void Uninit() = 0;
	virtual void Mute(bool bMute) = 0;
	virtual bool isMuted() = 0;
};

class MuteVolume_XP : public MuteVolume
{
public:
	virtual void Init();
	virtual void Uninit();
	virtual void Mute(bool bMute);
	virtual bool isMuted();

	static bool MUTE;
};

class MuteVolume_WinVista : public MuteVolume
{
public:
	virtual void Init();
	virtual void Uninit();
	virtual void Mute(bool bMute);
	virtual bool isMuted();
};

class MuteVolumeManager : public Singleton<MuteVolumeManager>
{
public:
	MuteVolumeManager();
	~MuteVolumeManager();
	void Mute(bool bMute);
	virtual bool isMuted();

private:
	MuteVolume * mMuteVolume;
};