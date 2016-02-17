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

#define PIPO_DEBUG 2

class PiPoParallel : public PiPo
{
private:
  /** class to merge several parallel pipo data streams, combining columns
   */
  class PiPoMerge : public PiPo
  {
#   define		 MAX_PAR 64
    int			 count_;
    int			 numpar_;
    PiPoStreamAttributes sa_;	// combined stream attributes
    int			 paroffset_[MAX_PAR]; // cumulative column offsets in output array
    int			 parwidth_[MAX_PAR];  // column widths of parallel pipos

    // working variables for merging of frames
    PiPoValue		*values_;
    int			 time_;
    int			 numrows_;
    int			 numframes_;

  public:
    PiPoMerge (PiPo::Parent *parent)
    : PiPo(parent), count_(0), numpar_(0), sa_(1024), values_(NULL)
    { }

    // copy constructor
    PiPoMerge (const PiPoMerge &other)
    : PiPo(other.parent), count_(other.count_), numpar_(other.numpar_), sa_(other.sa_)
    {
      printf("\n•••••• %s: COPY CONSTRUCTOR\n", __PRETTY_FUNCTION__); //db

      memcpy(paroffset_, other.paroffset_, numpar_ * sizeof(int));
      memcpy(parwidth_, other.parwidth_, numpar_ * sizeof(int));
      values_ = (PiPoValue *) malloc(sa_.maxFrames * sa_.dims[0] * sa_.dims[1] * sizeof(PiPoValue));
      memcpy(values_, other.values_, sa_.maxFrames * sa_.dims[0] * sa_.dims[1] * sizeof(PiPoValue));
    }

    // assignment operator
    PiPoMerge &operator= (const PiPoMerge &other)
    {
      printf("\n•••••• %s: ASSIGNMENT OPERATOR\n", __PRETTY_FUNCTION__); //db

      count_ = other.count_;
      numpar_ = other.numpar_;
      sa_ = other.sa_;
      
      memcpy(paroffset_, other.paroffset_, numpar_ * sizeof(int));
      memcpy(parwidth_, other.parwidth_, numpar_ * sizeof(int));
      values_ = (PiPoValue *) malloc(sa_.maxFrames * sa_.dims[0] * sa_.dims[1] * sizeof(PiPoValue));
      memcpy(values_, other.values_, sa_.maxFrames * sa_.dims[0] * sa_.dims[1] * sizeof(PiPoValue));

      return *this;
    }

  public:
    void start (int numpar)
    { // on start, record number of calls to expect from parallel pipos, each received stream call increments count_, when numpar_ is reached, merging has to be performed
      numpar_ = numpar;
      count_  = 0;
    }

