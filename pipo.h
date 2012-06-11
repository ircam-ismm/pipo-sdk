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
  
public:
  int propagateStreamAttributes(int streamId, int hasTimeTags, double frameRate, double frameOffset, int cols, int rows, char **colNames, int hasVarRows, double elemStep, int maxStreamBlock)
  { 
    if(this->receiver != NULL)
      return this->receiver->streamAttributes(streamId, hasTimeTags, frameRate, frameOffset, cols, rows, colNames, hasVarRows, elemStep, maxStreamBlock);
    else
      return -1; 
  };
  
  int propagateStreamAttributesChanged(int unitId)
  {
    if(this->receiver != NULL)
      return this->receiver->propagateStreamAttributesChanged(unitId + 1);
    else
      return -1; 
  };
  
  int propagateReset(int streamId)
  {
    if(this->receiver != NULL)
      return this->receiver->reset(streamId);
    else
      return -1; 
  }
  
  int propagateFrames(int streamId, double time, float *values, int num, int stride) 
  { 
    if(this->receiver != NULL)
      return this->receiver->frames(streamId, time, values, num, stride); 
    else
      return -1;
  }

  int propagateFinalize(int streamId, double inputEnd)
  {
    if(this->receiver != NULL)
      return this->receiver->finalize(streamId, inputEnd);
    else
      return -1; 
  }
  
public:
  void setReceiver(PiPo *receiver) { this->receiver = receiver; };
  void streamAttributesChanged(void) { this->propagateStreamAttributesChanged(0); };
  virtual int streamAttributes(int streamId, int hasTimeTags, double frameRate, double frameOffset, int cols, int rows, char **colNames, int hasVarRows, double elemStep, int maxStreamBlock) { if(this->receiver != NULL) return this->propagateStreamAttributes(streamId, hasTimeTags, frameRate, frameOffset, cols, rows, colNames, hasVarRows, elemStep, maxStreamBlock); else return -1; };
  virtual int reset(int streamId) { return this->propagateReset(streamId); };
  virtual int frames(int streamId, double time, float *values, int num, int stride) { return this->propagateFrames(streamId, time, values, num, stride); };
  virtual int finalize(int streamId, double inputEnd) { return this->propagateFinalize(streamId, inputEnd); };
  
  PiPo(void) { };
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

#define PIPO_MAX_CLASS(pipoName, pipoClass) \
static t_class *pipoName = NULL; \
typedef struct Max ## pipoClass ## St { t_object o; pipoClass pipoName; } Max ## pipoClass ## T; \
static void *newMaxObject(t_symbol *s, long ac, t_atom *at) { \
  Max ## pipoClass ## T *self = (Max ## pipoClass ## T *)object_alloc(pipoName); \
  if(self != NULL) new(&self->pipoName) pipoClass(); \
  return self; } \
static void freeMaxObject(Max ## pipoClass ## T *self) { self->pipoName.~pipoClass(); } \
int main(void) { \
  pipoName = class_new("pipo." #pipoName, (method)newMaxObject, (method)freeMaxObject, (long)sizeof(Max ## pipoClass ## T), 0L, A_GIMME, 0); \
  class_register(CLASS_NOBOX, pipoName);

#define PIPO_MAX_CLASS_EXIT() } return 0

#endif
