#pragma once

#include "Oscillator.h"
#include "NoiseGen.h"
#include "Envelope.h"
#include "Hardware.h"

#include <vector>

// ===========================================================================================

class SynthController
{
 public:
 
 static inline SynthController &SInstance()
 {
   return sInstance_; 
 }
 
 bool Init();
 void Calibrate();
 
 void StartNote(uint8_t note);
 void StopNote(uint8_t note);
 
 enum Parameter {
   WAVE_SELECT,
   WAVE_LOOP_START,
   WAVE_LOOP_END,
   ANALOG_OSC_ON,
   ANALOG_PITCH_FINE_TUNE,
   WAVE_OCTAVE,
   PITCH_GLIDE,
   KEYBOARD_MODE,
   NOISE_MIX,
   ENV_ATTACK,
   ENV_DECAY,
   ENV_SUSTAIN,
   ENV_RELEASE,
   VCA_ENV_GATER,
   VCF_ENV_MOD,
   AMP_GAIN,
   LAST
 };
 
 void SetParameterValue(const SynthController::Parameter& parameter, const fixed& value);
 void SetPitchBend(const fixed& value);
 
 //------------------------
 
 fixed ProcessSample();
 void onParamUpdate(Hardware::Parameters &parameters);
 
 //------------------------
 
 private:
   // == NoteStack
   
   class NoteStack
   {
    public:
    
      NoteStack();
      
      void Add(uint8_t note);
      void Remove(uint8_t note);
      uint8_t Size();
      
      uint8_t GetTargetNote();
      
    private:
       std::vector<uint8_t> stack_;
   };
 
   // == NoteControl

   class NoteControl
   {
   public:
   
     NoteControl();
     
     void Add(uint8_t note);
     void Remove(uint8_t note);
     
     enum Mode
     {
       MONO,
       DUO,
       LAST
     };
     
     void SetMode(Mode mode);
     
     uint8_t GetAnalogTargetNote();
     uint8_t GetDigitalTargetNote();
      
     bool ShouldTriggerEnvelope();
     bool ShouldKillEnvelope(); 

   private:
   
     Mode mode_;
     NoteStack loStack_;
     NoteStack hiStack_;
     
   };
   
   // == PitchControl

   class PitchControl
   {
   public:
     PitchControl();
     
     void SetTarget(uint8_t target);
     void SetGlide(fixed glide);
     
     bool GlideEnabled();
     
     void Trigger();
     
     inline float GetValue()
     {
       return value_;
     }
     
   private:
     uint8_t target_;
     float value_;
     fixed glide_;
   };

   void updateTargetPitches();
   void updateLevels();
   
   void updatePitch();
   void updateCutoff();
 
 private:
 
  static SynthController sInstance_;
 
   Oscillator oscillator_;
   NoiseGen noiseGenerator_;
   Envelope envelope_;
   NoteControl noteControl_;
   PitchControl anaPitchControl_;
   PitchControl digiPitchControl_;
   
   float pitchSlope_;
   float pitchIntercept_;
   
   bool enableAnalog_;

   fixed noiseMix_;
   fixed gain_;
 
   uint32_t attack_;
   uint32_t decay_;
   fixed sustain_;
   uint32_t release_;
 
   fixed cutoff_;
   fixed cutoffEnvMod_;
 
   float noteBend_;
   int16_t analogPitchFineTune_;
 
  Hardware::Parameters hwParameters_;
};


