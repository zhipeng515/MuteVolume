#pragma once

#include "../../CommonFunction/Singleton.h"

class MuteVolume
{
public:
	virtual void Init() = 0;
	virtual void Uninit() = 0;
	virtual void Mute(bool bMute) = 0;
	virtual bool IsMuted() = 0;
};

class MuteVolume_XP : public MuteVolume
{
public:
	virtual void Init();
	virtual void Uninit();
	virtual void Mute(bool bMute);
	virtual bool IsMuted();

	static bool MUTE;
};

class MuteVolume_WinVista : public MuteVolume
{
public:
	virtual void Init();
	virtual void Uninit();
	virtual void Mute(bool bMute);
	virtual bool IsMuted();
};

class MuteVolumeManager : public Singleton<MuteVolumeManager>
{
public:
	MuteVolumeManager();

	virtual bool Init();
	virtual void Uninit();
	void Mute(bool bMute);
	bool IsMuted();

private:
	MuteVolume * mMuteVolume;
};