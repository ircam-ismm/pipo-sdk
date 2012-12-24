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
#include <vector>

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
  
  virtual PiPo *create(unsigned int index, const std::string &pipoName, const std::string &instanceName, PiPoModule *&module) = 0;
  virtual void attrChanged(PiPo *pipo, unsigned int index) { };
};

class PiPoOp
{ 
  unsigned int index;
  std::string pipoName;
  std::string instanceName;
  PiPo *pipo;
  PiPoModule *module;
  
public:
  PiPoOp(unsigned int index) : pipoName(), instanceName()
  {
    this->index = index;
    this->pipo = NULL;
    this->module = NULL;
  };
  
  ~PiPoOp(void) { this->clear(); };

  void clear(void)
  {
    if(this->module != NULL)
      delete this->module;
    
    if(this->pipo != NULL)
      delete pipo;
  };
  
  void parse(std::string str, unsigned int &pos)
  {
    this->clear();
    
    unsigned int end = str.find_first_of(':', pos);
    unsigned int open = str.find_first_of('(', pos);
    unsigned int closed = str.find_first_of(')', pos);
    
    if(open < std::string::npos && closed < std::string::npos)
    {
      this->pipoName = str.substr(pos, open - pos);
      this->instanceName = str.substr(open + 1, closed - open - 1);
    }
    else
    {
      this->pipoName = str.substr(pos, end - pos);
      this->instanceName = this->pipoName;
    }
    
    if(end < std::string::npos)
      pos = end + 1;
    else
      pos = std::string::npos;
  };
  
  bool instantiate(PiPoModuleFactory *moduleFactory)
  {
    this->pipo = NULL;
    this->module = NULL;
    
    if(moduleFactory != NULL)
      this->pipo = moduleFactory->create(index, this->pipoName, this->instanceName, this->module);
    
    return (this->pipo != NULL);
  };

  void clone(PiPoModuleFactory *moduleFactory, const PiPoOp &other)
  {
    this->pipoName = other.pipoName;
    this->instanceName = other.instanceName;
    
    instantiate(moduleFactory);
    
    if(this->pipo != NULL)
      this->pipo->cloneAttrs(other.pipo);
  };
  
  PiPo *getPiPo() { return this->pipo; };
  const char *getInstanceName() { return this->instanceName.c_str(); };
  bool isInstanceName(const char *str) { return (this->instanceName.compare(str) == 0); };
};

class PiPoChain : public PiPo, public std::vector<PiPoOp>
{
  PiPoModuleFactory *moduleFactory;

public:
  PiPoChain(PiPoModuleFactory *moduleFactory = NULL) : PiPo(), std::vector<PiPoOp>()
  { 
    this->moduleFactory = moduleFactory;
  };  
  
  PiPoChain(const PiPoChain &other) : std::vector<PiPoOp>(*this)
  { 
    this->moduleFactory = NULL;
    this->connect(NULL);
  };  
  
  const PiPoChain& operator=(const PiPoChain &other)
  {
    this->moduleFactory = other.moduleFactory;
    
    this->clear();
    
    for(unsigned int i = 0; i < other.size(); i++)
    {
      this->resize(i + 1, i); 
      (*this)[i].clone(this->moduleFactory, other[i]);
    }
    
    this->connect(NULL);
    this->moduleFactory = NULL;

    return *this;
  };
  
  ~PiPoChain(void) { };
  
  unsigned int parse(const char *str)
  {
    std::string pipoChainStr = str;
    unsigned int pos = 0;
    
    this->clear();
    
    for(unsigned int i = 0; pos < std::string::npos; i++)
    {
      this->resize(i + 1, i); 
      (*this)[i].parse(pipoChainStr, pos);
    }
    
    return this->size();
  };
    
  bool instantiate(void)
  {
    if(this->size() > 0)
    {
      for(unsigned int i = 0; i < this->size(); i++)
      {
        if(!(*this)[i].instantiate(this->moduleFactory))
        {
          this->clear();
          return false; 
        }
      }
      
      return true;
    }
    
    return false;
  };
  
  bool connect(PiPo *receiver)
  {
    PiPo *next = getTail();
    
    if(next != NULL)
    {
      next->setReceiver(receiver);
      
      for(int i = this->size() - 2; i >= 0; i--)
      {
        PiPo *pipo = (*this)[i].getPiPo();
        pipo->setReceiver(next);
        next = pipo;
      }
      
      return true;
    }
    
    return false;
  };
  
  void setReceiver(PiPo *receiver)
  {
    if(this->size() > 0)
    {
      PiPo *tail = this->getTail();
      
      if(tail != NULL)
        tail->setReceiver(receiver);
    }
  };
  
  PiPo *getHead(void)
  {
    if(this->size() > 0)
      return (*this)[0].getPiPo();
    
    return NULL;
  };
  
  PiPo *getTail(void)
  {
    if(this->size() > 0)
      return (*this)[this->size() - 1].getPiPo();
    
    return NULL;
  };
  
  int getIndex(const char *instanceName)
  {
    for(unsigned int i = 0; i < this->size(); i++)
    {
      if((*this)[i].isInstanceName(instanceName))
        return i;
    }
    
    return -1;
  };  
  
  PiPo *getPiPo(unsigned int index)
  {
    return (*this)[index].getPiPo();
  };  
  
  PiPo *getPiPo(const char *instanceName)
  {
    int index = getIndex(instanceName);
    
    if(index >= 0)
      return getPiPo(index);
    
    return NULL;
  };  
  
  int streamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->streamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames);
    
    return -1;
  };
  
  int reset(void)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->reset();
    
    return -1;
  };
  
  int frames(double time, float *values, unsigned int size, unsigned int num)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->frames(time, values, size, num);
    
    return -1;
  };
  
  int finalize(double inputEnd)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->finalize(inputEnd);
    
    return -1;
  };
};

#endif
