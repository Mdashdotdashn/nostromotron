#include "Hardware.h"
#include "Frequencies.h"
#include "LinearRegression.h"

int8_t gMidiGateCount = 0;
uint8_t gMidiNoteValue = 0;
uint8_t gMidiCutOff = 0;
float gVCA = 0;
float gDecayFactor = 0.996;
int16_t gMidiPitchBend = 0;
uint16_t pitchControl = 0xFFFF;

float gSlope, gIntercept;
int LED_PIN = 13;

//const uint8_t LOWEST_KEY = 24; // C2

//-------------------------------------------------------------------------

void SOnNoteOn(byte channel, byte note, byte velocity)
{
  gMidiNoteValue = note;
  float freq = sNoteToFrequency[note];
//  float targetDAC = (1034.4928702938f - freq) / 0.015350199250741f;
  float targetDAC = freq*gSlope + gIntercept;
/*  Serial.print(note);
  Serial.print(" - ");
  Serial.print(freq, 2);
  Serial.print(" -> ");
  Serial.println(targetDAC);
*/  if (targetDAC > 0 && targetDAC < 65536)
  { 
    gMidiGateCount++;
    pitchControl = targetDAC;
    gVCA = 255.;
  }
}
 

//-------------------------------------------------------------------------

void SOnNoteOff(byte channel, byte note, byte velocity)
{
  gMidiGateCount--;
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
      float mix = float(value) / 127.f;
      gDecayFactor = 0.9 + mix * 0.097; 
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
  bool gateOn = gMidiGateCount > 0;
  digitalWrite(LED_PIN, gate
  if (!gateOn) gVCA *= gDecayFactor;
  parameters.gate_ = gateOn;
  parameters.pitch_ = pitchControl;
  parameters.cutoff_ = 255 - uint8_t(gVCA);
 }
 
 
//-------------------------------------------------------------------------

void SCalibrate()
{
  delay(3000);
  gMidiGateCount = 1;
  gVCA = 60;
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
  gMidiGateCount = 0;  
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

//  Serial.begin(57600);  

  pinMode(LED_PIN, OUTPUT);
  SCalibrate();
}

//-------------------------------------------------------------------------

float lastMeasure;

void loop() 
{
//  printCalibrationTable();
  usbMIDI.read();
}

