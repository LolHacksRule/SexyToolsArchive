//////////////////////////////////////////////////////////////////////////
//				Animator.h
//			
//		FEATURES:
//		* Subsection looping
//		* ping-pong animations
//		* additve drawing
//		* random frame drawing
//		* running for a set period of time
//		* looping for a set number of counts
//		* variable delays between each frame
//		* incrementing the frame count by a variable amount
//		* Can use with or without an image: if using without, the purpose
//			is to use the class as just a generic looper/timer/random frame chooser/etc
//		* Simple stretching/shrinking of animations
//		* Rotation of animations
//
//		DEFAULTS: paused, no frame delay, forward looping, not looping
//				  subsection, don't stop when done, single stepping,
//				  infinate loop count.
//
//		Author: Jeff "Architekt" Weinstein, jeff@popcap.com
//////////////////////////////////////////////////////////////////////////


#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__

#include "SexyAppFramework/Color.h"

namespace Sexy
{

	class Image;
	class Graphics;

class Animator
{

	protected:
		
		int			mUpdateCnt;
		int			mCurrentFrame;
		int			mMaxFrames;
		int			mFrameDelay;		// how many update counts to wait before stepping to the next frame
		int			mNumIterations;		// how many times to cycle
		int			mLoopCount;			// how many we've done
		int			mLoopStart;			// you can have it loop a specific...
		int			mLoopEnd;			// ...chunk of the entire anim
		int			mLoopDir;			// + means forward, - means backward, only applies to non ping-pong animations
		int			mStepAmt;			// how many frames to step at a time, default is 1
		int			mId;				// an optional ID tag
		int			mPriority;			// optional drawing priority, used for user sorted priority queues
		int			mTimeLimit;			// the animation can repeat for a set amount of time instead of just running for a loop count
		int			mCurrentTime;		// how much time has elapsed, used with mTimeLimit (all time is in update counts)
		float		mXOff, mYOff;		// how much to offset the drawing by
		
		bool		mPaused;
		bool		mPingPong;		
		bool		mAnimForward;		// If true, means the frames are incrementing, if false, decrementing
		bool		mStopWhenDone;		// If true, the animation stops animating at the end of its frames
		bool		mDone;				// If true, not animating any more
		bool		mFrameChanged;		// True if the frame has changed since the last update
		bool		mLoopSubsection;	// if true, loops a subsection after teh entire animation plays
		bool		mDrawAdditive;		// If true, draws the image additively
		bool		mDrawRandomly;		// if true, will select a random frame each time instead of linearly incrementing

		Color		mAdditiveColor;		// Color to use for additive drawing
		Image*		mImage;				// The image to draw, if any

		std::vector<int>	mFrameDelays;	//How much to delay between each frame
		std::vector<int>	mRandomFrames;	//a pool of available frames to randomly choose to draw

	public:

		bool		mCanRotate;			// is this image allowed to be drawn rotated?
		bool		mResetOnStart;		// flag indicating whether to reset the animation when it's first played or not

	public:

		Animator();
		virtual ~Animator() {;}

		//////////////////////////////////////////////////////////////////////////
		//	Updates the current animation frame, if not paused/stopped
		//////////////////////////////////////////////////////////////////////////		
		virtual void UpdateAnim(void);

		//////////////////////////////////////////////////////////////////////////
		//	Pauses the animation if true, otherwise unpauses.
		//////////////////////////////////////////////////////////////////////////		
		virtual void PauseAnim(bool pPause);

		//////////////////////////////////////////////////////////////////////////
		//  Resets the animation entirely and begins playing it.
		//	RESETS ALL SUBSECTION LOOPING INFO. Does not reset loop direction.
		//////////////////////////////////////////////////////////////////////////
		virtual void ResetAnim(void);

