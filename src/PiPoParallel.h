/** -*- mode: c++; c-basic-offset:2 -*-

@file PiPoParallel.h
@author Diemo.Schwarz@ircam.fr

@brief PiPo dataflow graph class that encapsulates a parallel section of pipo modules.

@copyright

Copyright (c) 2012–2016 by IRCAM – Centre Pompidou, Paris, France.
All rights reserved.

@par License (BSD 3-clause)

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

- Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef _PIPO_PARALLEL_H_
#define _PIPO_PARALLEL_H_

#include <assert.h> //db
#include <stdlib.h> //db
#include "PiPo.h"

#define PIPO_DEBUG DEBUG*1

class PiPoParallel : public PiPo
{
private:
  /** class to merge several parallel pipo data streams, combining columns
   */
  class PiPoMerge : public PiPo
  {
  private:
#   define		 MAX_PAR 64
#   define		 NUM_LABELS_INIT 16
    int			 count_  = 0;
    int			 branch_ = 0;
    int			 numpar_ = 0;
    int			 numseg_ = 0;
    PiPoStreamAttributes sa_;	// combined stream attributes
    int			 paroffset_[MAX_PAR]; // cumulative column offsets in output array TODO: use vector
    int			 parwidth_[MAX_PAR];  // column widths of parallel pipos
    int			 framesize_;		// output frame size = width * maxheight
    std::vector<std::string> labelstore_;

    // working variables for merging of frames
    PiPoValue		*values_;
    int			 time_;
    unsigned int	 numrows_;
    unsigned int	 numframes_;

  public:
    PiPoMerge (PiPo::Parent *parent)
    : PiPo(parent), sa_(NUM_LABELS_INIT), labelstore_(), framesize_(0), values_(NULL)
    {
#ifdef DEBUG	// clean memory to make possible memory errors more consistent at least
      memset(paroffset_, 0, sizeof(*paroffset_) * MAX_PAR);
      memset(parwidth_,  0, sizeof(*parwidth_) * MAX_PAR);
#endif
      labelstore_.reserve(NUM_LABELS_INIT);
    }

    // copy constructor
    PiPoMerge (const PiPoMerge &other)
    : PiPo(other.parent)
    {
#if defined(__GNUC__) &&  PIPO_DEBUG >= 2
      printf("\n•••••• %s: COPY CONSTRUCTOR\n", __PRETTY_FUNCTION__); //db
#endif

      copy_from(other);
    }

    // assignment operator
    PiPoMerge &operator= (const PiPoMerge &other)
    {
#if defined(__GNUC__) &&  PIPO_DEBUG >= 2
      printf("\n•••••• %s: ASSIGNMENT OPERATOR\n", __PRETTY_FUNCTION__); //db
#endif

      copy_from(other);

      return *this;
    }

    // destructor
    ~PiPoMerge ()
    {
      free(values_);
    }

  private:
    void copy_from (const PiPoMerge &other)
    {
#ifdef DEBUG	// clean memory to make possible memory errors more consistent at least
      memset(paroffset_, 0, sizeof(*paroffset_) * MAX_PAR);
      memset(parwidth_,  0, sizeof(*parwidth_) * MAX_PAR);
#endif

      parent      = other.parent;
      count_      = other.count_;
      branch_     = other.branch_;
      numpar_     = other.numpar_;
      numseg_     = other.numseg_;
      sa_         = other.sa_; // does shallow copy of labels array
      labelstore_ = other.labelstore_;
      framesize_  = other.framesize_;

      memcpy(paroffset_, other.paroffset_, numpar_ * sizeof(int));
      memcpy(parwidth_, other.parwidth_, numpar_ * sizeof(int));
      values_ = (PiPoValue *) malloc(sa_.maxFrames * framesize_ * sizeof(PiPoValue));
      memcpy(values_, other.values_, sa_.maxFrames * framesize_ * sizeof(PiPoValue));

      // redirect labels string pointers to copied labelstore_
      for (unsigned int i = 0; i < sa_.numLabels; i++)
	if (sa_.labels[i] != NULL)
	  sa_.labels[i] = labelstore_[i].c_str();
    }

    // copy label strings into internal storage (parallel branches are not on stack anymore)
    void append_labels (const char **labels, unsigned int width)
    {
      unsigned int new_num = sa_.numLabels + width;

      if (new_num > sa_.labels_alloc)
      {
	sa_.labels = (const char **) realloc(sa_.labels, new_num * sizeof(const char **));
	sa_.labels_alloc = new_num;
      }
      labelstore_.resize(new_num);

      for (int i = sa_.numLabels, k = 0; i < new_num; i++, k++)
      {
        if (labels && labels[k])
          labelstore_[i].assign(labels[k]);	// copy input c-string into std::string
        else
          labelstore_[i].assign("");  		// set empty string
      }

      for (int i = 0; i < new_num; i++)
        sa_.labels[i] = labelstore_[i].c_str(); // have our labels point to (possibly moved) internal label storage

      sa_.numLabels = new_num;
    }
    
  public: // PiPoMerge control functions called by PiPoParallel's pipo functions at start of parallel branches
    void start (size_t numpar, bool reset = false)
    { // on start, record number of calls to expect from parallel pipos, each received stream call increments count_, when numpar_ is reached, merging has to be performed
      if (reset)
	numseg_ = 0;
      
      numpar_ = (int) numpar - numseg_;
      count_  = 0;
      branch_ = 0;
    } // end PiPoMerge::start()

    void setbranch (size_t i)
    { // PiPoParallel's streamAttributes() and frames() methods set branch explicitly (to detect if one branch's result is missing)
      branch_  = i;
    }

// TODO: signal end of parallel pipos, accomodates for possibly missing calls down the chain
    void finish () { }

  public: // PiPoMerge pipo functions, called by last pipos of parallel branches
    int streamAttributes (bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
    { // PiPoMerge: collect stream attributes declarations from parallel pipos
#if PIPO_DEBUG >= 1
      printf("PiPoMerge %d (count %d / numpar %d - seg %d) streamAttributes timetags %d  rate %f  offset %f  width %d  height %d  labels %s  varsize %d  domain %f  maxframes %d\n", count_, branch_, numpar_, numseg_,
	     hasTimeTags, rate, offset, width, height, labels && width > 0 ? labels[0] : "n/a", hasVarSize, domain, maxFrames);
#endif
      int ret = 0;

      if (count_ == 0)
      {	// first parallel pipo defines most stream attributes, we store then here
	sa_.hasTimeTags = hasTimeTags;
    	sa_.rate = rate;
    	sa_.offset = offset;
    	sa_.dims[0] = width;
    	sa_.dims[1] = height; //FIXME: what to do with empty frame with height 0?
    	sa_.hasVarSize = hasVarSize;
    	sa_.domain = domain;
    	sa_.maxFrames = maxFrames;

	sa_.numLabels = 0;
	append_labels(labels, width);
	
	paroffset_[0] = 0;
	parwidth_[0] = width;
      }
      else
      { // apply merge rules with following pipos
	if (height > 0  &&  width > 0)
	{
	  std::string errmsg = "";
	
	  // check that timetags, rate, maxframes, height... are compatible
	  if (sa_.hasTimeTags == 1  ||  hasTimeTags == 1) // accept only sampled for now
	    errmsg += "Only sampled streams can be put in parallel.  ";
	  if (height > 0  &&  width > 0  &&  sa_.rate != rate)
	    errmsg += "Streams differ in rate (" + std::to_string(sa_.rate) + " and " + std::to_string(rate) + ").  ";
	  if (sa_.dims[1] != height)
	    // zero-size streams (as from segmenters) are ignored when their time structure is compatible
	    errmsg += "Streams differ in frame height (" + std::to_string(sa_.dims[1]) + " and " + std::to_string(height) + ").  ";
	  if (sa_.hasVarSize == 1  ||  hasVarSize == 1) // accept only fixed height for now
	    errmsg += "Only streams with fixed frame size can be put in parallel.  ";
	  
	  if (errmsg.size() > 0)
	  {
	    signalError("Incompatible parallel streams: " + errmsg);
	    ret = -1;
	  }
	}
	else
	{ // detect parallel branch that produces only segment() calls, no data output
	  // (preliminarily detected via 0 size frames, but beware, also old-style segmenters like onseg can produce 0 size frames as markers)
	  // we'll remove it from the count of expected calls
#if PIPO_DEBUG
	  printf("PiPoMerge::streamAttributes: detected zero size frame output from branch %d of %d, eliding branch\n", count_, numpar_);
#endif
	  numseg_++;
	}
	
	// columns are concatenated
	append_labels(labels, width);
      	sa_.dims[0] += width;
	paroffset_[count_] = paroffset_[count_ - 1] + parwidth_[count_ - 1];
	parwidth_[count_] = width;
	
	//TODO: option to transpose column vectors
      }
      
      if (++count_ == numpar_)
      { // last parallel pipo, now reserve memory and pass merged stream attributes onwards
        framesize_ = sa_.dims[0] * sa_.dims[1];
	values_ = (PiPoValue *) realloc(values_, sa_.maxFrames * framesize_ * sizeof(PiPoValue)); // alloc space for maximal block size
	
	if (ret == 0)
	  return propagateStreamAttributes(sa_.hasTimeTags, sa_.rate, sa_.offset, sa_.dims[0], sa_.dims[1], sa_.labels, sa_.hasVarSize, sa_.domain, sa_.maxFrames);
      }
      // else: continue receiving stream attributes
      
      return ret; 
    } // end PiPoMerge::streamAttributes()

    
    int reset ()
    {
      if (++count_ == numpar_)
	return this->propagateReset();
      else
	return 0; // continue receiving reset
    } // end PiPoMerge::reset()
    

    int frames (double time, double weight, PiPoValue *values, unsigned int size, unsigned int num)
    { // PiPoMerge: collect data from parallel pipos
      if (count_ >= numpar_) // bug is still there
      {
        printf("PiPoMerge::frames(%f): ARGH! count_ %d >= numpar_ %d\n", time, count_, numpar_);
        count_ = numpar_ - 1;
      }
      //assert(size / parwidth_[count_] == 1);

      int width = parwidth_[branch_];
      unsigned int height = width > 0  ?  size / width  :  1;	// number of input rows, 1 if empty frame
      
      if (count_++ == 0)
      { // first actually arriving parallel branch determines time tag, num. rows and frames
	time_      = time;
	numrows_   = height;
	numframes_ = num;

	// clear memory just in case one pipo doesn't output data (FIXME: handle this correctly)
	memset(values_, 0, num * framesize_ * sizeof(PiPoValue));
      }

      // copy input data to be kept from parallel pipo to merged values_
      if (num > numframes_)	num = numframes_;
      if (height > numrows_)	height = numrows_;
      
      for (unsigned int i = 0; i < num; i++)   // for all frames present
	for (unsigned int k = 0; k < height; k++)   // for all rows to be kept
        {
          //printf("merge::frames %p\n  values_ %p + %d + %d + %d,\n  values %p + %d,\n  size %d\n",
          //       this, values_, i * framesize_, k * sa_.dims[0], paroffset_[branch_], values, i * size, parwidth_[branch_] * sizeof(PiPoValue));
	  //TODO: zero pad if num rows here: size / parwidth_[branch_] < numrows_
	  memcpy(values_ + i * framesize_ + k * sa_.dims[0] + paroffset_[branch_],
		 values  + i * size + k * width,  width * sizeof(PiPoValue));
        }

      if (branch_ == numpar_ - 1) // last parallel pipo: pass on to receiver(s) //TODO: in desync case (with last branch not outputting), this might never happen ---> need check in parallel::frames
	// NOTE: when no branch produces output, merge::frames is never called, which is ok
	return propagateFrames(time_, 0 /*weight to disappear*/, values_, numrows_ * sa_.dims[0], numframes_);
      else
	return 0; // continue receiving frames
    } // end PiPoMerge::frames()


    int finalize (double inputEnd)
    {
      if (count_ == 0)
	time_ = inputEnd;
      
      if (++count_ == numpar_)
	return this->propagateFinalize(time_);
      else
	return 0; // continue receiving finalize
    } // end PiPoMerge::finalize()
  }; // end class PiPoMerge

  PiPoMerge merge; // the merge pipo module at the end of the parallel section

public:
  // constructor
  PiPoParallel (PiPo::Parent *parent)
  : PiPo(parent), merge(parent)
  { }

  //TODO: varargs constructor PiPoParallel (PiPo::Parent *parent, PiPo *pipos ...)

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
  // destructor
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

  void setParent (PiPo::Parent *parent)
  {
    this->parent = parent;
    
    for (unsigned int i = 0; i < receivers.size(); i++)
      receivers[i]->setParent(parent);
  }
  
  void setReceiver (PiPo *receiver, bool add = false)
  {
    merge.setReceiver(receiver, add);
  }
    
  /** @name preparation and processing methods: just notify merge, and let propagate* do the branching */
  /** @{ */

  /** start stream preparation */
  int streamAttributes (bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
  {
    int ret = 0;

    merge.start(receivers.size(), true);
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      merge.setbranch(i);
      ret = this->receivers[i]->streamAttributes(hasTimeTags, rate, offset, width, height, labels, hasVarSize, domain, maxFrames);

      if(ret < 0)
        break;
    }

    return ret;
  } // end PiPoParallel::streamAttributes()
  
  int reset ()
  {
    merge.start(receivers.size());
    return PiPo::propagateReset();
  }
  /** @} end of preparation of processing methods */

  
  /** @name processing */
  /** @{ */
  int segment (double time, bool start) 
  { // PiPoParallel: pass segment call to parallel branches, count branches since propagateFrames() might be called
    int ret = -1;

    merge.start(receivers.size());
    for (unsigned int i = 0; i < receivers.size(); i++)
    {
      merge.setbranch(i);
      ret = receivers[i]->segment(time, start);
	
      if (ret < 0)
	break;
    }

    return ret;
  } // end PiPoParallel::segment()

  
  int frames (double time, double weight, PiPoValue *values, unsigned int size, unsigned int num)
  {
    merge.start(receivers.size());

    int ret = -1;

    for (unsigned int i = 0; i < receivers.size(); i++)
    {
      merge.setbranch(i); // tell merge which branch's output to expect
      ret = receivers[i]->frames(time, weight, values, size, num);

      if(ret < 0)
        break;
    }

    merge.finish();
    return ret;
  } // end PiPoParallel::frames()
  
  int finalize (double inputEnd)
  {
    merge.start(receivers.size());
    return PiPo::propagateFinalize(inputEnd);
  } // end PiPoParallel::finalize()

  /** @} end of processing methods */
  /** @} end of overloaded PiPo methods */
};

#endif /* _PIPO_PARALLEL_H_ */


/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset:2
 * End:
 */
