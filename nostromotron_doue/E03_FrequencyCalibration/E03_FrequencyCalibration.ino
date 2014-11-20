#include "Hardware.h"
//#include "IntervalTimer.h"

bool gMidiGateOn = false;
uint8_t gMidiNoteValue = 0;
uint8_t gMidiCutOff = 127;
int16_t gMidiPitchBend = 0;
uint16_t pitchControl = 0xFFFF;
//const uint8_t LOWEST_KEY = 24; // C2

//-------------------------------------------------------------------------

void SOnNoteOn(byte channel, byte note, byte velocity)
{
  gMidiNoteValue = note;
  float freq = sNoteToFrequency[note];
  float targetDAC = (1061.3065592413f - freq) / 0.015598034068237f 
  Serial.print(note);
  Serial.print(" - ");
  Serial.print(freq, 2);
  Serial.print(" -> ");
  Serial.println(targetDAC);
  if ((targetDAC > 0) && targetDAC < 65536))
  { 
    gMidiGateOn = true;
    pitchControl = targetDAC;
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
      pitchControl = (127-value) << 9;
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
  parameters.pitch_ = pitchControl;
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

  Serial.begin(57600);  

  // wait for go
  delay(10000);
}

//-------------------------------------------------------------------------

size_t i = 0;
float lastFreq = 0;

void printCalibrationTable()
{
  if (i < 0x10)
  {
    pitchControl = i << 12;
    if (i>0)
    {
      pitchControl |= 0xFFF;
    }
    Serial.print(pitchControl);
    delay(2000);
    float freq = Hardware::SInstance().MeasuredVCOFrequency();
    Serial.print(",");
    Serial.println(freq, 2);
    lastFreq = freq;
    i+=1;
  }
}


//-------------------------------------------------------------------------

void loop() 
{
//  printCalibrationTable();
  usbMIDI.read();
}

