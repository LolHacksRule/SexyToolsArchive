#pragma warning(disable : 4244 4267 4018)
#include "SexyAppFramework/Image.h"
#include "SexyAppFramework/Graphics.h"
#include "SexyAppFramework/MTRand.h"
#include "SexyAppFramework/SexyAppBase.h"
#include "Animator.h"

using namespace Sexy;

Animator::Animator(void)
{
	mPaused = true; 
	mUpdateCnt = mCurrentFrame = 0; 
	mFrameDelay = 1;
	mMaxFrames = 1; 
	mLoopDir = 1;
	mPingPong = false; 
	mAnimForward = true;
	mLoopSubsection = false;
	mStopWhenDone = false;
	mDone = false;
	mFrameChanged = true;
	mLoopCount = mNumIterations = 0;
	mStepAmt = 1;
	mImage = NULL;
	mXOff = mYOff = 0.0f;
	mLoopStart = mLoopEnd = 0;
	mId = -1;
	mPriority = 0;
	mResetOnStart = false;
	mCanRotate = false;
	mDrawAdditive = false;
	mDrawRandomly = false;
	mTimeLimit = mCurrentTime = -1;
}

//EXPECTED: pFrame < mMaxFrames
void Animator::SetFrame(int pFrame)
{
	mCurrentFrame = pFrame;
}

void Animator::SetImage(Image* pImage)
{
	mImage = NULL;
	mCurrentFrame = 0;
	if (pImage)
		SetMaxFrames(pImage->mNumCols > pImage->mNumRows ? pImage->mNumCols : pImage->mNumRows);
	mImage = pImage;
}

// EXPECTED: mImage IS NULL, SINCE THIS CAN'T BE USED WITH AN IMAGE, AS IMAGES ALREADY CONTAIN THE
// NUMBER OF FRAMES.
void Animator::SetMaxFrames(int pMax)
{
	mMaxFrames = pMax;
	mCurrentFrame = 0;

	if (mLoopDir < 0)
		mCurrentFrame = mMaxFrames - 1;

	mFrameDelays.clear();
	mFrameDelays.resize(mMaxFrames);
	for (int i = 0; i < mFrameDelays.size(); i++)
		mFrameDelays[i] = mFrameDelay;
}

void Animator::PauseAnim(bool pPause)
{
	mPaused = pPause;
}

Animator &Animator::operator =(const Animator &rhs)
{
	if (this == &rhs)
		return *this;

	mImage = NULL;
	this->mTimeLimit = rhs.mTimeLimit;
	this->mCurrentTime = rhs.mCurrentTime;
	this->mUpdateCnt = rhs.mUpdateCnt;
	this->mAnimForward = rhs.mAnimForward;	
	this->mDone = rhs.mDone;
	this->mFrameChanged = rhs.mFrameChanged;
	this->mFrameDelay = rhs.mFrameDelay;
	SetMaxFrames(rhs.mMaxFrames);
	this->mPaused = rhs.mPaused;
	this->mPingPong = rhs.mPingPong;
	this->mStopWhenDone = rhs.mStopWhenDone;
	if (rhs.mImage)
		SetImage(rhs.mImage);
	this->mNumIterations = rhs.mNumIterations;
	this->mLoopCount = rhs.mLoopCount;
	this->mLoopStart = rhs.mLoopStart;
	this->mLoopEnd = rhs.mLoopEnd;
	this->mLoopDir = rhs.mLoopDir;
	this->mStepAmt = rhs.mStepAmt;
	this->mLoopSubsection = rhs.mLoopSubsection;
	this->mXOff = rhs.mXOff;
	this->mYOff = rhs.mYOff;
	this->mPriority = rhs.mPriority;
	this->mId = rhs.mId;
	this->mDrawAdditive = rhs.mDrawAdditive;
	this->mAdditiveColor = rhs.mAdditiveColor;

	return *this;
}

