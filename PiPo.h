/**
 *
 * @file PiPo.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Plugin Interface for Processing Objects
 * 
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _PIPO_
#define _PIPO_

#include <string>
#include "PiPoAttrs.h"

class PiPo
{
public:
  PiPo(PiPo *receiver = NULL) : attrs() { this->receiver = receiver; };  
  ~PiPo(void) { destroyAttrs(); };
  
  /**
   * @brief Propagates a module's output stream attributes to its reciever.
   *
   * This method is called in the streamAttributes() method of a PiPo module.
   *
   * @param hasTimeTags a boolean representing whether the elements of the stream are time-tagged
   * @param rate the frame rate (highest average rate for time-tagged streams)
   * @param offset the lag of the output stream relative to the input
   * @param width the frame width (also number of channels or matrix columns)
   * @param size the frame size (or number of matrix rows)
   * @param labels optional labels for the frames' channels or columns
   * @param hasVarSize a boolean representing whether the frames have a variable size (respecting the given frame size as maximum)
   * @param domain extent of a frame in the given domain (e.g. duration or frequency range)
   * @param maxFrames maximum number of frames in a block exchanged between two modules
   *
   * @return used as return value of the streamAttributes() method
   *
   */
  int propagateStreamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames) 
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
   * @return used as return value of the reset() method
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
   * @param time time-tag for a single frame or a block of frames
   * @param values interleaved frames values, row by row (interleaving channels or columns), frame by frame
   * @param size size of eaqch of all frames
   * @param num number of frames
   *
   * @return used as return value of the frames() method
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
   * @return used as return value of the finalize() method
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
   * @brief Gets a PiPo modules reciver (call only by the PiPo host)
   *
   * return receiver PiPo module receiving this module's output stream
   *
   */
  PiPo *getReceiver(void) { return this->receiver; };
  
  /**
   * @brief Sets a PiPo modules reciver (call only by the PiPo host)
   *
   * @param receiver PiPo module receiving this module's output stream
   *
   */
  virtual void setReceiver(PiPo *receiver) { this->receiver = receiver; };
  
  /**
   * @brief Configures a PiPo module according to the input stream attributes and propagate output stream attributes
   *
   * PiPo module:
   * Any implementation of this method requires a propagateStreamAttributes() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the final output stream attributes and usally returns 0.
   *
   * @param hasTimeTags a boolean representing whether the elements of the stream are time-tagged
   * @param rate the frame rate (highest average rate for time-tagged streams)
   * @param offset the lag of the output stream relative to the input
   * @param width the frame width (also number of channels or matrix columns)
   * @param size the frame size (or number of matrix rows)
   * @param labels optional labels for the frames' channels or columns
   * @param hasVarSize a boolean representing whether the frames have a variable size (respecting the given frame size as maximum)
   * @param domain extent of a frame in the given domain (e.g. duration or frequency range)
   * @param maxFrames maximum number of frames in a block exchanged between two modules
   *
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   *
   */
  virtual int streamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames) 
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
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
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
   * @param time time-tag for a single frame or a block of frames
   * @param values interleaved frames values, row by row (interleaving channels or columns), frame by frame
   * @param size size of eaqch of all frames
   * @param num number of frames
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
   * @param inputEnd end time of the finalized input stream
   *
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   *
   */
  virtual int finalize(double inputEnd) 
  {
    return this->propagateFinalize(inputEnd); 
  };

  /**
   * @brief Signals that the output stream parameters of a given module have changed
   *
   * PiPo module:
   * This method is called (with 0) in method setting a module parameter that requires changing the output stream attributes.
   *
   * PiPo host:
   * The implementation of this method by the terminating receiver module provided by a PiPo host
   * would repropagate the input stream attributes by calling streamAttributes() of the first module.
   *
   * param unitId (for host) index of the module that initially called streamAttributesChanged().
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   *
   */
  virtual int streamAttributesChanged(unsigned int unitId = 0) 
  { 
    if(this->receiver != NULL) 
      return this->receiver->streamAttributesChanged(unitId + 1); 
    else 
      return -1; 
  };
    
