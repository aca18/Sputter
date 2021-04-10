#include "Sputter.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"


Sputter::Sputter(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kfreq1)->InitDouble("Rate 1", 0.5, 0.1, 60.0, 0.1, "hz", IParam::kFlagsNone, "", IParam::ShapePowCurve(1.));
  GetParam(kfreq2)->InitDouble("Rate 2", 2.0, 0.1, 60.0, 0.1, "hz", IParam::kFlagsNone, "", IParam::ShapePowCurve(1.));
  GetParam(kstrength)->InitDouble("Strength", 50, 0.0, 100, 0.1, "%");
  GetParam(kslew)->InitDouble("Slew", 0, 0, 500, 1, "ms");
  GetParam(kintensity)->InitDouble("Contrast", 0, 0, 100, 0.1, "%");
  GetParam(kmode)->InitEnum("Mode", 0, kNumModes, "", IParam::kFlagsNone, "", "Mono", "Stereo", "Pan", "M / S", "M/S Pan");

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
    const IRECT centerGrid = ctrlRect.GetGridCell(3, 1, 10, EDirection::Horizontal, 4).GetScaledAboutCentre(0.9); //center grid
    const IRECT strengthRect = centerGrid.FracRectVertical(0.8, true).GetScaledAboutCentre(0.9);
    const IRECT modeRect = centerGrid.FracRectVertical(0.1, false).GetScaledAboutCentre(0.9);
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
    pGraphics->AttachControl(new ICaptionControl(modeRect, kmode, IText(15), DEFAULT_FGCOLOR, false));
  };
#endif
}

#if IPLUG_DSP
void Sputter::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double strength = GetParam(kstrength)->Value() / 100.;
  const int nChans = NOutChansConnected();
  const int mode = GetParam(kmode)->Value();

  switch (mode)
  {
    case kMono:
    {
      for (int s = 0; s < nFrames; s++) {
        double gain = 1. - strength + mSlowRand1.Process() * strength;
        for (int c = 0; c < nChans; c++) {
          outputs[c][s] = inputs[c][s] * gain;
        }
      }
      break;
    }
    case kStereo:
    {
      for (int s = 0; s < nFrames; s++) {
        double gainL = 1. - strength + mSlowRand1.Process() * strength;
        double gainR = 1. - strength + mSlowRand2.Process() * strength;
        for (int c = 0; c < nChans; c++) {
          if (c == 0) {
            outputs[c][s] = inputs[c][s] * gainL;
          }
          else
          {
            outputs[c][s] = inputs[c][s] * gainR;
          }
        }
      }
      break;
    }
    case kPan:
    {
      for (int s = 0; s < nFrames; s++) {
        if (nChans != 2)
        {
          double gain = 1. - strength + mSlowRand1.Process() * strength;
          for (int c = 0; c < nChans; c++) {
            outputs[c][s] = inputs[c][s] * gain;
          }
        }
        else
        {
          double pan = 2.0 * strength * (0.5 - mSlowRand1.Process());
          double gainL = 0.5 * (pan - 1.); //not even-power panning
          double gainR = 0.5 * (pan + 1.);
          outputs[0][s] = gainL * inputs[0][s];
          outputs[1][s] = gainR * inputs[1][s];
        }
      }
      break;
    }
    case kMidSide:
    {
      for (int s = 0; s < nFrames; s++) {
        double g1 = 1. - strength + mSlowRand1.Process() * strength;
        double g2 = 1. - strength + mSlowRand2.Process() * strength;
        if (nChans != 2)
        {
          for (int c = 0; c < nChans; c++) {
            outputs[c][s] = inputs[c][s] * g1;
          }
        }
        else
        {
          sample mid = 0.5 * (inputs[0][s] + inputs[1][s]);
          sample side = 0.5 * (inputs[0][s] - inputs[1][s]);
          outputs[0][s] = mid * g1 + side * g2;
          outputs[1][s] = mid * g1 - side * g2;
        }
      }
      break;
      /*  Mid - Side Notes (algebra)
      -----------
       
       M = (L + R) * 0.5
       S = (L - R) * 0.5
       
       L = (M + S)
       R = (M - S)
       
       -----------
       
       L = (M*g1         + S*g2)
         = ((L+R)*0.5*g1  + (L-R)*0.5*g2)
         = 0.5*L*g1 + 0.5*R*g1 + 0.5*L*g2 - 0.5*R*g2
         = 0.5*L*(g1 + g2) + 0.5*R*(g1 - g2)
       
       
       R = (M*g1 - S*g2)
         = ((L+R)*0.5*g1 - (L-R)*0.5*g2)
         = 0.5*L*g1 + 0.5*R*g1 - 0.5*L*g2 + 0.5*R*g2
         = 0.5*L*(g1 - g2) + 0.5*R*(g1 + g2)
       
      */
    }
    case kMSPan:
    {
      for (int s = 0; s < nFrames; s++) {
        if (nChans != 2)
        {
          double gain = 1. - strength + mSlowRand1.Process() * strength;
          for (int c = 0; c < nChans; c++) {
            outputs[c][s] = inputs[c][s] * gain;
          }
        }
        else
        {
          double pan = 2.0 * strength * (0.5 - mSlowRand1.Process());
          double g1 = 0.5 * (pan - 1.); //not even-power panning
          double g2 = 0.5 * (pan + 1.);
          sample mid = 0.5 * (inputs[0][s] + inputs[1][s]);
          sample side = 0.5 * (inputs[0][s] - inputs[1][s]);
          outputs[0][s] = mid * g1 + side * g2;
          outputs[1][s] = mid * g1 - side * g2;
        }
      }
      break;
    }
  }
}

void Sputter::OnReset()
{
  mSlowRand1.SetSampleRate((int)GetSampleRate());
  mSlowRand2.SetSampleRate((int)GetSampleRate());
}

void Sputter::OnParamChange(int paramIdx)
{
  switch (paramIdx) {
    case kfreq1:
    case kfreq2:
    {
      mSlowRand1.SetRatesRange(GetParam(kfreq1)->Value(), GetParam(kfreq2)->Value());
      mSlowRand2.SetRatesRange(GetParam(kfreq1)->Value(), GetParam(kfreq2)->Value());
      break;
    }
    case kstrength:
      //mTargetAmp = GetParam(paramIdx)->Value();
      break;
    case kslew:
      mSlowRand1.SetSlew(0.001 * GetParam(paramIdx)->Value());
      mSlowRand2.SetSlew(0.001 * GetParam(paramIdx)->Value());
      break;
    case kintensity:
      mSlowRand1.SetIntensity(0.005 * GetParam(paramIdx)->Value());
      mSlowRand2.SetIntensity(0.005 * GetParam(paramIdx)->Value());
      break;
  }
}
#endif
