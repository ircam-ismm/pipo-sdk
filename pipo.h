/**
 *
 * @file pipo.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Plugin Interface for Processing Objects 0.1 (experimental)
 * 
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _PIPO_H_
#define _PIPO_H_

class PiPo
{
public:
  /**
   * @brief Propagates a module's output stream attributes to its reciever.
   *
   * This method is called in the streamAttributes() method of a PiPo module.
   *
   * @param hasTimeTags   a boolean representing whether the elements of the stream are time-tagged
   * @param rate   the frame rate (highest average rate for time-tagged streams)
   * @param offset   the lag of the output stream relative to the input
   * @param width   the frame width (also number of channels or matrix columns)
   * @param size   the frame size (or number of matrix rows)
   * @param labels   optional labels for the frames' channels or columns
   * @param hasVarSize   a boolean representing whether the frames have a variable size (respecting the given frame size as maximum)
   * @param domain   extent of a frame in the given domain (e.g. duration or frequency range)
   * @param maxFrames   maximum number of frames in a block exchanged between two modules
   *
   * @return   used as return value of the streamAttributes() method
   *
   */
  int propagateStreamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, char **labels, bool hasVarSize, double domain, unsigned int maxFrames) 
  { 
    if(this->receiver != NULL)
      return this->receiver->streamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames);
    else
      return -1;
  };
  
  /**
   * @brief Propagates the reset control event.
   *
   * This method is called in the reset() method of a PiPo module.
   *
   * @return   used as return value of the reset() method
   *
   */
  int propagateReset(void) 
  {
    if(this->receiver != NULL)
      return this->receiver->reset();
    else
      return -1;
  }
  
  /**
   * @brief Propagates a module's output frames to its reciever.
   *
   * This method is called in the frames() method of a PiPo module.
   *
   * @param time   time-tag for a single frame or a block of frames
   * @param values   interleaved frames values, row by row (interleaving channels or columns), frame by frame
   * @param size   size of eaqch of all frames
   * @param num   number of frames
   *
   * @return   used as return value of the frames() method
   *
   */
  int propagateFrames(double time, float *values, unsigned int size, unsigned int num) 
  { 
    if(this->receiver != NULL)
      return this->receiver->frames(time, values, size, num); 
    else
      return -1;
  }
  
  /**
   * @brief Propagates the finalize control event.
   *
   * This method is called in the finalize() method of a PiPo module.
   *
   * @return   used as return value of the finalize() method
   *
   */
  int propagateFinalize(double inputEnd) 
  {
    if(this->receiver != NULL)
      return this->receiver->finalize(inputEnd);
    else
      return -1;
  }
  
  /**
   * @brief Sets a PiPo modules reciver (call only by the PiPo host)
   *
   * @param receiver   PiPo module receiving this module's output stream
   *
   */
  void setReceiver(PiPo *receiver) { this->receiver = receiver; };
  
  /**
   * @brief Configures a PiPo module according to the input stream attributes and propagate output stream attributes
   *
   * PiPo module:
   * Any implementation of this method requires a propagateStreamAttributes() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the final output stream attributes and usally returns 0.
   *
   * @param hasTimeTags   a boolean representing whether the elements of the stream are time-tagged
   * @param rate   the frame rate (highest average rate for time-tagged streams)
   * @param offset   the lag of the output stream relative to the input
   * @param width   the frame width (also number of channels or matrix columns)
   * @param size   the frame size (or number of matrix rows)
   * @param labels   optional labels for the frames' channels or columns
   * @param hasVarSize   a boolean representing whether the frames have a variable size (respecting the given frame size as maximum)
   * @param domain   extent of a frame in the given domain (e.g. duration or frequency range)
   * @param maxFrames   maximum number of frames in a block exchanged between two modules
   *
   * @return   0 for ok or a negative error code (to be specified), -1 for an unspecified error
   *
   */
  virtual int streamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, char **labels, bool hasVarSize, double domain, unsigned int maxFrames) 
  { 
    return this->propagateStreamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames); 
  };

  /**
   * @brief Resets processing (optional)
   *
   * PiPo module:
   * Any implementation of this method requires a propagateReset() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host usally simply returns 0.
   *
   * @return   0 for ok or a negative error code (to be specified), -1 for an unspecified error
   *
   */
  virtual int reset(void) 
  { 
    return this->propagateReset(); 
  };
  
  /**
   * @brief Processes a single frame or a block of frames
   *
   * PiPo module:
   * Any implementation of this method requires a propagateFrames() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the received frames and usally returns 0.
   *
   * @return   0 for ok or a negative error code (to be specified), -1 for an unspecified error
   *
   */
  virtual int frames(double time, float *values, unsigned int size, unsigned int num) 
  { 
    return this->propagateFrames(time, values, size, num); 
  };
  
  /**
   * @brief Finalizes processing (optinal)
   *
   * PiPo module:
   * Any implementation of this method requires a propagateFinalize() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host usally simply returns 0.
   *
   * @param inputEnd   end time of the finalized input stream
   *
   * @return   0 for ok or a negative error code (to be specified), -1 for an unspecified error
   *
   */
  virtual int finalize(double inputEnd) 
  {
    return this->propagateFinalize(inputEnd); 
  };
  
  PiPo(void) { this->receiver = NULL; };  
  ~PiPo(void) { };
  
protected:
  PiPo *receiver;

  /**
   * @brief Signals that the output stream parameters of a given module have changed
   *
   * PiPo module:
   * This method is called (with 0) in method setting a module parameter that requires changing the output stream attributes.
   *
   * PiPo host:
   * The implmentation of this method by the terminating receiver module provided by a PiPo host
   * would repropagate the input stream attributes by calling streamAttributes() of the first module.
   *
   * param unitId   (for host) index of the module that initially called streamAttributesChanged().
   * @return   0 for ok or a negative error code (to be specified), -1 for an unspecified error
   *
   */
  virtual int streamAttributesChanged(unsigned int unitId = 0) 
  { 
    if(this->receiver != NULL) 
      return this->receiver->streamAttributesChanged(unitId + 1); 
    else 
      return -1; 
  };
};

/*************************************************
 *
 *  Max/MSP support
 *
 */
#include "ext.h"
#include "ext_obex.h"
#include <new>

typedef struct MaxPiPoSt {
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
