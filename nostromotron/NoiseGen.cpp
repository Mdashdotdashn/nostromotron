
#include "NoiseGen.h"

// from http://www.musicdsp.org/showone.php?id=216

NoiseGen::NoiseGen()
:x1_(0x67452301)
,x2_(0xefcdab89)
,scale_(0)
{
  SetLevel(i2fp(1));
}

static const fixed sScaleFactor = fl2fp(1.0f/fp2fl(fixed(0x7FFF)));

void NoiseGen::SetLevel(fixed level)
{
  scale_ = fp_mul(level,sScaleFactor); 
}

fixed NoiseGen::ProcessSample()
{
  x1_ ^= x2_;
  fixed value = (x2_>>16) ;
  x2_ += x1_; 
  return fp_mul(value,scale_); 
}

