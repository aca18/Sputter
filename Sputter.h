#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "SlowRand.h"

const int kNumPresets = 1;

enum EParams
{
  kfreq1 = 0,
  kfreq2,
  kstrength,
  kslew,
  kintensity,
  kmode,
  kNumParams
};

enum EStereoModes
{
  kMono = 0,
  kStereo,
  kPan,
  kMidSide,
  kMSPan,
  kNumModes
};

using namespace iplug;
using namespace igraphics;

class Sputter final : public Plugin
{
public:
  Sputter(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
private:
  SlowRand mSlowRand1;
  SlowRand mSlowRand2;
#endif
};
