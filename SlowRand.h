#pragma once
/*
 ==============================================================================

Andrew C. Appelbaum (ACA) - July 2020

Generates a modulation signal based on upper and lower bound frequencies
Signal ramps between random values at slew rate
Random target value that the signal moves towards is reset when one of two
  counters resets.
Two counters start at user-defined frequencies, and on phase reset, use the
  current output value to determine a new frequency, using the user-defined
  frequencies as max and min on the frequency range

Output is between 0.0 - 1.0

 ==============================================================================
*/

#include "IPlugPlatform.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <ctime>

class SlowRand
{
public:
  SlowRand()
  {
    mSampleRate = 44100;
    std::srand(std::time(0));  //seed c++ random number generator
    mValue = fRand();
    mNextValue = fRand();
    mSlew = 0.1;
    mIntensity = 0.;
    SetRatesRange(3.1, 1.3);
    SetNext();
  }

  double fRand()
  {
    return (double)std::rand() / RAND_MAX;
  }

  void SetSampleRate(int sampleRate)
  {
    mSampleRate = sampleRate;
  }

  void SetSlew(double newslew) //newslew in seconds
  {
    mSlew = std::max(0., newslew);
  }

  void SetIntensity(double newintense)
  {
    mIntensity = std::max(0., std::min(0.5, newintense));
  }

  void SetRatesRange(double r1, double r2) //r1 r2 in Hz
  {
    mT1 = floor(abs(mSampleRate / r1)); //period in samples
    mT2 = floor(abs(mSampleRate / r2)); //period in samples
    mTmin = std::min(mT1, mT2);
    mTmax = std::max(mT1, mT2);
    mCount1 = 0;
    mCount2 = 0;
  }

  void SetNext()
  {
    mNextValue = fRand();
    if (mNextValue > 1. - mIntensity)
    {
      mNextValue = 1.0;
    }
    else if (mNextValue < mIntensity)
    {
      mNextValue = 0.0;
    }
    mIncr = (mNextValue - mValue) / fmax(1., mSampleRate * mSlew);
  }

  double Process()
  {
    mCount1++;
    mCount2++;
    if (mCount1 > mT1)
    {
      mCount1 = 0;
      mT1 = mTmin + (1. - mValue) * (mTmax - mTmin);
      SetNext();
    }
    else if (mCount2 > mT2)
    {
      mCount2 = 0;
      mT2 = mTmin + mValue * (mTmax - mTmin);
      SetNext();
    }

    if (mValue < mNextValue)
    {
      if (mIncr < 0)
      {
        mValue = mNextValue;
      }
      else //mIncr >= 0
      {
        mValue += mIncr;
      }
    }
    else  //mValue >= mNextValue
    {
      if (mIncr > 0)
      {
        mValue = mNextValue;
      }
      else //mIncr <= 0
      {
        mValue += mIncr;
      }
    }
    return mValue;
  }

protected:
  int mT1;
  int mT2;
  int mTmin;
  int mTmax;
  double mSlew = 0.1;
  double mIntensity = 0.0;
  int mCount1 = 0;
  int mCount2 = 0;
  int mSampleRate = 44100;
  double mValue = 0.;
  double mNextValue = 0.;
  double mIncr = 0.;
};
