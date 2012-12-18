/**
 *
 * @file PiPoHost.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Plugin Interface for Processing Objects
 * 
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _PIPO_HOST_
#define _PIPO_HOST_

#include "PiPo.h"
#include <string>

using namespace std;

class PiPoModule
{ 
public:
  PiPoModule(void) { };
  ~PiPoModule(void) { };
};

class PiPoModuleFactory
{ 
public:
  PiPoModuleFactory(void) { };
  ~PiPoModuleFactory(void) { };
  
  virtual PiPoModule *createModule(unsigned int index, string &pipoName, string &instanceName, PiPo *&pipo) = 0;
};

class PiPoDecr 
{ 
public:
  string pipoName;
  string instanceName;
  
  PiPoDecr(void) : pipoName(), instanceName()
  {
  };
  
  PiPoDecr(const PiPoDecr &other)
  {
    this->pipoName = other.pipoName;
    this->instanceName = other.instanceName;
  }
  
  void parse(string str, unsigned int &pos)
  {
    unsigned int end = str.find_first_of(':', pos);
    unsigned int open = str.find_first_of('(', pos);
    unsigned int closed = str.find_first_of(')', pos);
    
    if(open < string::npos && closed < string::npos)
    {
      this->pipoName = str.substr(pos, open - pos);
      this->instanceName = str.substr(open + 1, closed - open - 1);
    }
    else
    {
      this->pipoName = str.substr(pos, end - pos);
      this->instanceName = this->pipoName;
    }
    
    if(end < string::npos)
      pos = end + 1;
    else
      pos = string::npos;
  };
  
  const char *getPiPoName()
  {
    return this->pipoName.c_str();
  }

  const char *getInstanceName()
  {
    return this->instanceName.c_str();
  }

  bool isInstanceName(const char *str)
  {
    return (this->instanceName.compare(str) == 0);
  }
};

class PiPoChainDescr : public vector<PiPoDecr>
{
public:
  PiPoChainDescr(const char *chainStr = NULL) : vector<PiPoDecr>()
  { 
    if(chainStr != NULL)
      this->parse(chainStr);
  };  
  
  ~PiPoChainDescr(void) { };
  
  unsigned int parse(const char *str)
  {
    string pipoChainStr = str;
    unsigned int pos = 0;
    
    this->clear();
    
    while(pos < string::npos)
    {
      PiPoDecr descr;
      
      descr.parse(pipoChainStr, pos);
      this->push_back(descr);
    }
    
    return this->size();
  }
  
  int getIndex(const char *instanceName)
  {
    for(unsigned int i = 0; i < this->size(); i++)
    {
      if((*this)[i].isInstanceName(instanceName))
        return i;
    }
    
    return -1;
  }
};

class PiPoOp
{ 
  PiPoDecr *descr;
  PiPo *pipo;
  PiPoModule *module;
  
public:
  
  PiPoOp(void)
  {
    this->descr = NULL;;
    this->pipo = NULL;
    this->module = NULL;
  };
  
  PiPoOp(const PiPoOp &other)
  {
    this->descr = other.descr;
    this->pipo = other.pipo;
    this->module = NULL; /* clone is a pure PiPo without module */
  };
  
  ~PiPoOp(void)
  {
    if(this->module != NULL)
      delete this->module;
    
    if(this->pipo != NULL)
      delete pipo;
  };
  
  PiPo *instantiate(PiPoModuleFactory *moduleFactory, unsigned int index, PiPoDecr &descr)
  {
    this->descr = &descr;
    this->pipo = NULL;
    this->module = NULL;
    
    if(moduleFactory != NULL)
      this->module = moduleFactory->createModule(index, descr.pipoName, descr.instanceName, this->pipo);
    
    return this->pipo;
  };
  
  PiPo *getPiPo()
  {
    return this->pipo;
  }
};

class PiPoChain : public PiPo, public vector<PiPoOp>
{
  PiPoModuleFactory *moduleFactory;

public:
  PiPoChain(PiPoModuleFactory *moduleFactory = NULL, PiPoChainDescr *chainDescr = NULL, PiPo *receiver = NULL) : PiPo(), vector<PiPoOp>()
  { 
    this->moduleFactory = moduleFactory;
    
    if(chainDescr != NULL)
      this->set(chainDescr, receiver);
  };  
  
  PiPoChain(const PiPoChain &other) : vector<PiPoOp>(*this)
  { 
    this->moduleFactory = NULL;
    
    PiPo *next = getTail();
    
    if(next != NULL)
    {
      for(int i = this->size() - 2; i >= 0; i--)
      {
        PiPo *pipo = (*this)[i].getPiPo();
        pipo->setReceiver(next);
        next = pipo;
      }
    }
  };  
  
  ~PiPoChain(void) { };
  
  PiPo *getHead(void)
  {
    if(this->size() > 0)
      return (*this)[0].getPiPo();
    
    return NULL;
  }
  
  PiPo *getTail(void)
  {
    if(this->size() > 0)
      return (*this)[this->size() - 1].getPiPo();
    
    return NULL;
  }
  
  void setReceiver(PiPo *receiver)
  {
    if(this->size() > 0)
    {
      PiPo *tail = this->getTail();
      
      if(tail != NULL)
        tail->setReceiver(receiver);
    }
  }
  
  int streamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->streamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames);
    
    return -1;
  }
  
  int reset(void)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->reset();
    
    return -1;
  }
  
  int frames(double time, float *values, unsigned int size, unsigned int num)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->frames(time, values, size, num);
    
    return -1;
  }
  
  int finalize(double inputEnd)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->finalize(inputEnd);
    
    return -1;
  }
  
  bool set(PiPoChainDescr *chainDescr, PiPo *receiver = NULL)
  {
    this->clear();
    
    if(chainDescr->size() > 0)
    {
      this->resize(chainDescr->size());
      
      for(int i = this->size() - 1; i >= 0; i--)
      {
        PiPo *pipo = (*this)[i].instantiate(this->moduleFactory, i, (*chainDescr)[i]);
        
        if(pipo == NULL)
        {    
          this->clear();
          return false; 
        }
        
        pipo->setReceiver(receiver);
        receiver = pipo;
      }
      
      return true;
    }
    
    return false;
  };
};

#endif