protected:
  PiPo *receiver;
  vector<PiPoAttr *> attrs;
 
  unsigned int addAttr(const char *name, enum PiPoAttrDef::Type type, unsigned int size, const char *enumItems, bool changesStream, const char *descr)
  {
    unsigned int  index = this->attrs.size();
    
    PiPoAttrDef *attrDef = new PiPoAttrDef(name, type, size, enumItems, changesStream, descr);
    PiPoAttr *attr = attrDef->instantiate();
    this->attrs.push_back(attr);
    
    return index;
  };
  
  unsigned int addAttr(const char *name, enum PiPoAttrDef::Type type, unsigned int size, const char *enumItems, bool changesStream, const char *descr, int initVal)
  {
    unsigned int index = this->addAttr(name, type, size, enumItems, changesStream, descr);
    
    this->attrs[index]->set(initVal);
    
    return index;
  };
  
  unsigned int addAttr(const char *name, enum PiPoAttrDef::Type type, unsigned int size, const char *enumItems, bool changesStream, const char *descr, double initVal)
  {
    unsigned int index = this->addAttr(name, type, size, enumItems, changesStream, descr);
    
    this->attrs[index]->set(initVal);
    
    return index;
  };
  
  void addAttr(const char *name, PiPoAttr *attr)
  {
    attr->rename(name);
    this->attrs.push_back(attr);
  }
  
  void destroyAttrs(void)
  {
    if(this->attrs.size() > 0)
    {
      for(unsigned int i = 0; i < this->attrs.size(); i++)
      {
        this->attrs[i]->destroyDef();
        delete this->attrs[i];
      }
    }
  }  
  
public:
  int getAttrId(const char *attrName)
  {
    for(unsigned int i = 0; i < attrs.size(); i++)
    {
      if(strcasecmp(this->attrs[i]->getName(), attrName) == 0)
        return i;
    }
    
    return -1;
  }

  PiPoAttr *getAttr(unsigned int attrId)
  {
    if(attrId >= this->attrs.size())
      attrId = this->attrs.size() - 1;
    
    return this->attrs[attrId];
  }
    
  unsigned int getNumAttrs(void)
  {
    return this->attrs.size();
  }
  
  const char *getAttrName(unsigned int attrId) { return this->getAttr(attrId)->getName(); };
  enum PiPoAttrDef::Type getAttrType(unsigned int attrId) { return this->getAttr(attrId)->getType(); };
  unsigned int getAttrSize(unsigned int attrId) { return this->getAttr(attrId)->getSize(); };
  bool attrChangesStream(unsigned int attrId) { return this->getAttr(attrId)->changesStream(); };
  const char *getAttrEnum(unsigned int attrId) { return this->getAttr(attrId)->getEnum(); };
  const char *getAttrDescr(unsigned int attrId) { return this->getAttr(attrId)->getDescr(); };
  
  void clearAttribute(unsigned int attrId);
  bool setAttrVal(unsigned int attrId, int value){ return this->getAttr(attrId)->set(value); };
  bool setAttrVal(unsigned int attrId, double value) { return this->getAttr(attrId)->set(value); };
  bool setAttrVal(unsigned int attrId, const char *value) { return this->getAttr(attrId)->set(value); };
  bool setAttrVal(unsigned int attrId, vector<int> &value) { return this->getAttr(attrId)->set(value); };
  bool setAttrVal(unsigned int attrId, vector<double> &value) { return this->getAttr(attrId)->set(value); };
  bool setAttrVal(unsigned int attrId, vector<const char *> &value) { return this->getAttr(attrId)->set(value); };
  
  bool getAttrVal(unsigned int attrId, int &value) { return this->getAttr(attrId)->get(value); };
  bool getAttrVal(unsigned int attrId, double &value) { return this->getAttr(attrId)->get(value); };
  bool getAttrVal(unsigned int attrId, const char *&value) { return this->getAttr(attrId)->get(value); };
  bool getAttrVal(unsigned int attrId, vector<int> &value) { return this->getAttr(attrId)->get(value); };
  bool getAttrVal(unsigned int attrId, vector<double> &value) { return this->getAttr(attrId)->get(value); };
  bool getAttrVal(unsigned int attrId, vector<const char *> &value) { return this->getAttr(attrId)->get(value); };
};

#endif
