#include "Fixed.h"

class Envelope
{
  class Segment
  {
   
    //-----------------------------------------------------------

    public:
        
      enum Mode
      {
        MODE_LINEAR,
        MODE_EXPONENTIAL,
      };
      
      Segment();
      ~Segment();
    
      void Setup(
        const fixed from,
        const fixed to,
        const uint32_t timeInTriggerStep,
        const Segment::Mode mode = MODE_LINEAR);
      
      void Trigger();
      bool IsFinished();

      inline fixed GetValue()
      {
        return current_;
      }
    
    private:
    
      bool finished_;
      fixed from_;
      fixed to_;
      fixed speed_;
      fixed current_;
      Segment::Mode mode_;
  };
  
  //-----------------------------------------------------------

  public:
    Envelope();
    ~Envelope();
    
    void Setup(
      const uint32_t attackInTriggerStep,
      const uint32_t decayInTriggerStep,
      const fixed sustain,
      const uint32_t releaseInTriggerStep);
      
    void Trigger();

    void SetScaler(fixed scaler);
    
    void Start();
    void Stop();
    
    inline fixed GetValue()
    {
      return segment_.GetValue();
    }
    
    inline fixed GetScaledValue()
    {
      return scaledValue_;
    }
  
    bool IsActive();
    
   private:
     enum Stage
     {
       STAGE_INIT = 0,
       STAGE_ATTACK,
       STAGE_DECAY,
       STAGE_SUSTAIN,
       STAGE_RELEASE,
       STAGE_FINISHED
     };
 
    enum Request
    {
      REQUEST_NONE,
      REQUEST_ATTACK,
      REQUEST_RELEASE
    };
    
  private:   
     void processRequest();
     void triggerNextStage();
     void setupStage(Envelope::Stage stage);
     bool isSegmentedStage();
     bool isStageFinished();
     void computeScaledValue();
   private:
     Envelope::Segment segment_;
     uint32_t attack_;
     uint32_t decay_;
     fixed sustain_;
     uint32_t release_;
     
     fixed scaler_;
     fixed scaledValue_;
     Envelope::Stage stage_;
     Envelope::Request request_;
};
