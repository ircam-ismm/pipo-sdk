/**
 *
 * @file PiPoSequence.h
 * @author Diemo.Schwarz@ircam.fr
 * 
 * @brief PiPo dataflow graph class that encapsulates a sequence of pipo modules.
 * 
 * Copyright (C) 2016 by ISMM IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */

#include "PiPo.h"

class PiPoSequence : public PiPo
{
private:    
  std::vector<PiPo *> seq_;

public:
  PiPoSequence (PiPo::Parent *parent)
  : PiPo(parent), seq_()
  { }

  /** varargs constructor with a list of pipos that will be connected:

        PiPoSequence (PiPo::Parent *parent, PiPo &pipos ...) 
        seq(parent, pipo1, pipo2, ...);

      (using C++11 variadic templates)
   */
  template<typename ...Args>
  PiPoSequence (PiPo::Parent *parent, Args&... pipos)
    : PiPo(parent), seq_{&pipos ...} // use C++11 initilizer_list syntax and variadic templates
  {
    // set parents of all pipos?
    connect(NULL);
  }

  // copy constructor
  PiPoSequence (const PiPoSequence &other)
  : PiPo(other), seq_(other.seq_)
  { 
    connect(NULL);
  }  
  
  const PiPoSequence& operator=(const PiPoSequence &other)
  {
    parent = other.parent;
    seq_   = other.seq_;
    connect(NULL);

    return *this;
  }
  
  ~PiPoSequence (void) { }
  

  /** @name PiPoSequence setup methods */
  /** @{ */

  /** append module \p pipo to sequential data flow graph
   */
  void add (PiPo *pipo, bool autoconnect = true)
  {
    seq_.push_back(pipo);
    
    if (autoconnect  &&  seq_.size() > 1)
      seq_[seq_.size() - 2]->setReceiver(pipo); // connect previous to just added pipo
  }

  void add (PiPo &pipo, bool autoconnect = true)
  {
    add(&pipo, autoconnect);
  }

  
  void clear ()
  {
    for (unsigned int i = 0; i < seq_.size(); i++)
      seq_[i] = NULL;

    seq_.clear();
  }
  

  /** connect each PiPo in PiPoSequence (from end to start)

      @param receiver is terminating PiPo of the host that finally receives data
   */
  bool connect (PiPo *receiver)
  {
    PiPo *next = getTail();
    
    if (next != NULL)
    {
      next->setReceiver(receiver);
      
      for (int i = seq_.size() - 2; i >= 0; i--)
      {
        PiPo *pipo = seq_[i];
        pipo->setReceiver(next);
        next = pipo;
      }
      
      return true;
    }
    
    return false;
  }

  /** @} PiPoSequence setup methods */

  /** @name PiPoChain query methods */
  /** @{ */

  int getSize()
  {
    return seq_.size();
  }

  PiPo *getHead ()
  {
    if (seq_.size() > 0)
      return seq_[0];
    
    return NULL;
  }
  
  PiPo *getTail ()
  {
    if (seq_.size() > 0)
      return seq_[seq_.size() - 1];
    
    return NULL;
  }
  
  PiPo *getPiPo (unsigned int index)
  {
    if (index < seq_.size())
      return seq_[index];
    
    return NULL;
  }  

  /** @} PiPoSequence query methods */
    
  /** @name overloaded PiPo methods */
  /** @{ */

  virtual void setParent (PiPo::Parent *parent)
  {
    this->parent = parent;
    
    for (unsigned int i = 0; i < seq_.size(); i++)
      seq_[i]->setParent(parent);
  }
  
  virtual void setReceiver (PiPo *receiver, bool add = false)
  {
    PiPo *tail = getTail();
      
    if (tail != NULL)
      tail->setReceiver(receiver, add);
  }
    
  /** @name preparation of processing */
  /** @{ */

  /** start stream preparation */
  int streamAttributes (bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
  {
    PiPo *head = getHead();
    
    if (head != NULL)
      return head->streamAttributes(hasTimeTags, rate, offset, width, height, labels, hasVarSize, domain, maxFrames);
    
    return -1;
  }
  
  int reset ()
  {
    PiPo *head = getHead();
    
    if (head != NULL)
      return head->reset();
    
    return -1;
  }
  
  /** @} end of preparation of processing methods */

  /** @name processing */
  /** @{ */

  int frames (double time, double weight, float *values, unsigned int size, unsigned int num)
  {
    PiPo *head = getHead();
    
    if (head != NULL)
      return head->frames(time, weight, values, size, num);
    
    return -1;
  }
  
  int finalize (double inputEnd)
  {
    PiPo *head = getHead();
    
    if (head != NULL)
      return head->finalize(inputEnd);
    
    return -1;
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
