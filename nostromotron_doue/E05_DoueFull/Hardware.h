#pragma once
#include <stdint.h>
#include <string.h>

class Hardware
{
 public:

    class Configuration;

    //---------------------------------------------------------------------
  
    static Hardware &SInstance();

    bool Init(const Configuration& configuration);

    //---------------------------------------------------------------------

    // Parameters we have access to

    struct Parameters
    {
      bool gate_;
      uint16_t pitch_;
      uint8_t cutoff_;
      uint8_t vca_;
    };

    //---------------------------------------------------------------------

    typedef uint16_t (*AudioCallback)();
    typedef void (*ParameterCallback)(Parameters& parameters);   

    // Returns the current evaluated frequency from feedback
    
    float MeasuredVCOFrequency();

    //---------------------------------------------------------------------

    struct Configuration
    {
      AudioCallback audioCB_;
      ParameterCallback paramCB_;

      uint32_t audioRate_;
      uint32_t paramRate_;
    };

    //---------------------------------------------------------------------

  public:

    void onParameterUpdate();
    void onAudioUpdate();
     void onPls1Raised();
   
    Hardware();
    ~Hardware() {};


  private:
    void SetDACValue(uint8_t channel, uint16_t value, uint8_t gain);
    Configuration configuration_;
    uint16_t pitchValue_;
    
    
    //-----------------------------------------------------------

    size_t audioTicks_;
    size_t microsSum_;
    size_t plsTickCount_;
    float measureVCOFreq_;    
};