  public:
    int streamAttributes (bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
    { // collect stream attributes declarations from parallel pipos
#if PIPO_DEBUG >= 1
      printf("PiPoParallel streamAttributes timetags %d  rate %f  offset %f  width %d  height %d  labels %s  varsize %d  domain %f  maxframes %d\n",
	     hasTimeTags, rate, offset, width, height, labels ? labels[0] : "n/a", hasVarSize, domain, maxFrames);
#endif

      if (count_ == 0)
      {	// first parallel pipo defines most stream attributes, we store then here
	sa_.hasTimeTags = hasTimeTags;
    	sa_.rate = rate;
    	sa_.offset = offset;
    	sa_.dims[0] = width;
    	sa_.dims[1] = height;
    	sa_.numLabels = 0;
    	sa_.hasVarSize = hasVarSize;
    	sa_.domain = domain;
    	sa_.maxFrames = maxFrames;
	sa_.concat_labels(labels, width);
	
	paroffset_[0] = 0;
	parwidth_[0] = width;
      }
      else
      { // apply merge rules with following pipos
	// columns are concatenated
	sa_.concat_labels(labels, width);
      	sa_.dims[0] += width;
	paroffset_[count_] = paroffset_[count_ - 1] + parwidth_[count_ - 1];
	parwidth_[count_] = width;

	//TODO: check maxframes, should not differ
	//TODO: option to transpose column vectors
      }
      
      if (++count_ == numpar_)
      { // last parallel pipo, now reserve memory and pass merged stream attributes onwards
	values_ = (PiPoValue *) realloc(values_, sa_.maxFrames * sa_.dims[0] * sa_.dims[1] * sizeof(PiPoValue)); // alloc space for maxmal block size
	
	return propagateStreamAttributes(sa_.hasTimeTags, sa_.rate, sa_.offset, sa_.dims[0], sa_.dims[1], sa_.labels, sa_.hasVarSize, sa_.domain, sa_.maxFrames);
      }
      else
	return 0; // continue receiving stream attributes
    }

    
    int reset ()
    {
      if (++count_ == numpar_)
	return this->propagateReset();
      else
	return 0; // continue receiving reset
    }

    
    int frames (double time, double weight, float *values, unsigned int size, unsigned int num)
    { // collect data from parallel pipos
      if (count_ == 0)
      { // for parallel pipo determines time tag, num. rows and frames
	time_      = time;
	numrows_   = size / parwidth_[0];	// number of rows
	numframes_ = num;
      }
      
      for (int i = 0; i < numframes_; i++)   // for all frames
	for (int k = 0; k < numrows_; k++)   // for all rows to be kept
	  //TODO: zero pad if num rows here: size / parwidth_[count_] < numrows_
	  memcpy(values_ + paroffset_[count_], values, parwidth_[count_] * sizeof(PiPoValue));
      
      if (++count_ == numpar_) // last parallel pipo: pass on to receiver(s)
	return propagateFrames(time_, 0 /*weight to disappear*/, values_, numrows_ * sa_.dims[0], numframes_);
      else
	return 0; // continue receiving frames
    }


    int finalize (double inputEnd)
    {
      if (count_ == 0)
	time_ = inputEnd;
      
      if (++count_ == numpar_)
	return this->propagateFinalize(time_);
      else
	return 0; // continue receiving finalize
    }
  }; // end class PiPoMerge

  PiPoMerge merge;
  
public:
  PiPoParallel (PiPo::Parent *parent)
  : PiPo(parent), merge(parent)
  { }

  //todo: varargs constructor PiPoParallel (PiPo::Parent *parent, PiPo *pipos ...)

private:
  // copy constructor
  PiPoParallel (const PiPoParallel &other)
  : PiPo(other), merge(other.merge)
  { }

  // assignment operator
  const PiPoParallel& operator= (const PiPoParallel &other)
  {
    parent = other.parent;
    merge  = other.merge;

    return *this;
  }
  
public:
  ~PiPoParallel (void) { }
  

  /** @name PiPoParallel setup methods */
  /** @{ */
  
  /** Add module @p{pipo} to the data flow graph in parallel.
   */
  void add (PiPo *pipo)
  { // add to list of receivers of this parallel module, to branch out on input
    PiPo::setReceiver(pipo, true);
    // then connect module to internal merge module
    pipo->setReceiver(&merge);
  }

  void add (PiPo &pipo)
  {
    add(&pipo);
  }

  /** @} PiPoParallel setup methods */

  /** @name overloaded PiPo methods */
  /** @{ */

  virtual void setParent (PiPo::Parent *parent)
  {
    this->parent = parent;
    
    for (unsigned int i = 0; i < receivers.size(); i++)
      receivers[i]->setParent(parent);
  }
  
  virtual void setReceiver (PiPo *receiver, bool add = false)
  {
    merge.setReceiver(receiver, add);
  }
    
  /** @name preparation and processing methods: just notify merge, and let propagate* do the branching */
  /** @{ */

  /** start stream preparation */
  int streamAttributes (bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
  {
    merge.start(receivers.size());
    return PiPo::propagateStreamAttributes(hasTimeTags, rate, offset, width, height, labels, hasVarSize, domain, maxFrames);
  }
  
  int reset ()
  {
    merge.start(receivers.size());
    return PiPo::propagateReset();
  }
  
  /** @} end of preparation of processing methods */

  /** @name processing */
  /** @{ */

  int frames (double time, double weight, float *values, unsigned int size, unsigned int num)
  {
    merge.start(receivers.size());
    return PiPo::propagateFrames(time, weight, values, size, num);
  }
  
  int finalize (double inputEnd)
  {
    merge.start(receivers.size());
    return PiPo::propagateFinalize(inputEnd);
  }

  /** @} end of processing methods */
  /** @} end of overloaded PiPo methods */
};

/** EMACS **
 * Local variables:
 * mode: c
 * c-basic-offset:2
 * End:
 */
