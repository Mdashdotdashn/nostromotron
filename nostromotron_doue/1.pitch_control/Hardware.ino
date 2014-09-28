#include "Hardware.h"
#include "IntervalTimer.h"
#include <SPI.h>

const int8_t GATE_PIN = 5;
const int8_t CUTOFF_PIN = 3;

const int8_t SPI_CS_PIN = 10;

//-----------------------------------------------------------

void SOnParameterTimer()
{
  Hardware::SInstance().onParameterUpdate();
}


//-----------------------------------------------------------

void SOnAudioTimer()
{
  Hardware::SInstance().onAudioUpdate();
}


//-----------------------------------------------------------

void Hardware::onParameterUpdate()
{
  Parameters parameters;

  configuration_.paramCB_(parameters);
  digitalWrite(GATE_PIN, parameters.gate_? HIGH :LOW);
  analogWrite(CUTOFF_PIN, parameters.cutoff_);
  pitchValue_ = parameters.pitch_;
}


//-----------------------------------------------------------

// Expects 16 bit input value

void Hardware::SetDACValue(uint8_t channel, uint16_t value, uint8_t div)
{
  const uint8_t MCP4822_NO_SHUTDOWN= 0x10;
  
  value = value >> 4; // Trim down to 12bit
  byte lowByte = value & 0xff;
  byte highByte = ((value >> 8) & 0x0f | channel << 7 |  div << 5 | MCP4822_NO_SHUTDOWN );
  
  digitalWrite(SPI_CS_PIN, LOW); // Signal beginning of transmission
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  digitalWrite(SPI_CS_PIN, HIGH); // Signal end of transmission  
}


//-----------------------------------------------------------

uint16_t val =0 ;

void Hardware::onAudioUpdate()
{
  const uint8_t MCP4822_LOW_GAIN = 1;
  const uint8_t MCP4822_HIGH_GAIN = 0;
  val+=128;
  SetDACValue(0, val, MCP4822_HIGH_GAIN);
  SetDACValue(1, pitchValue_, MCP4822_HIGH_GAIN  );
}
 
  
//-----------------------------------------------------------

static Hardware sInstance;

Hardware &Hardware::SInstance()
{
  return sInstance; 
}


//-----------------------------------------------------------

bool Hardware::Init(const Hardware::Configuration& configuration)
{
  configuration_ = configuration;

  // Define control pins as outputs
  
  pinMode(GATE_PIN, OUTPUT);
  pinMode(CUTOFF_PIN, OUTPUT);
 // pinMode(PITCH_PIN, OUTPUT);
  
  // Set the PWM rate for all pins at high rate
  // the high frequency genetated by the PWM will
  // be filtered in the analog world
  // All pins we use share the same timer so only
  // one needs to be set
  
  const unsigned int PWM_FREQUENCY = 31250;
  analogWriteFrequency(CUTOFF_PIN, PWM_FREQUENCY);	

  // SPI Bus setup to communicate with the MCP4822

  SPI.begin();  
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  pinMode(SPI_CS_PIN, OUTPUT);
  
  // Define our signal update rate way lower since we don't need
  // high range for control signals

  IntervalTimer timer0;
  timer0.begin(SOnParameterTimer, 1000000 / configuration_.paramRate_);

  // Define our audio update rate using 44100 Hrz
  
  IntervalTimer timer1;
  timer1.begin(SOnAudioTimer, 1000000 / configuration_.audioRate_);

}

//-----------------------------------------------------------

