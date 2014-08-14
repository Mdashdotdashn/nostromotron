#include "SynthController.h"
#include <algorithm>

void std::__throw_length_error(char const*) {};

static const uint8_t SET_NOTE_OFF = 0xFF;
static const fixed SET_GLIDE_OFF = 0;
  
//-----------------------------------------------------------

uint16_t onSampleUpdateCB()
{
  fixed value = SynthController::SInstance().ProcessSample();
  return (int32_t(value<<(15-FIXED_SHIFT)) + 0x8000);    
}


//-----------------------------------------------------------

void onParamUpdateCB(Hardware::Parameters& parameters)
{
  SynthController::SInstance().onParamUpdate(parameters);
}

//==================SynthController::NoteStack ================================================

SynthController::NoteStack::NoteStack()
{
}


//-----------------------------------------------------------

void SynthController::NoteStack::Add(uint8_t note)
{
  Remove(note);
  stack_.push_back(note);
}


//-----------------------------------------------------------

void SynthController::NoteStack::Remove(uint8_t note)
{
  std::vector<uint8_t>::iterator it = find(stack_.begin(), stack_.end(), note);
  if (it != stack_.end())
  {
    stack_.erase(it);
  }
}


//-----------------------------------------------------------

uint8_t SynthController::NoteStack::Size()
{
  return stack_.size();
}


//-----------------------------------------------------------  

uint8_t SynthController::NoteStack::GetTargetNote()
{
  return stack_.size() > 0 ? stack_.back() : SET_NOTE_OFF;  
}

//==================SynthController::NoteControl ==========================================

SynthController::NoteControl::NoteControl()
: mode_(MONO)
{
}


//-----------------------------------------------------------

void SynthController::NoteControl::Add(uint8_t note)
{
  switch(mode_)
  {
    case MONO:
      loStack_.Add(note);
      hiStack_.Add(note);
      break;
      
    case DUO:
    {
      uint8_t loTarget = loStack_.GetTargetNote();
      uint8_t hiTarget = hiStack_.GetTargetNote();
      
      if ((hiTarget == SET_NOTE_OFF) && (loTarget == SET_NOTE_OFF))
      {
        loStack_.Add(note);
        hiStack_.Add(note);
      }
      else
      {
        if (note < loTarget)
        {
          loStack_.Add(note);
        }
        else if (note > hiTarget)
        {
          hiStack_.Add(note);
        }
        else
        {
          if ((note - loTarget) < (hiTarget - note))
          {
            loStack_.Add(note);
          }
          else
          {
            hiStack_.Add(note);
          }
        }
      }      
      break;
    }
  }
}


//-----------------------------------------------------------

void SynthController::NoteControl::Remove(uint8_t note)
{
  loStack_.Remove(note);
  hiStack_.Remove(note);
  
  const int loStackSize = loStack_.Size();
  const int hiStackSize = hiStack_.Size();
  if (loStackSize == 0)
  {
    if (hiStackSize != 0)
    {
      loStack_.Add(hiStack_.GetTargetNote());
    }
  }
  else
  {
    if (hiStackSize == 0)
    {
      hiStack_.Add(loStack_.GetTargetNote());
    }
  }
}


//-----------------------------------------------------------

uint8_t SynthController::NoteControl::GetAnalogTargetNote()
{
  return hiStack_.GetTargetNote();
}


//-----------------------------------------------------------

uint8_t SynthController::NoteControl::GetDigitalTargetNote()
{
  return loStack_.GetTargetNote();
}


//-----------------------------------------------------------

bool SynthController::NoteControl::ShouldTriggerEnvelope()
{
  return (loStack_.Size() == 1) && (hiStack_.Size() == 1);
}


//-----------------------------------------------------------

bool SynthController::NoteControl::ShouldKillEnvelope()
{
  return (loStack_.Size() == 0) && (hiStack_.Size() == 0);  
}


//-----------------------------------------------------------

void SynthController::NoteControl::SetMode(Mode mode)
{
  mode_ = mode;
}

//==================SynthController::PitchControl ==========================================

SynthController::PitchControl::PitchControl()
: target_(SET_NOTE_OFF)
, value_(SET_NOTE_OFF)
, glide_(SET_GLIDE_OFF)
{
}


//-----------------------------------------------------------

void SynthController::PitchControl::SetTarget(uint8_t target)
{
  target_ = target;
}


//-----------------------------------------------------------

