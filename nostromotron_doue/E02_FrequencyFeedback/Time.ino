static unsigned int lastMicros;

void SInitIntervalEvaluator()
{  
  lastMicros = micros();
}

unsigned int SIntervalInMicroSecs()
{
  unsigned int micro = micros();
  unsigned int mDelta = micro - lastMicros;
  if (lastMicros > micro)
  {
    Serial.print("iverted");
    mDelta = 429497296 - lastMicros + micro;
  }
  lastMicros = micro;
  return mDelta;
}

