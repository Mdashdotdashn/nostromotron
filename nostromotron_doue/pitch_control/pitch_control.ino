#include "Hardware.h"
//#include "IntervalTimer.h"

bool gMidiGateOn = false;
uint8_t gMidiNoteValue = 0;
uint8_t gMidiCutOff = 127;
uint8_t gMidiCtrl2 = 0;
int16_t gMidiPitchBend = 0;

const uint8_t LOWEST_KEY = 24; // C2

//-------------------------------------------------------------------------

void SOnNoteOn(byte channel, byte note, byte velocity)
{
  if (note >= LOWEST_KEY)
  {
    gMidiGateOn = true;
    gMidiNoteValue = note;
  }
}
 

//-------------------------------------------------------------------------

void SOnNoteOff(byte channel, byte note, byte velocity)
{
  gMidiGateOn = false;
}
 
//-------------------------------------------------------------------------

void SOnControlChange(byte channel, byte control, byte value)
{
  switch (control)
  {
    case 1:
      gMidiCutOff = value;
      break;
    case 2:
      gMidiCtrl2 = value;
      break;
  }
}


//-------------------------------------------------------------------------

void SOnPitchChange(byte channel, int pitch)
{
  gMidiPitchBend = pitch-8192;
}


//-------------------------------------------------------------------------

uint16_t onSampleUpdate()
{
  return 0;
}


//-------------------------------------------------------------------------

void onParamUpdate(Hardware::Parameters& parameters)
{
  parameters.gate_ = gMidiGateOn;
  parameters.pitch_ = uint16_t(gMidiCtrl2 << (1+8));  // 8191/12
  parameters.cutoff_ = (gMidiCutOff<<1);
 }


//-------------------------------------------------------------------------

void setup()
{ 
  // Initialise hardware

  Hardware::Configuration configuration;

  configuration.audioRate_ = 44100;
  configuration.paramRate_ = 800;

  configuration.audioCB_ = onSampleUpdate;
  configuration.paramCB_ = onParamUpdate;

  bool status = Hardware::SInstance().Init(configuration);
  
  // Initialise midi input callbacks

  usbMIDI.setHandleNoteOff(SOnNoteOff);
  usbMIDI.setHandleNoteOn(SOnNoteOn) ; 
  usbMIDI.setHandleControlChange(SOnControlChange) ; 
  usbMIDI.setHandlePitchChange(SOnPitchChange);
}


//-------------------------------------------------------------------------

void loop() 
{
  usbMIDI.read();
}

