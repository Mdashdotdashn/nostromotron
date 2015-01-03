#pragma once

class Trace
{
public:
  static void Init();
  static void Debug(const char *fmt, ...);
  static void DoLoop();

  static void SetFDebugVal(float f);
  static void SetIDebugVal(int i);
private:
  static float fValue_;
  static int iValue_;
  static bool dump_;
  static bool enabled_;
};
