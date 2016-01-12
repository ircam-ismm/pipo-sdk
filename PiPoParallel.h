/**
 *
 * @file PiPoParallel.h
 * @author Diemo.Schwarz@ircam.fr
 * 
 * @brief PiPo dataflow graph class that encapsulates a parallel section of pipo modules.
 * 
 * Copyright (C) 2016 by ISMM IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */

#include "PiPo.h"

class PiPoParallel : public PiPo
{
private:
  /** class to merge several parallel pipo data streams, combining columns
   */
  class PiPoMerge : public PiPo
  {
    int count_;
    int numpar_;
    
    int width_;	// combined num. of columns

  public:
    PiPoMerge (PiPo::Parent *parent)
    : PiPo(parent), count_(0), numpar_(0), width_(0)
    { }

    void start (int numpar)
    { // on start, record number of calls to expect from parallel pipos, each received stream call decrements count, on 0 merging has to be performed
      numpar_ = numpar;
      count_  = 0;
    }

    int streamAttributes (bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
    { // collect stream attributes declarations from parallel pipos
      if (count_ == 0)
	;	// first parallel pipo sets most stream attributes

      width_ += width;	// columns are concatenated
      count_++;
      
      if (count_ == numpar_)
	return propagateStreamAttributes(hasTimeTags, rate, offset, width_, height, labels, hasVarSize, domain, maxFrames);
      else
	return 0; // continue receiving stream attributes
    };

    
    int frames (double time, double weight, float *values, unsigned int size, unsigned int num)
    { // collect data from parallel pipos
      count_++;
      
      if (count_ == numpar_)
	return propagateFrames(time, weight, values, size, num);
      else
	return 0; // continue receiving frames
    };
  
  };

  PiPoMerge merge;
  
public:
  PiPoParallel (PiPo::Parent *parent)
  : PiPo(parent), merge(parent)
  { };  

  // copy constructor
  PiPoParallel (const PiPoParallel &other)
  : PiPo(other), merge(other.merge)
  { };  

  //todo: varargs constructor PiPoParallel (PiPo::Parent *parent, PiPo *pipos ...)
  
  const PiPoParallel& operator= (const PiPoParallel &other)
  {
    parent = other.parent;
    merge  = other.merge;

    return *this;
  };
  
  ~PiPoParallel (void) { };
  

  /** @name PiPoParallel setup methods */
  /** @{ */
  
  void add (PiPo *pipo)
  {
    setReceiver(pipo, true);
    pipo->setReceiver(&merge);
  }
  
  /** @} PiPoParallel setup methods */

  /** @name overloaded PiPo methods */
  /** @{ */

  void setParent (PiPo::Parent *parent)
  {
    this->parent = parent;
    
    for (unsigned int i = 0; i < receivers.size(); i++)
      receivers[i]->setParent(parent);
  };
  
  void setReceiver (PiPo *receiver, bool add = false)
  {
    merge.setReceiver(receiver, add);
  };
    
  /** @name preparation and processing methods: just notify merge, and let propagate* do the branching */
  /** @{ */

  /** start stream preparation */
  int streamAttributes (bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
  {
    merge.start(receivers.size());
    return PiPo::propagateStreamAttributes(hasTimeTags, rate, offset, width, height, labels, hasVarSize, domain, maxFrames);
  };
  
  int reset ()
  {
    merge.start(receivers.size());
    return PiPo::propagateReset();
  };
  
  /** @} end of preparation of processing methods */

  /** @name processing */
  /** @{ */

  int frames (double time, double weight, float *values, unsigned int size, unsigned int num)
  {
    merge.start(receivers.size());
    return PiPo::propagateFrames(time, weight, values, size, num);
  };
  
  int finalize (double inputEnd)
  {
    merge.start(receivers.size());
    return PiPo::propagateFinalize(inputEnd);
  };

  /** @} end of processing methods */
  /** @} end of overloaded PiPo methods */
};

/** EMACS **
 * Local variables:
 * mode: c
 * c-basic-offset:2
 * End:
 */
