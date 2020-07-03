#include "Sputter.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"


Sputter::Sputter(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kfreq1)->InitDouble("Rate 1", 0.5, 0.1, 60.0, 0.1, "hz");
  GetParam(kfreq2)->InitDouble("Rate 2", 2.0, 0.1, 60.0, 0.1, "hz");
  GetParam(kstrength)->InitDouble("Strength", 50, 0.0, 100, 0.1, "%");
  GetParam(kslew)->InitDouble("Slew", 0, 0, 500, 1, "ms");
  GetParam(kintensity)->InitDouble("Contrast", 0, 0, 100, 0.1, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    //pGraphics->AttachPanelBackground(IColor::GetRandomColor ());
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    const IRECT titleRect = b.FracRectVertical(0.1, true).GetScaledAboutCentre(0.6);
    const IRECT ctrlRect = b.FracRectVertical(0.9, false).GetScaledAboutCentre(0.9);
    const IRECT leftGrid = ctrlRect.GetGridCell(0, 1, 10, EDirection::Horizontal, 3);
    const IRECT strengthRect = ctrlRect.GetGridCell(3, 1, 10, EDirection::Horizontal, 4).GetScaledAboutCentre(0.9); //center grid
    const IRECT rightGrid = ctrlRect.GetGridCell(7, 1, 10, EDirection::Horizontal, 3);
    const IRECT f1Rect = leftGrid.GetGridCell(0, 0, 2, 1).GetScaledAboutCentre(0.9); //top of left
    const IRECT f2Rect = leftGrid.GetGridCell(1, 0, 2, 1).GetScaledAboutCentre(0.9); //bottom of left
    const IRECT slewRect = rightGrid.GetGridCell(1, 0, 2, 1).GetScaledAboutCentre(0.9); //bottom of right
    const IRECT intensityRect = rightGrid.GetGridCell(0, 0, 2, 1).GetScaledAboutCentre(0.9); //top of right

    pGraphics->AttachControl(new ITextControl(titleRect, "-  S  P  U  T  T  E  R  -", IText(20)));
    pGraphics->AttachControl(new IVKnobControl(f1Rect, kfreq1));
    pGraphics->AttachControl(new IVKnobControl(f2Rect, kfreq2));
    pGraphics->AttachControl(new IVKnobControl(strengthRect, kstrength));
    pGraphics->AttachControl(new IVKnobControl(slewRect, kslew));
    pGraphics->AttachControl(new IVKnobControl(intensityRect, kintensity));
  };
#endif
}

#if IPLUG_DSP
void Sputter::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double strength = GetParam(kstrength)->Value() / 100.;
  const int nChans = NOutChansConnected();
  for (int s = 0; s < nFrames; s++) {
    double gain = 1. - strength + mSlowRand.Process() * strength;
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}

void Sputter::OnReset()
{
  mSlowRand.SetSampleRate((int)GetSampleRate());
}

void Sputter::OnParamChange(int paramIdx)
{
  switch (paramIdx) {
    case kfreq1:
    case kfreq2:
    {
      mSlowRand.SetRatesRange(GetParam(kfreq1)->Value(), GetParam(kfreq2)->Value());
      break;
    }
    case kstrength:
      //mTargetAmp = GetParam(paramIdx)->Value();
      break;
    case kslew:
      mSlowRand.SetSlew(0.001 * GetParam(paramIdx)->Value());
      break;
    case kintensity:
      mSlowRand.SetIntensity(0.005 * GetParam(paramIdx)->Value());
      break;
  }
}
#endif