void Animator::UpdateAnim(void)
{
	++mUpdateCnt;

	if ((mTimeLimit > 0) && !mPaused && !mDone && (++mCurrentTime >= mTimeLimit))
	{
		mPaused = true;
		mDone = true;
	}
		

	if (!mPaused && !mDone) 
	{
		int aStartFrame = 0;
		int aEndFrame = mMaxFrames;
		int aFrameDelay = ((mFrameDelays.size() > 0) && (mMaxFrames > 1)) ? mFrameDelays[mCurrentFrame] : mFrameDelay;

		if (mLoopSubsection && (((mLoopCount >= 1) && !mPingPong) || (mPingPong && (mLoopCount >= 2))))
		{
			aStartFrame = mLoopStart;
			aEndFrame = mLoopEnd;
		}

		if ((mUpdateCnt % aFrameDelay == 0) && ((mNumIterations == 0) || (mLoopCount <= mNumIterations)))
		{
			mFrameChanged = true;
			if (!mPingPong || mDrawRandomly)
			{
				if (!mDrawRandomly)
				{
					if (mLoopDir >= 0)
					{
						if ((mCurrentFrame += mStepAmt) >= aEndFrame)
						{
							if (mLoopSubsection)
								aStartFrame = mLoopStart;

							mCurrentFrame = aStartFrame;
							++mLoopCount;
							if (mStopWhenDone || ((mLoopCount >= mNumIterations) && (mNumIterations > 0)))
							{
								mPaused = true;
								mDone = true;
							}
						}
					}
					else
					{
						if ((mCurrentFrame -= mStepAmt) <= aStartFrame)
						{
							if (mLoopSubsection)
								aEndFrame = mLoopEnd;

							mCurrentFrame = aEndFrame;
							++mLoopCount;
							if (mStopWhenDone || ((mLoopCount >= mNumIterations) && (mNumIterations > 0)))
							{
								mPaused = true;
								mDone = true;
							}
						}
					}
				}
				else
				{
					if (mRandomFrames.size() == 0)
					{
						++mLoopCount;
						if (mStopWhenDone || ((mLoopCount >= mNumIterations) && (mNumIterations > 0)))
						{
							mPaused = true;
							mDone = true;
						}

						for (int i = 0; i < mMaxFrames; i++)
							mRandomFrames.push_back(i);
					}

					mCurrentFrame = Rand() % mRandomFrames.size();
					mRandomFrames.erase(mRandomFrames.begin() + mCurrentFrame);					
				}
			}
			else if (!mAnimForward)
			{
				if ((mCurrentFrame += mStepAmt) >= aEndFrame)
				{
					++mLoopCount;
					mCurrentFrame = aEndFrame - 2;
					if (mMaxFrames == 1)
						mCurrentFrame = 0;

					// EXPECTED: mCurrentFrame >= 0 OTHERWISE INVALID SITUATION
					mAnimForward = true;

					if ((mLoopCount >= mNumIterations) && (mNumIterations > 0))
						mPaused = mDone = true;
				}
			}
			else if (mAnimForward)
			{
				if ((mCurrentFrame -= mStepAmt) < aStartFrame)
				{
					++mLoopCount;
					mCurrentFrame = aStartFrame + 1;
					
					// EXPECTED: ((mCurrentFrame < mMaxFrames) || (mMaxFrames == 1))

					if ((mMaxFrames == 1) && mCurrentFrame == 1)
						--mCurrentFrame;

					mAnimForward = false;
					if (mStopWhenDone ||  ((mLoopCount >= mNumIterations) && (mNumIterations > 0)))
					{
						mCurrentFrame = aStartFrame;
						mPaused = true;
						mDone = true;
					}
				}
			}
		}
	}

	if (mMaxFrames == 1)
		mFrameChanged = false;
}

// EXPECTED: pEndFrame < mMaxFrames
// EXPECTED: pStartFrame >= 0
// EXPECTED: pSTartFrame < pEndFrame
void Animator::LoopSubsection(int pStartFrame, int pEndFrame)
{
	mLoopSubsection = true;
	mLoopStart = pStartFrame;
	mLoopEnd = pEndFrame;
}

void Animator::SetLoopDir(int pDir)
{
	mLoopDir = pDir;

	if (pDir < 0)
		mCurrentFrame = mMaxFrames - 1;
}

void Animator::ResetAnim(void)
{
	mPaused = false; 
	mCurrentFrame = mLoopDir >= 0 ? 0 : mMaxFrames - 1;
	mDone = false; 
	mLoopCount = 0;
	mLoopStart = 0;
	mLoopEnd = mMaxFrames;
	mLoopSubsection = false;
	mCurrentTime = 0;
}

// EXPECTED: pFrame < mFrameDelays.size()
void Animator::SetDelay(int pDelay, int pFrame)
{
	mFrameDelays[pFrame] = pDelay;
	mFrameDelay = pDelay;
}

void Animator::SetDelay(int pDelay)
{
	mFrameDelay = pDelay;
	if (mFrameDelays.size() > 0)
		for (int i = 0; i < mFrameDelays.size(); i++)
			mFrameDelays[i] = pDelay;
}


void Animator::Draw(Graphics* g, int pX, int pY) const
{
	if ((mImage == NULL) || IsPaused() || IsDone())
		return;

	if (mDrawAdditive)
	{
		g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
		g->SetColor(mAdditiveColor);
		g->SetColorizeImages(true);
	
		g->DrawImageCel(mImage, pX + mXOff, pY + mYOff, GetFrame());

		g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
		g->SetColorizeImages(false);
	}
	else
		g->DrawImageCel(mImage, pX + mXOff, pY + mYOff, GetFrame());
}

