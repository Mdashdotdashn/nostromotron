
#pragma once

#include "Fixed.h"

class NoiseGen
{
public:
  NoiseGen();
  void SetLevel(fixed level);
  fixed ProcessSample();
private:
  float scale_;
  int x1_;
  int x2_;
};
