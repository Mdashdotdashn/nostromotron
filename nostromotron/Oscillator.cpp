#include "Oscillator.h"
#include "OscillatorData.h"
#include <string.h>
#include <cmath>

//-----------------------------------------------------------

Oscillator::Oscillator()
:shape_(0)
{
}


//-----------------------------------------------------------

Oscillator::~Oscillator()
{
}


//-----------------------------------------------------------

bool Oscillator::Init(uint32_t sampleRate)
{
  sampleRate_ = sampleRate;
  
  pitch_ = sNoteToFrequency[48];
  loopStart_ = fl2fp(0);
  loopWidth_ = fl2fp(1.0);
  gain_ = fl2fp(1.0);
  shape_ = 2;
  octave_ = 0;
  oscPosition_ = 0;
  updateLoopPoints();
}

//-----------------------------------------------------------

static const float SHAPE_COUNT_MULT_FACTOR = oscShapeCount - 0.01f ;

void Oscillator::SetShape(fixed shape)
{
  int newShape = fp2fl(shape) * SHAPE_COUNT_MULT_FACTOR;
  if (newShape == shape_) return ;
  
  shape_ = newShape ;
  
  updateLoopPoints() ;
  
  oscPosition_=0;
}

//-----------------------------------------------------------

void Oscillator::SetLoopStart(fixed loopStart)
{
  loopStart_ = loopStart;
  updateLoopPoints();
}


//-----------------------------------------------------------

void Oscillator::SetLoopWidth(fixed loopWidth)
{
  loopWidth_ = loopWidth;
  updateLoopPoints();
}


//-----------------------------------------------------------

void Oscillator::SetGain(fixed gain)
{
  gain_ = gain;
}


//-----------------------------------------------------------

void Oscillator::SetNote(float note)
{
  float midiNotePitch = (note + 12 * octave_);
  pitch_ = convertToPitch(midiNotePitch);
  updateOscSpeed();
}


//-----------------------------------------------------------

void Oscillator::SetOctave(int32_t octave)
{
  octave_ = octave;
}

//-----------------------------------------------------------

void Oscillator::updateLoopPoints()
{
  int len=oscShapeLength[shape_] ;
  
  int o1 = len*fp2fl(loopStart_) ;
  
  fixed width = fp_sub(i2fp(1),loopStart_);
  width = fp_mul(width,loopWidth_);
  width = fp_add(loopStart_,width);
  int o2 = len*fp2fl(width);
  if (o1+o2>len) o2=len-o1 ;

  oscDataStart_=oscShapeTable[shape_]+o1 ;
  oscDataLen_=o2 ;
  
  updateOscSpeed() ;  
}


//-----------------------------------------------------------

void Oscillator::updateOscSpeed()
{
  oscSpeed_=(pitch_*oscDataLen_)/float(sampleRate_); 
}


//-----------------------------------------------------------

float Oscillator::convertToPitch(float midiNotePitch)
{
  if (midiNotePitch > 127)
  {
    midiNotePitch = 127;
  }
  if (midiNotePitch < 0)
  {
    midiNotePitch = 0;
  }
  
  uint8_t baseIndex = uint8_t(floor(midiNotePitch));
  float baseFreq = sNoteToFrequency[baseIndex];
     
  float interval = (midiNotePitch - baseIndex);
  uint16_t intervalIndex = interval * sIntervalToFreqFactorSize;
  
  return baseFreq * sIntervalToFreqFactor[intervalIndex];
   
}

//-----------------------------------------------------------

fixed Oscillator::ProcessSample()
{
  oscPosition_ += oscSpeed_;
  if (oscPosition_ > oscDataLen_) 
  {
    oscPosition_ -= oscDataLen_;
  }
  uint16_t index=uint16_t(oscPosition_);

  fixed osc;  
  memcpy_P(&osc,&oscDataStart_[index],sizeof(fixed));

  osc = fp_mul(osc, gain_);
  return osc;
}
