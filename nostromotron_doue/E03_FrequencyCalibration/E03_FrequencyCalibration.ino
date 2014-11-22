#include "Hardware.h"
#include "Frequencies.h"
#include "LinearRegression.h"

bool gMidiGateOn = false;
uint8_t gMidiNoteValue = 0;
uint8_t gMidiCutOff = 127;
int16_t gMidiPitchBend = 0;
uint16_t pitchControl = 0xFFFF;

float gSlope, gIntercept;
//const uint8_t LOWEST_KEY = 24; // C2

//-------------------------------------------------------------------------

void SOnNoteOn(byte channel, byte note, byte velocity)
{
  gMidiNoteValue = note;
  float freq = sNoteToFrequency[note];
//  float targetDAC = (1034.4928702938f - freq) / 0.015350199250741f;
  float targetDAC = freq*gSlope + gIntercept;
  Serial.print(note);
  Serial.print(" - ");
  Serial.print(freq, 2);
  Serial.print(" -> ");
  Serial.println(targetDAC);
  if (targetDAC > 0 && targetDAC < 65536)
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

void SCalibrate()
{
  delay(3000);
  gMidiGateOn = true;
  Serial.print("Calibrating...");

  size_t size = 4;
  float cv[size];
  float pitch[size];
  char *progress="FEDCBA9876543210";
  
  for (size_t i = 0; i < size; i++)
  {
    pitchControl = i << 14;
    pitchControl += 0x3FFF;
    Serial.print(progress[i%16]);
    delay(2000);
    
    cv[i] = pitchControl;
    pitch[i] = Hardware::SInstance().MeasuredVCOFrequency();
  }
  Serial.println("Done");
  gMidiGateOn = false;  
  float slope, intercept;
  
  SLinearRegression(pitch, cv, size, &gSlope, &gIntercept);
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

  SCalibrate();
}

//-------------------------------------------------------------------------

float lastMeasure;

void loop() 
{
//  printCalibrationTable();
  usbMIDI.read();
  if (gMidiGateOn)
  {
    float freq = Hardware::SInstance().MeasuredVCOFrequency();
    if (freq != lastMeasure)
    {
      Serial.print("Measured freq: ");
      Serial.println(freq);
      lastMeasure = freq;
    }
  }
}

