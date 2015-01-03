#include "Debug.h"
#include <stdarg.h>

#include "Arduino.h"

float Trace::fValue_ = -1;
int Trace::iValue_ = -1;
bool Trace::dump_ = false;
bool Trace::enabled_ = false;
//-------------------------------------------------------

void Trace::Init()
{
  enabled_ = true;
}


//-------------------------------------------------------

void Trace::SetFDebugVal(float f)
{
  if (f != fValue_)
  {
    fValue_ = f;
    dump_ = true;
  }
}


//-------------------------------------------------------

void Trace::SetIDebugVal(int i)
{
  iValue_ = i;
  dump_ = true;
}


//-------------------------------------------------------

void Trace::DoLoop()
{
  if (dump_ && enabled_)
  {
    Serial.print("i[");
    Serial.print(iValue_);
    Serial.print("] - f[");
    Serial.print(fValue_);
    Serial.println("]");    
    dump_ = false;
  }
}


//-------------------------------------------------------

void Trace::Debug(const char *fmt, ...)
{
  if (!enabled_) return;
  
  char buffer[4096] ;
  
  va_list args; 
  va_start(args,fmt);  
  vsprintf(buffer,fmt,args ); 
  va_end(args);
  
  Serial.println(buffer);
}




