#include "Fixed.h"
#include <avr/pgmspace.h>

class Oscillator 
{
public:
  
  Oscillator();
  ~Oscillator();
  
  bool Init(uint32_t sampleRate);

  void SetNote(float note);  
  void SetShape(fixed shape);
  void SetLoopStart(fixed start);
  void SetLoopWidth(fixed width);
  void SetGain(fixed gain);
  void SetOctave(int32_t octave);

  fixed ProcessSample();

private:
  void updateLoopPoints();
  void updateOscSpeed();
  float convertToPitch(float semiTonesFromC3);
  
private:
  fixed loopStart_;   
  fixed loopWidth_;  
  fixed gain_; 

  int shape_;
  float pitch_;
  
  float oscPosition_;
  float oscSpeed_;
  const fixed *oscDataStart_;
  float oscDataLen_;
  
  int32_t octave_;
  uint32_t sampleRate_;
};
