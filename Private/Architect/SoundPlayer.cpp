#include "SoundPlayer.h"

#include "SexyAppFramework/DSoundInstance.h"
#include "SexyAppFramework/SexyApp.h"
#include "SexyAppFramework/DSoundManager.h"

using namespace Sexy;
using namespace JeffLib;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
SoundPlayer::SoundPlayer(Sexy::SexyApp* a)
{
	mApp = a; 
	mUpdateCnt = mMuteCount = 0;
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
		mCachedSounds[i] = NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
SoundPlayer::~SoundPlayer()
{
	ReleaseAllCached();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::Update()
{
	++mUpdateCnt;

	for (int i = 0; i < mDelayedSFX.size(); i++)
	{
		if (mDelayedSFX[i].second-- <= 0)
		{
			Play(mDelayedSFX[i].first, 0);
			mDelayedSFX.erase(mDelayedSFX.begin() + i);
			--i;
		}
	}

	// Poll the sounds and query whether or not they are done playing. When
	// one is done, trigger the next after the desired delay has elapsed.
	for (int i = 0; i < mChainedSFX.size(); i++)
	{			
		if (!mChainedSFX[i].mSourceSound->IsPlaying())
		{
			if (--mChainedSFX[i].mDelay <= 0)
			{
				Play(mChainedSFX[i].mNextId, 0);
				mChainedSFX.erase(mChainedSFX.begin() + i);
				--i;
			}
		}
	}

	for (LoopingSoundMap::iterator it = mLoopingSounds.begin(); it != mLoopingSounds.end(); ++it)
	{
		// See if we need to fade out the sound...
		if (it->second.second > 0)
		{
			double v = it->second.first->GetVolume();
			v -= it->second.second;

			if (v > 0)
				it->second.first->SetVolume(v);
			else
			{
				// Fade volume dropped below 0, remove the looping sound
				it->second.first->Release();
				mLoopingSounds.erase(it);
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::PlayDelayed(int sound_id, int delay)
{
	mDelayedSFX.push_back(IntPairVector::value_type(sound_id, delay));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::Play(int sound_id, int min_time)
{
	IntMap::iterator it = mSoundTimes.find(sound_id);
	
	// Make sure we aren't playing the sound too recently
	if ((it == mSoundTimes.end()) || (mUpdateCnt - it->second > min_time))
	{
		// Get a cached one if it exists
		SoundInstance* si; 
		bool auto_rel = true;
		if (mCachedSounds[sound_id] != NULL)
		{
			auto_rel = false;
			si = mCachedSounds[sound_id];
		}
		else
			si = mApp->mSoundManager->GetSoundInstance(sound_id);

		if (si != NULL)
			si->Play(false, auto_rel);

		mSoundTimes.insert(IntMap::value_type(sound_id, mUpdateCnt));
	}
}
 
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::Play(int sound_id, int pan, int min_time)
{
	IntMap::iterator it = mSoundTimes.find(sound_id);

	// Make sure we aren't playing the sound too recently
	if ((it == mSoundTimes.end()) || (mUpdateCnt - it->second > min_time))
	{
		// Get a cached one if it exists
		SoundInstance* si; 
		bool auto_rel = true;
		if (mCachedSounds[sound_id] != NULL)
		{
			auto_rel = false;
			si = mCachedSounds[sound_id];
		}
		else
			si = mApp->mSoundManager->GetSoundInstance(sound_id);

		if (si != NULL)
		{
			si->SetPan(pan);
			si->Play(false, auto_rel);
		}

		mSoundTimes.insert(IntMap::value_type(sound_id, mUpdateCnt));
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::PlayCached(int sound_id, int pan, int min_time)
{
	IntMap::iterator it = mSoundTimes.find(sound_id);

	// Make sure we aren't playing the sound too recently
	if ((it == mSoundTimes.end()) || (mUpdateCnt - it->second > min_time))
	{
		// Maintain our own pointer to the sound
		if (mCachedSounds[sound_id] == NULL)
			mCachedSounds[sound_id] = mApp->mSoundManager->GetSoundInstance(sound_id);

		SoundInstance* si = mCachedSounds[sound_id];
		if (si)
		{
			if (pan != 0)
				si->SetPan(pan);

			// Make sure we don't auto-release, we'll handle releasing ourselves.
			si->Play(false, false);
		}

		mSoundTimes.insert(IntMap::value_type(sound_id, mUpdateCnt));
	}
}
			

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::PlayChained(int sound_id, int next_sound_id, int delay)
{
	if (!mApp->mSoundManager)
		return;

	SoundInstance* si = mApp->mSoundManager->GetSoundInstance(sound_id);
	if (si != NULL)
	{
		si->Play(false, false);
		mChainedSFX.push_back(ChainedSound(sound_id, next_sound_id, delay, si));
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::Loop(int sound_id)
{
	if (mLoopingSounds.find(sound_id) != mLoopingSounds.end())
		return;

	DSoundInstance* d = (DSoundInstance*) mApp->mSoundManager->GetSoundInstance(sound_id);
	if (d != NULL)
	{
		d->Play(true, false);
		mLoopingSounds[sound_id] = std::pair<DSoundInstance*, float>(d, 0);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::Stop(int sound_id)
{
	LoopingSoundMap::iterator it = mLoopingSounds.find(sound_id);
	if (it == mLoopingSounds.end())
		return;

	it->second.first->Release();
	mLoopingSounds.erase(it);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::FadeOut(int id, float speed)
{
	LoopingSoundMap::iterator it = mLoopingSounds.find(id);
	if (it == mLoopingSounds.end())
		return;

	it->second.second = speed;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::PlayPitchShifted(int sound_id, int pitch, int min_time)
{
	if (!mApp->mSoundManager)
		return;

	SoundInstance* si = mApp->mSoundManager->GetSoundInstance(sound_id);
	if (si != NULL)
	{
		
		IntMap::iterator it = mSoundTimes.find(sound_id);
		if (it == mSoundTimes.end())
		{
			si->AdjustPitch(pitch);
			si->Play(false, true);
			mSoundTimes.insert(IntMap::value_type(sound_id, mUpdateCnt));
		}
		else if (mUpdateCnt - it->second > min_time)
		{
			si->AdjustPitch(pitch);
			si->Play(false, true);
			mSoundTimes.insert(IntMap::value_type(sound_id, mUpdateCnt));
		}		
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::MuteLoopingSounds(bool m)
{
	mMuteCount += m ? 1 : -1;
	if (mMuteCount < 0)
		mMuteCount = 0;

	if ((!m && (mMuteCount == 0)) || m)
	{
		for (LoopingSoundMap::iterator it = mLoopingSounds.begin(); it != mLoopingSounds.end(); ++it)
			it->second.first->SetVolume(m ? 0 : 1);

	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::SetVolume(int sound_id, double volume)
{
	SoundInstance* si = NULL; 
	if (mCachedSounds[sound_id] != NULL)
		si = mCachedSounds[sound_id];

	if (si != NULL)
		si->SetVolume(volume);
}

void SoundPlayer::IncVolume(int sound_id, double inc)
{
	SoundInstance* si = NULL; 
	if (mCachedSounds[sound_id] != NULL)
		si = mCachedSounds[sound_id];

	if (si != NULL)
	{
		double current = si->GetVolume();
		si->SetVolume(current + inc);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SoundPlayer::ReleaseCached(int sound_id)
{
	if (mCachedSounds[sound_id])
		mCachedSounds[sound_id]->Release();

	mCachedSounds[sound_id] = NULL;
}

void SoundPlayer::ReleaseAllCached()
{
	for (int i = 0; i < MAX_SOURCE_SOUNDS; i++)
		ReleaseCached(i);
}