		//////////////////////////////////////////////////////////////////////////
		//	After the full animation plays, this will loop a subsection
		//	according to the same rules that apply for a full animation.
		//	YOU MUST HAVE SET THE MAX NUMBER OF FRAMES BEFORE CALLING THIS.
		//////////////////////////////////////////////////////////////////////////
		virtual void LoopSubsection(int pStartFrame, int pEndFrame);

		//////////////////////////////////////////////////////////////////////////
		//	Sets the looping direction. >= 0 means forward (default), < 0 means
		//	backwards. 0 means stupid.
		//////////////////////////////////////////////////////////////////////////
		virtual void SetLoopDir(int pDir);

		//////////////////////////////////////////////////////////////////////////
		//	Sets a time limit. When the time is up, the animation will stop,
		//	regardless of other parameters such as loop counts, stop when done,
		//	etc. However, if you also set a loop count, and that expires before
		//	the timer does, the animation will then stop as well. First come
		//	first serve basis. ALL TIME IS IN UPDATE COUNTS, ALWAYS.
		//////////////////////////////////////////////////////////////////////////
		virtual void SetTimeLimit(int t) {mTimeLimit = t; mCurrentTime = 0;}

		//////////////////////////////////////////////////////////////////////////
		//	Instructs this animation to draw additively using the specified
		//	color. 
		//////////////////////////////////////////////////////////////////////////
		virtual void DrawAdditively(Color pColor)	{mDrawAdditive = true; mAdditiveColor = pColor;}
		virtual void StopDrawingAdditively(void)	{mDrawAdditive = false;}

		//////////////////////////////////////////////////////////////////////////
		//		Sets the maximum number of frames. DO NOT USE IF SPECIFYING AN
		//	IMAGE TO DRAW, AS THE IMAGE WILL CONTAIN THE MAX NUMBER OF FRAMES.
		//////////////////////////////////////////////////////////////////////////		
		virtual void SetMaxFrames(int pMax);

		//////////////////////////////////////////////////////////////////////////
		// Sets the image to use for drawing. DO NOT USE WITH SET MAX FRAMES
		// AS THE IMAGE WILL CONTAIN THE NUMBER OF FRAMES TO DRAW ON ITS OWN. 
		// That is, assuming you either set the number of rows/columns manually
		// or used the resource manager to do so.
		//////////////////////////////////////////////////////////////////////////
		virtual void SetImage(Image* pImage);

		//////////////////////////////////////////////////////////////////////////
		//	Draws the image, if there is one, at the given coordinate with
		//	the current frame. All drawing is offset by mX/YOff.
		//////////////////////////////////////////////////////////////////////////
		virtual void Draw(Graphics* g, int pX, int pY) const;
		virtual void Draw(Graphics* g, float pX, float pY, bool pSmooth) const;

		//////////////////////////////////////////////////////////////////////////
		//	Draws the animation frame stretched by a certain percent. Valid values
		//	are 0 through 1.0.
		//////////////////////////////////////////////////////////////////////////		
		virtual void DrawStretched(Graphics* g, float pX, float pY, float pPct) const;

		//////////////////////////////////////////////////////////////////////////
		//	Draws the animation stretched/shrunk to the given width/height
		//////////////////////////////////////////////////////////////////////////		
		virtual void DrawStretched(Graphics* g, float pX, float pY, int pWidth, int pHeight) const;

		//////////////////////////////////////////////////////////////////////////
		//	Draws the image rotated about a specified center at a given angle.
		//	Specify true for smooth rotation (longer draw time but nicer image)
		//	or false for non smooth rotation (faster draw time but less nicer).
		//////////////////////////////////////////////////////////////////////////		
		virtual void DrawRotated(Graphics* g, float pX, float pY, float pAngle, bool pSmooth, float pCenterX, float pCenterY) const;

		//////////////////////////////////////////////////////////////////////////
		//	Draws a random frame instead of linearly progressing through
		//	frames. MUST BE CALLED AFTER HAVING SET MAX FRAMES OR HAVING
		//	SPECIFIED AN IMAGE.
		//////////////////////////////////////////////////////////////////////////
		virtual void DrawRandomly(bool pRandom);