void Animator::Draw(Graphics* g, float pX, float pY, bool pSmooth) const
{
	if ((mImage == NULL) || IsPaused() || IsDone())
		return;

	if (pSmooth)
	{
		int aSX = 0;
		int aSY = 0;
		if (mImage->mNumCols > mImage->mNumRows)
		{
			aSX = GetFrame() * mImage->GetCelWidth();
			aSY = 0;
		}
		else if (mImage->mNumRows > mImage->mNumCols)
		{
			aSX = 0;
			aSY = GetFrame() * mImage->GetCelHeight();
		}
		//else this is a single frame "animation"		

		if (mDrawAdditive)
		{
			g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
			g->SetColor(mAdditiveColor);
			g->SetColorizeImages(true);

			g->DrawImageF(mImage, pX + mXOff, pY + mYOff, Rect(aSX, aSY, mImage->GetCelWidth(), mImage->GetCelHeight()));

			g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
			g->SetColorizeImages(false);
		}
		else
			g->DrawImageF(mImage, pX + mXOff, pY + mYOff, Rect(aSX, aSY, mImage->GetCelWidth(), mImage->GetCelHeight()));
	}
	else
	{	
		if (mDrawAdditive)
		{
			g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
			g->SetColor(mAdditiveColor);
			g->SetColorizeImages(true);

			g->DrawImageCel(mImage, pX + mXOff, pY + mYOff, GetFrame());

			g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
			g->SetColorizeImages(false);
		}
		else
			g->DrawImageCel(mImage, pX + mXOff, pY + mYOff, GetFrame());
	}

}

void Animator::DrawStretched(Graphics* g, float pX, float pY, float pPct) const
{
	if ((mImage == NULL) || IsPaused() || IsDone())
		return;

	float aWidth = (float)(mImage->GetCelWidth()) * pPct;
	float aHeight = (float)(mImage->GetCelHeight()) * pPct;
	Rect aDestRect = Rect(pX + mXOff, pY + mYOff, aWidth, aHeight);

	if (mDrawAdditive)
	{
		g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
		g->SetColor(mAdditiveColor);
		g->SetColorizeImages(true);
		
		g->DrawImageCel(mImage, aDestRect, GetFrame());

		g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
		g->SetColorizeImages(false);
	}
	else
		g->DrawImageCel(mImage, aDestRect, GetFrame());

}

void Animator::DrawStretched(Graphics* g, float pX, float pY, int pWidth, int pHeight) const
{
	if ((mImage == NULL) || IsPaused() || IsDone())
		return;

	Rect aDestRect = Rect(pX + mXOff, pY + mYOff, pWidth, pHeight);

	if (mDrawAdditive)
	{
		g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
		g->SetColor(mAdditiveColor);
		g->SetColorizeImages(true);

		g->DrawImageCel(mImage, aDestRect, GetFrame());

		g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
		g->SetColorizeImages(false);
	}
	else
		g->DrawImageCel(mImage, aDestRect, GetFrame());

}

void Animator::DrawRotated(Graphics* g, float pX, float pY, float pAngle, bool pSmooth, float pCenterX, float pCenterY) const
{
	if ((mImage == NULL) || IsPaused() || IsDone())
		return;

	if (pSmooth)
	{
		int aSX = 0;
		int aSY = 0;
		if (mImage->mNumCols > mImage->mNumRows)
		{
			aSX = GetFrame() * mImage->GetCelWidth();
			aSY = 0;
		}
		else if (mImage->mNumRows > mImage->mNumCols)
		{
			aSX = 0;
			aSY = GetFrame() * mImage->GetCelHeight();
		}
		//else this is a single frame "animation"

		Rect aRect = Rect(aSX, aSY, mImage->GetCelWidth(), mImage->GetCelHeight());
		g->DrawImageRotatedF(mImage, pX + mXOff, pY + mYOff, pAngle, pCenterX, pCenterY, &aRect);
	}
	else
	{
		Rect aRect = mImage->GetCelRect(GetFrame());
		g->DrawImageRotated(mImage, pX + mXOff, pY + mYOff, pAngle, pCenterX, pCenterY, &aRect);
	}
}


bool Animator::FrameChanged(void)
{
	if (mFrameChanged)
	{
		mFrameChanged = false;
		return true;
	}

	return false;
}

void Animator::DrawRandomly(bool pRandom)
{
	mDrawRandomly = pRandom;
	if (mDrawRandomly)
	{
		mRandomFrames.clear();

		for (int i = 0; i < mMaxFrames; i++)
			mRandomFrames.push_back(i);
	}
}