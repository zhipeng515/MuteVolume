#pragma once

#include "Singleton.h"

class MuteVolume
{
public:
	virtual void Init() = 0;
	virtual void Uninit() = 0;
	virtual void Mute(BOOL bMute) = 0;
	virtual BOOL isMuted() = 0;
};

class MuteVolume_XP : public MuteVolume
{
public:
	virtual void Init();
	virtual void Uninit();
	virtual void Mute(BOOL bMute);
	virtual BOOL isMuted();

	static BOOL MUTE;
};

class MuteVolume_WinVista : public MuteVolume
{
public:
	virtual void Init();
	virtual void Uninit();
	virtual void Mute(BOOL bMute);
	virtual BOOL isMuted();
};

class MuteVolumeManager : public Singleton<MuteVolumeManager>
{
public:
	MuteVolumeManager();
	~MuteVolumeManager();
	void Mute(BOOL bMute);
	virtual BOOL isMuted();

private:
	MuteVolume * mMuteVolume;
};