void SynthController::PitchControl::SetGlide(fixed glide)
{
  glide_ = glide;
}


//-----------------------------------------------------------

bool SynthController::PitchControl::GlideEnabled()
{
  return glide_ != SET_GLIDE_OFF;
}

//-----------------------------------------------------------

void SynthController::PitchControl::Trigger()
{
   if (target_ != SET_NOTE_OFF)
  {
    if ((glide_ != SET_GLIDE_OFF) 
      && (value_ != target_)
      && (value_ != SET_NOTE_OFF))
    {
      float offset = fp2fl(glide_);
      if (target_ > value_)
      {
        value_ += offset;
        if (value_ >= target_)
        {
          value_ = target_;
        }
      }
      else
      {
        value_ -= offset;
        if (value_ <= target_)
        {
          value_ = target_;
        }
      }
    }
    else
    {
      value_ = target_;
    }
  }
  else
  {
    value_ = SET_NOTE_OFF;
  }  
}

//====================== SynthController ===================================================

SynthController SynthController::sInstance_;

//-----------------------------------------------------------

bool SynthController::Init()
{
  // Initialise members
  
  hwParameters_.gate_ = false;
  hwParameters_.cutoff_ = 0x00;
  hwParameters_.pitch_ = 0;
      
  attack_ = 5;
  decay_ = 600;
  sustain_ = FP_ONE;
  release_ = 90;
  
  cutoff_ = FP_ONE;
  cutoffEnvMod_ = FP_ZERO;

  gain_ = FP_ONE;
  noiseMix_ = FP_ZERO;
  updateLevels();
  
  noteBend_ = 0;
  analogPitchFineTune_ =0;
  
  enableAnalog_ = true;
  
  envelope_.Setup(attack_, decay_, sustain_, release_);
  
  // Initialise hardware

  Hardware::Configuration configuration;

  configuration.audioRate_ = 44100;
  configuration.paramRate_ = 800;

  configuration.audioCB_ = onSampleUpdateCB;
  configuration.paramCB_ = onParamUpdateCB;

  bool status = Hardware::SInstance().Init(configuration); 
  
  // Initialise engine
  
  return oscillator_.Init(configuration.audioRate_);
}


//-----------------------------------------------------------

void SynthController::StartNote(uint8_t note)
{
  noteControl_.Add(note);
  updateTargetPitches();  

  if ((noteControl_.ShouldTriggerEnvelope()) || ! digiPitchControl_.GlideEnabled() )
  {
    envelope_.Start();
  }
}


//-----------------------------------------------------------

void SynthController::StopNote(uint8_t note)
{
  noteControl_.Remove(note);
  updateTargetPitches();
  
  if (noteControl_.ShouldKillEnvelope())
  {
      envelope_.Stop();
  }
}

void SynthController::updateTargetPitches()
{  
  digiPitchControl_.SetTarget(noteControl_.GetDigitalTargetNote());
  anaPitchControl_.SetTarget(noteControl_.GetAnalogTargetNote()); 
}

static const uint32_t MAX_ENVELOPE_TICK = 2000;
static const fixed ENVELOPE_SCALE = fl2fp(0.007874015748031f);

//-----------------------------------------------------------

