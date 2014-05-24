#include "Envelope.h"

//==================== Envelope::Segment ===================================

Envelope::Segment::Segment()
:finished_(true)
,from_(0)
,to_(0)
,mode_(MODE_LINEAR)
,speed_(0)
,current_(0)
{
}


//-----------------------------------------------------------

Envelope::Segment::~Segment()
{
}


//-----------------------------------------------------------

void Envelope::Segment::Setup(
  const fixed from, 
  const fixed to, 
  const uint32_t timeInTriggerStep, 
  const Segment::Mode mode)
{
  from_ = from;
  to_ = to;
  mode_ = mode;

  finished_ = ((timeInTriggerStep == 0) || (to_ == from_)) ;

  if (!finished_)
  {
    float speedFactor = 1.0f/float(timeInTriggerStep);
    speed_ = fp_mul(fp_sub(to_, from_), fl2fp(speedFactor));
  }
  current_ = finished_ ? to_ : from_;
}


//-----------------------------------------------------------

void Envelope::Segment::Trigger()
{  

  if (!finished_) 
  {
    current_ = fp_add(current_,speed_);

    if ((to_ > from_) && (current_ > to_)) 
    {
	finished_ = true;
	current_=to_;
    }

    if ((from_ > to_) && (current_ < to_)) 
    {
	finished_ = true;
	current_=to_;
    }
  } 
}


//-----------------------------------------------------------

bool Envelope::Segment::IsFinished()
{
  return finished_ ;
}


//==================== Envelope =============================

Envelope::Envelope()
 :stage_(STAGE_FINISHED)
,request_(REQUEST_NONE)
,scaler_(0)
{
}


//-----------------------------------------------------------

Envelope::~Envelope()
{
}


//-----------------------------------------------------------

void Envelope::Setup(
      const uint32_t attackInTriggerStep,
      const uint32_t decayInTriggerStep,
      const fixed sustain,
      const uint32_t releaseInTriggerStep)
{
  attack_ = attackInTriggerStep;
  decay_ = decayInTriggerStep;
  sustain_ = sustain;
  release_ = releaseInTriggerStep;
}


//-----------------------------------------------------------

bool Envelope::IsActive()
{
  return (stage_ != STAGE_FINISHED);
}

//-----------------------------------------------------------

void Envelope::triggerNextStage()
{
  Envelope::Stage nextStage = STAGE_FINISHED;
  switch(stage_)
  {
    case STAGE_INIT:
      nextStage = STAGE_ATTACK;
      break;    
      
    case STAGE_ATTACK:
      nextStage = STAGE_DECAY;
      break;    
      
    case STAGE_DECAY:
      nextStage = STAGE_SUSTAIN;
      break;
      
    case STAGE_SUSTAIN:
      nextStage = STAGE_RELEASE;
      break;    

    case STAGE_RELEASE:
      nextStage = STAGE_FINISHED;
      break;    
  }
  
  stage_ = nextStage;
  
  if (isSegmentedStage())
  {
    setupStage(stage_);
    if (isStageFinished())
    {
      triggerNextStage();
    }
  }
  
}


//-----------------------------------------------------------

bool Envelope::isSegmentedStage()
{
  return (stage_ != STAGE_FINISHED) && (stage_ != STAGE_SUSTAIN);
}

  
//-----------------------------------------------------------

bool Envelope::isStageFinished()
{
  if (isSegmentedStage())
  {
    return segment_.IsFinished();
  }
  return false;
}


//-----------------------------------------------------------

void Envelope::setupStage(Envelope::Stage stage)
{  
  // Get new segment values
  
  fixed from = segment_.GetValue();
  fixed to = 0;
  uint32_t speed =0;
  
  switch(stage)
  {
    case STAGE_ATTACK:
      to = i2fp(1);
      speed = attack_;
      break;
    case STAGE_DECAY:
      to = sustain_;
      speed = decay_;
      break;
    case STAGE_RELEASE:
      to = i2fp(0);
      speed = release_;
      break;
  }
  
  // Setup the segment
  
  segment_.Setup(from, to, speed);
}


//-----------------------------------------------------------

void Envelope::Stop()
{
  request_ = REQUEST_RELEASE;
}


//-----------------------------------------------------------

void Envelope::Start()
{
  request_ = REQUEST_ATTACK;
}


//-----------------------------------------------------------

void Envelope::processRequest()
{
  switch(request_)
  {
    case REQUEST_ATTACK:
      stage_ = STAGE_INIT;
      triggerNextStage();
      break;
      
    case REQUEST_RELEASE:
      stage_ = STAGE_SUSTAIN;
      triggerNextStage();
      break;
     
    default:
     break; 
  }
  request_ = REQUEST_NONE;
}


//-----------------------------------------------------------

void Envelope::Trigger()
{
  processRequest();
  
  if (isSegmentedStage())
  {
    segment_.Trigger();
    if (segment_.IsFinished())
    {
      triggerNextStage();
    }
  }
  
  computeScaledValue();
}


//-----------------------------------------------------------

void Envelope::SetScaler(fixed scaler)
{
  scaler_ = scaler;
}


//-----------------------------------------------------------

void Envelope::computeScaledValue()
{
  fixed value = GetValue();
  
  switch(scaler_)
  {
    case FP_ZERO:
      scaledValue_ = value;
      break;
      
    case FP_ONE:
      switch(stage_)
      {
        case STAGE_ATTACK:
        case STAGE_DECAY:
        case STAGE_SUSTAIN:
          scaledValue_=FP_ONE;
          break;
        default:
          scaledValue_=FP_ZERO;
          break;
      }
      break;

    default:
    {    
      fixed clip = fp_sub(FP_ONE, scaler_);
      if (value > clip)
      {
        value = clip;
      }
      float fValue = fp2fl(value)/fp2fl(clip);
      scaledValue_ = fl2fp(fValue);
      break;
    }  
  }
}