		//////////////////////////////////////////////////////////////////////////
		//Sets a max number of times to loop. AUTOMATICALLY STOPS WHEN DONE,
		//REGARDLESS OF WHETHER STOPWHENDONE IS SET OR NOT. A loop is 
		//one full cycle of frames, so in a ping-pong anim it means 
		//that both a ping and a pong each count as a loop, because a full
		//iteration over all the frames has been made.
		//////////////////////////////////////////////////////////////////////////		
		virtual void SetNumIterations(int aIt)	{mNumIterations = aIt; mLoopCount = 0;}

		//////////////////////////////////////////////////////////////////////////
		//	Sets the current frame to the given frame number. YOU MUST MAKE
		//	SURE THIS IS A VALID FRAME AND ISN'T OUT OF BOUNDS.
		//////////////////////////////////////////////////////////////////////////		
		virtual void	SetFrame(int pFrame);

		//////////////////////////////////////////////////////////////////////////
		//	Sets the delay between all frames.
		//////////////////////////////////////////////////////////////////////////		
		virtual void 	SetDelay(int pDelay);

		//////////////////////////////////////////////////////////////////////////
		//	For a given frame, sets the delay for it. This is the amount
		//	of time to wait before changing frames once "pFrame" has
		//	become active. Useful for specifying non-default delays on a per-frame
		//	basis.
		//////////////////////////////////////////////////////////////////////////		
		virtual void 	SetDelay(int pDelay, int pFrame);	//frame is 0 based!

		//////////////////////////////////////////////////////////////////////////		
		// Returns true if there has been a frame change since the last time this 
		// function was called. IMPORTANT: IF THIS METHOD RETURNS TRUE, IT
		// IMMEDIATELY MODIFIES INTERNAL VARIABLES SO THAT ANY SUBSEQUENT CALLS
		// WILL RETURN FALSE, UNTIL THE NEXT FRAME CHANGE OCCURS. Thus, do not
		// query this more than once per update.
		//////////////////////////////////////////////////////////////////////////		
		virtual bool	FrameChanged(void);

		virtual Animator &operator =(const Animator &rhs);

		//////////////////////////////////////////////////////////////////////////
		//					ACCESSOR METHODS
		//////////////////////////////////////////////////////////////////////////		
		bool	IsDone(void) const					{return (mDone || ((mNumIterations > 0) && (mLoopCount >= mNumIterations)));}
		bool	IsPaused(void) const				{return mPaused;}
		bool	PingPongs(void) const				{return mPingPong;}
		bool	IsPlaying(void)	const				{return !IsDone() && !IsPaused();}
		bool	StopWhenDone(void) const			{return mStopWhenDone;}
		int		GetFrame(void) const				{return mMaxFrames == 1 ? 0 : mCurrentFrame;}
		int		GetMaxFrames(void) const			{return mMaxFrames;}
		int		GetDelay(void) const				{return mFrameDelay;}
		int		GetStepAmt(void) const				{return mStepAmt;}
		int		GetId(void) const					{return mId;}
		int		GetPriority(void) const				{return mPriority;}
		float	GetXOff(void) const					{return mXOff;}
		float	GetYOff(void) const					{return mYOff;}
		Image*	GetImage(void) const				{return mImage;}

		void 	SetPingPong(bool pPong)				{mPingPong = pPong;}
		void 	StopWhenDone(bool pStop)			{mStopWhenDone = pStop;}		
		void 	SetStepAmount(int pStep)			{mStepAmt = pStep;}
		void 	SetXOffset(float x)					{mXOff = x;}
		void 	SetYOffset(float y)					{mYOff = y;}
		void 	SetXYOffset(float x, float y)		{SetXOffset(x); SetYOffset(y);}
		void 	SetId(int id)						{mId = id;}
		void 	SetPriority(int p)					{mPriority = p;}
		void 	SetDone(void)						{mDone = true;}

		



};

}

#endif