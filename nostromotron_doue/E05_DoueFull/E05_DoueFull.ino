#include "SynthController.h"
#include "Debug.h"
#include "Arduino.h"
//#define USE_SERIAL

//-------------------------------------------------------------------------

void SOnNoteOn(byte channel, byte note, byte velocity)
{
  SynthController::SInstance().StartNote(note);
}
 

//-------------------------------------------------------------------------

void SOnNoteOff(byte channel, byte note, byte velocity)
{
  SynthController::SInstance().StopNote(note);
}

 
//-------------------------------------------------------------------------

void SOnControlChange(byte channel, byte control, byte value)
{
  SynthController::Parameter parameter = SynthController::LAST;
  
  switch (control)
  {
    
    case 11:
      parameter = SynthController::WAVE_SELECT;
      break;
    case 12:
      parameter = SynthController::WAVE_LOOP_START;
      break;
    case 13:
      parameter = SynthController::WAVE_LOOP_END;
      break;
    case 14:
      parameter = SynthController::WAVE_OCTAVE;
      break;
      
    case 15:
      parameter = SynthController::ANALOG_OSC_ON;
      break;
    case 16:
      parameter = SynthController::ANALOG_PITCH_FINE_TUNE;
      break;
    case 17:
      parameter = SynthController::PITCH_GLIDE;
      break;
    case 18:
      parameter = SynthController::KEYBOARD_MODE;
      break;
    case 19:
      parameter = SynthController::NOISE_MIX;
      break;

    case 20:
      parameter = SynthController::ENV_ATTACK;
      break;
    case 21:
      parameter = SynthController::ENV_DECAY;
      break;
    case 22:
      parameter = SynthController::ENV_SUSTAIN;
      break;
    case 23:
      parameter = SynthController::ENV_RELEASE;
      break;
      
    case 30:
      parameter = SynthController::AMP_GAIN;
      break;
    case 31:
      parameter = SynthController::VCA_ENV_GATER;
      break;
      
    case 40:
      parameter = SynthController::VCF_ENV_MOD;
      break;
  }
  if (parameter != SynthController::LAST)
  {
    fixed fpvalue = fl2fp(float(value)/127.0f);
    SynthController::SInstance().SetParameterValue(parameter, fpvalue);
  }
}


//-------------------------------------------------------------------------

void SOnPitchChange(byte channel, int pitch)
{
  SynthController::SInstance().SetPitchBend(fl2fp((pitch-8192.0f)/8192.0f));
}


//-------------------------------------------------------------------------

void setParameterDefault(const SynthController::Parameter& parameter, unsigned int value)
{
  SynthController::SInstance().SetParameterValue(parameter, fl2fp(float(value)/127.0));
}

void installDefaultSound()
{
  setParameterDefault(SynthController::WAVE_SELECT, 0);
  setParameterDefault(SynthController::WAVE_LOOP_START, 0);
  setParameterDefault(SynthController::WAVE_LOOP_END, 127);
  setParameterDefault(SynthController::WAVE_OCTAVE, 0);
  setParameterDefault(SynthController::ANALOG_OSC_ON, 127);
  setParameterDefault(SynthController::ANALOG_PITCH_FINE_TUNE, 64);
  setParameterDefault(SynthController::PITCH_GLIDE, 0);
  setParameterDefault(SynthController::KEYBOARD_MODE, 0);
  setParameterDefault(SynthController::NOISE_MIX, 8);
  setParameterDefault(SynthController::ENV_ATTACK, 0);
  setParameterDefault(SynthController::ENV_DECAY, 11);
  setParameterDefault(SynthController::ENV_SUSTAIN, 0);
  setParameterDefault(SynthController::ENV_RELEASE, 12);
  setParameterDefault(SynthController::VCA_ENV_GATER, 63);
  setParameterDefault(SynthController::VCF_ENV_MOD, 127);
}

//-------------------------------------------------------------------------

void setup()
{ 

  SynthController::SInstance().Init();
  
  installDefaultSound();
  
  // Initialise midi input callbacks

#if defined(USE_SERIAL)
  Trace::Init();
#else
  usbMIDI.setHandleNoteOff(SOnNoteOff);
  usbMIDI.setHandleNoteOn(SOnNoteOn) ; 
  usbMIDI.setHandleControlChange(SOnControlChange) ; 
  usbMIDI.setHandlePitchChange(SOnPitchChange);
#endif
}


//-------------------------------------------------------------------------

extern float fDebugVal ;
extern int iDebugVal;
byte value =0;

void loop() 
{
#if defined(USE_SERIAL) 
  static bool started = false;
  
  if (Serial.available() > 0 && (!started)) 
  {  
    Trace::Debug("Starting note 48");
    SynthController::SInstance().StartNote(48);
    started = true;
    
    SOnPitchChange(0, 10000);
}

  Trace::DoLoop();
#else
  usbMIDI.read();
#endif
}