void SynthController::SetParameterValue(const SynthController::Parameter& parameter, const fixed& value)
{
  bool envSetup = false;
  
  switch(parameter)
  {
    case WAVE_SELECT:
      oscillator_.SetShape(value);
      break;

    case WAVE_LOOP_START:
      oscillator_.SetLoopStart(value);
      break;

    case WAVE_LOOP_END:
      oscillator_.SetLoopWidth(value);
      break;

    case WAVE_OCTAVE:
    {
      int32_t octave = 0;
      
      if (value < fl2fp(0.33f))
      {
        octave = -1;
      }
      
      if (value > fl2fp(0.66f))
      {
        octave = 1;
      }
      oscillator_.SetOctave(octave);
      break;
    }
      
    case ANALOG_OSC_ON:
      enableAnalog_ = value > fl2fp(0.5f);
      break;
      
    case ANALOG_PITCH_FINE_TUNE:
      analogPitchFineTune_ = fp2fl(value)*256 - 128 ;
      break;

    case PITCH_GLIDE:
    {
      const fixed GLIDE_MULTIPLIER = fl2fp(0.4f);
      
      fixed glideFactor = (value != 0 ) 
        ? fp_sub(GLIDE_MULTIPLIER, fp_mul(value, GLIDE_MULTIPLIER))
        : SET_GLIDE_OFF;
      anaPitchControl_.SetGlide(glideFactor);
      digiPitchControl_.SetGlide(glideFactor);
      break;
    }
    
    case NOISE_MIX:
      noiseMix_ = value;
      updateLevels();
      break;
            
    case KEYBOARD_MODE:
    {
      NoteControl::Mode mode = (NoteControl::Mode)int(fp2fl(value)*NoteControl::LAST*0.999f);
      noteControl_.SetMode(mode);
      break;
    }
    
    case ENV_ATTACK:
      attack_ = MAX_ENVELOPE_TICK*fp2fl(value);
      envSetup = true;
      break;
      
    case ENV_DECAY:
      {
        fixed scaledDecay = fp_add(value, ENVELOPE_SCALE);
        decay_ = MAX_ENVELOPE_TICK*fp2fl(scaledDecay);
        envSetup = true;
      }
      break;
      
    case ENV_SUSTAIN:
      {
        fixed scaledSustain = fp_add(value, ENVELOPE_SCALE);
        if (scaledSustain > FP_ONE)
        {
          scaledSustain = FP_ONE;
        }
        sustain_ = scaledSustain;
        envSetup = true;
      }
      break;
      
    case ENV_RELEASE:
      release_ = MAX_ENVELOPE_TICK*fp2fl(value);
      envSetup = true;
      break;
      
    case VCA_ENV_GATER:
      envelope_.SetScaler(value);
      break;

    case VCF_ENV_MOD:
      cutoffEnvMod_ = value;
      break;

    case AMP_GAIN:
      gain_ = value;
      updateLevels();
      break;
      
  }
  
  if (envSetup)
  {
    envelope_.Setup(attack_, decay_, sustain_, release_);
  }
}


//-----------------------------------------------------------

void SynthController::SetPitchBend(const fixed& value)
{
  static const uint8_t PITCH_BEND_SEMITONE_INTERVAL = 5.0f;
  
  noteBend_ = fp2fl(value)*PITCH_BEND_SEMITONE_INTERVAL;
}

//-----------------------------------------------------------

void SynthController::updateLevels()
{
  noiseGenerator_.SetLevel(fp_mul(gain_,noiseMix_));
  oscillator_.SetGain(fp_mul(gain_,fp_sub(FP_ONE,noiseMix_)));
}

//-----------------------------------------------------------


void SynthController::updateCutoff()
{  
  fixed envValue = envelope_.GetValue();
    
  fixed envContrib = fp_sub(FP_ONE, envValue);
  envContrib = fp_mul(envContrib, cutoffEnvMod_);
  envContrib = fp_sub(FP_ONE, envContrib);
    
  fixed current = fp_mul(cutoff_, envContrib);
  hwParameters_.cutoff_ = int(fp2fl(current)*255);
}


//-----------------------------------------------------------

void SynthController::updatePitch()
{
  
  // Update digital pitch
  
  float digiNote = digiPitchControl_.GetValue();
  if (digiNote != SET_NOTE_OFF)
  {  
    oscillator_.SetNote(digiNote + noteBend_);
  }
  
  // Update analog pitch
  
  float anaNote = anaPitchControl_.GetValue();
  if (anaNote != SET_NOTE_OFF)
  {  
    static const uint8_t LOWEST_HW_NOTE = 24;
    const float noteValue = anaNote + noteBend_; 

    if (noteValue >= LOWEST_HW_NOTE)
    {
      hwParameters_.pitch_ = uint16_t((noteValue-LOWEST_HW_NOTE)*844.023333333266f + analogPitchFineTune_);
      hwParameters_.gate_ = enableAnalog_;
    }
  } 
  else
  {
    hwParameters_.gate_ = false;
  }
}


//-----------------------------------------------------------

void SynthController::onParamUpdate(Hardware::Parameters &parameters)
{
  envelope_.Trigger();
  anaPitchControl_.Trigger();  
  digiPitchControl_.Trigger();  
  updatePitch();
  updateCutoff();
  parameters = hwParameters_;
}


//-----------------------------------------------------------

fixed SynthController::ProcessSample()
{
  const fixed noise = noiseGenerator_.ProcessSample();
  const fixed osc = oscillator_.ProcessSample();
  const fixed value=fp_add(noise,osc);
  const fixed envValue =envelope_.GetScaledValue();
  return fp_mul(value, envValue);
}


