#pragma once
#include <stdint.h>
#include <vector>

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
    };

    //---------------------------------------------------------------------

    typedef uint16_t (*AudioCallback)();
    typedef void (*ParameterCallback)(Parameters& parameters);   

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
    
    Hardware() {};
    ~Hardware() {};


  private:
    void SetDACValue(uint8_t channel, uint16_t value, uint8_t gain);
    Configuration configuration_;
    uint16_t pitchValue_;
};
