/**
 *
 * @file pipo.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Plugin Interface for Processing Objects
 * 
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _PIPO_H_
#define _PIPO_H_

class PiPo
{
protected:
  PiPo *receiver;
  
  int propagateStreamAttributesChanged(unsigned int unitId) 
  {
    if(this->receiver != NULL) 
      return this->receiver->streamAttributesChanged(unitId + 1); 
    else 
      return -1; 
  };  
  
  virtual int streamAttributesChanged(unsigned int unitId = 0) 
  { 
    return this->propagateStreamAttributesChanged(unitId); 
  };
    
public:
  /**
   * @brief 
   * Propagate stream attributes to following PiPo module.
   *
   * @param maxOutputBlockSize maximum signal block size in frames
   * @param maxDelay maximum grain delay in frames
   * @param maxResampling maximum upsampling factor
   * @param sampleRate sampling rate (the source sample is assumed to fit this sample rate)
   *
   */
  int propagateStreamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, char **labels, unsigned int hasVarSize, double domain, unsigned int maxFrames) 
  { 
    if(this->receiver != NULL)
      return this->receiver->streamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames);
    else
      return -1; 
  };
  
  int propagateReset(void) 
  {
    if(this->receiver != NULL)
      return this->receiver->reset();
    else
      return -1; 
  }
  
  int propagateFrames(double time, float *values, unsigned int size, unsigned int num) 
  { 
    if(this->receiver != NULL)
      return this->receiver->frames(time, values, size, num); 
    else
      return -1;
  }
  
  int propagateFinalize(double inputEnd) 
  {
    if(this->receiver != NULL)
      return this->receiver->finalize(inputEnd);
    else
      return -1; 
  }
  
  void setReceiver(PiPo *receiver) { this->receiver = receiver; };
  
  virtual int streamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, char **labels, unsigned int hasVarSize, double domain, unsigned int maxFrames) 
  { 
    return this->propagateStreamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames); 
  };

  virtual int reset(void) 
  { 
    return this->propagateReset(); 
  };
  
  virtual int frames(double time, float *values, unsigned int size, unsigned int num) 
  { 
    return this->propagateFrames(time, values, size, num); 
  };
  
  virtual int finalize(double inputEnd) 
  {
    return this->propagateFinalize(inputEnd); 
  };
  
  PiPo(void) { this->receiver = NULL; };
  ~PiPo(void) { };
};

/*************************************************
 *
 *  Max/MSP class
 *
 */
#include "ext.h"
#include "ext_obex.h"
#include <new>

typedef struct MaxPiPoSt
{
  t_object head;
  PiPo pipo;
} MaxPiPoT;

#define atom_isnum(a) ((a)->a_type == A_LONG || (a)->a_type == A_FLOAT)
#define atom_issym(a) ((a)->a_type == A_SYM)
#define mysneg(s) ((s)->s_name)

#define PIPO_MAX_CLASS(pipoClassName, pipoClass, pipoName) \
static t_class *pipoClassName = NULL; \
typedef struct Max ## pipoClass ## St { t_object o; pipoClass pipoClassName; } Max ## pipoClass ## T; \
static void *newMaxObject(t_symbol *s, long ac, t_atom *at) { \
  Max ## pipoClass ## T *self = (Max ## pipoClass ## T *)object_alloc(pipoClassName); \
  if(self != NULL) new(&self->pipoClassName) pipoClass(); \
  return self; } \
static void freeMaxObject(Max ## pipoClass ## T *self) { self->pipoClassName.~pipoClass(); } \
int main(void) { \
  pipoClassName = class_new("pipo." pipoName, (method)newMaxObject, (method)freeMaxObject, (long)sizeof(Max ## pipoClass ## T), 0L, A_GIMME, 0); \
  class_register(CLASS_NOBOX, pipoClassName);

#define PIPO_MAX_CLASS_EXIT() } return 0

#endif
