#pragma once

////////////////////////////////////////////////////////////////////////////////
//	A basic sound playing class with some extra useful features.
////////////////////////////////////////////////////////////////////////////////

#include "SexyAppFramework/SoundManager.h"
#include <vector>
#include <utility>
#include <map>

namespace Sexy
{
	class SexyApp;
	class SoundInstance;
	class DSoundInstance;
}


namespace JeffLib
{
	typedef std::map<int, int> IntMap;
	typedef std::vector<std::pair<int, int> > IntPairVector;
	typedef std::map<int, std::pair<Sexy::DSoundInstance*, float> > LoopingSoundMap;

	struct ChainedSound
	{
		int							mSourceId;				// The Sound ID of the first, playing sound
		int							mNextId;				// The Sound ID to play after completion of the source sound
		int							mDelay;					// How long to wait after playing the first sound before playing the second after the first is done
		Sexy::SoundInstance*		mSourceSound;			// SoundInstance used to poll when the first sound is done playing

		ChainedSound() {mSourceId = mNextId = mDelay = -1;}
		ChainedSound(int s, int n, int d, Sexy::SoundInstance* si) {mSourceId = s; mNextId = n; mDelay = d; mSourceSound = si;}
	};

	////////////////////////////////////////////////////////////////////////////////
	//	The main class. You need to call Update() yourself each frame.
	////////////////////////////////////////////////////////////////////////////////
	class SoundPlayer
	{

		protected:

			Sexy::SexyApp*				mApp;
			IntMap						mSoundTimes;			// Keeps track of when a given sound ID last played
			IntPairVector				mDelayedSFX;			// Keeps a list of delay times before playing a sound
			std::vector<ChainedSound>	mChainedSFX;			// A list of sounds who will play another sound effect when complete
			LoopingSoundMap				mLoopingSounds;			// A list of sounds that loop over and over
			int							mUpdateCnt;
			int							mMuteCount;
			Sexy::SoundInstance*		mCachedSounds[MAX_SOURCE_SOUNDS];	// Used by PlayCached

		public:

			SoundPlayer(Sexy::SexyApp* a);
			virtual ~SoundPlayer();

			////////////////////////////////////////////////////////////////////////////////
			// IMPORTANT: Call this every frame in the app
			////////////////////////////////////////////////////////////////////////////////
			void	Update();

			////////////////////////////////////////////////////////////////////////////////
			// Plays the given sound after a specified number of frames
			////////////////////////////////////////////////////////////////////////////////
			void	PlayDelayed(int sound_id, int delay);

			////////////////////////////////////////////////////////////////////////////////
			// Plays the given sample: specify a minimum amount of time between duplicates
			// to avoid flanging effects (or 0 if you don't care). The second form allows
			// a panning value which has a range of -hundredth db to +hundredth db = left to right.
			////////////////////////////////////////////////////////////////////////////////
			void	Play(int sound_id, int min_time);
			void	Play(int sound_id, int pan, int min_time);
			
			////////////////////////////////////////////////////////////////////////////////
			// Will cache the sound effect. Subsequent calls to Play or PlayCached will use
			// the cached sound instead. Set/IncVolume calls will also operate on the cached
			// sound rather than a new instance. Use 0 for pan to not pan at all. If you
			// don't cache a sound and try to use Set/IncVolume then the volume change will
			// only work on that specificly played sample: subsequent playing of the same
			// sample will not be at the last set volume. The other functions do not
			// use cached sounds from this function.
			////////////////////////////////////////////////////////////////////////////////
			void	PlayCached(int sound_id, int pan, int min_time);

			////////////////////////////////////////////////////////////////////////////////
			//	Releases a specific sound or all sounds. All cached sounds are auto-released
			//	in the destructor, otherwise they remain cached until removed.
			////////////////////////////////////////////////////////////////////////////////
			void	ReleaseCached(int sound_id);
			void	ReleaseAllCached();

			////////////////////////////////////////////////////////////////////////////////
			// 0 = silent, 1 = max volume. This will only work with a sample that
			// was previously cached.
			////////////////////////////////////////////////////////////////////////////////
			void	SetVolume(int sound_id, double volume);
			void	IncVolume(int sound_id, double inc);

			////////////////////////////////////////////////////////////////////////////////
			// Plays a sample: when it is done will play the next sample.
			// You can specify a time delay to elapse after the original sound completes before
			// triggering the next sound.
			////////////////////////////////////////////////////////////////////////////////
			void	PlayChained(int sound_id, int next_sound_id, int delay);

			////////////////////////////////////////////////////////////////////////////////
			// Loops the given sound id with no delay for a continuous sound.
			////////////////////////////////////////////////////////////////////////////////
			void	Loop(int sound_id);

			////////////////////////////////////////////////////////////////////////////////
			// Stops a looping sound.
			////////////////////////////////////////////////////////////////////////////////
			void	Stop(int id);

			////////////////////////////////////////////////////////////////////////////////
			// Mutes/unmutes looping sounds ONLY.
			////////////////////////////////////////////////////////////////////////////////
			void	MuteLoopingSounds(bool m);

			////////////////////////////////////////////////////////////////////////////////
			// Fades out a looping sound. When the volume reaches 0, the sound will
			// be released.
			////////////////////////////////////////////////////////////////////////////////
			void	FadeOut(int id, float speed);

			////////////////////////////////////////////////////////////////////////////////
			// Plays the given sound pitch shifted by a certain amount.
			// specify a minimum amount of time between duplicates
			// to avoid flanging effects (or 0 if you don't care).
			////////////////////////////////////////////////////////////////////////////////
			void	PlayPitchShifted(int sound_id, int pitch, int min_time);			
	};


}