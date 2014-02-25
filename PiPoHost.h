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
};

class PiPoOp
{ 
  unsigned int index;
  std::string pipoName;
  std::string instanceName;
  PiPo *pipo;
  PiPoModule *module;
  
public:
  PiPoOp(unsigned int index = 0) : pipoName(), instanceName()
  {
    this->index = index;
    this->pipo = NULL;
    this->module = NULL;
  };
  
//  PiPoOp(const PiPoOp &other) : pipoName(), instanceName()
//  { 
//    this->index = other.index;
//    this->pipoName = other.pipoName;
//    this->instanceName = other.instanceName;
//    this->pipo = other.pipo;
//    this->module = other.module;
//  };
  
  ~PiPoOp(void) { };

  void clear(void)
  {
    if(this->module != NULL)
      delete this->module;
    
    this->module = NULL;
    
    if(this->pipo != NULL)
      delete pipo;
    
    this->pipo = NULL;
  };
  
  void parse(std::string str, size_t &pos)
  {
    this->clear();
    
    size_t end = str.find_first_of(':', pos);
    size_t open = str.find_first_of('(', pos);
    size_t closed = str.find_first_of(')', pos);
    
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
  
  bool instantiate(PiPo::Parent *parent, PiPoModuleFactory *moduleFactory)
  {
    this->pipo = NULL;
    this->module = NULL;
    
    if(moduleFactory != NULL)
      this->pipo = moduleFactory->create(index, this->pipoName, this->instanceName, this->module);
    
    if(this->pipo != NULL)
    {
      this->pipo->setParent(parent);
      return true;
    }
    
    return false;
  };

  void set(unsigned int index, PiPo::Parent *parent, PiPoModuleFactory *moduleFactory, const PiPoOp &other)
  {
    this->index = index;
    this->pipoName = other.pipoName;
    this->instanceName = other.instanceName;
    
    this->instantiate(parent, moduleFactory);
    
    if(this->pipo != NULL)
      this->pipo->cloneAttrs(other.pipo);
  };
  
  PiPo *getPiPo() { return this->pipo; };
  const char *getInstanceName() { return this->instanceName.c_str(); };
  bool isInstanceName(const char *str) { return (this->instanceName.compare(str) == 0); };
};

class PiPoChain : public PiPo
{
  std::vector<PiPoOp> ops;
  PiPo::Parent *parent;
  PiPoModuleFactory *moduleFactory;

public:
  PiPoChain(PiPo::Parent *parent, PiPoModuleFactory *moduleFactory = NULL) : PiPo(parent), ops()
  {
    this->parent = parent;
    this->moduleFactory = moduleFactory;
  };  
  
  PiPoChain(const PiPoChain &other) : PiPo(other), ops()
  { 
    this->moduleFactory = NULL;
    this->connect(NULL);
  };  
  
  const PiPoChain& operator=(const PiPoChain &other)
  {
    unsigned int numOps = other.ops.size();
    
    this->moduleFactory = other.moduleFactory;
    
    this->ops.clear();
    this->ops.resize(numOps);
    
    for(unsigned int i = 0; i < numOps; i++)
      this->ops[i].set(i, this->parent, this->moduleFactory, other.ops[i]);
    
    this->connect(NULL);
    this->moduleFactory = NULL;

    return *this;
  };
  
  ~PiPoChain(void) 
  { 
    this->clear();
  };
  
  int getSize()
  {
    return this->ops.size();
  }
  
  void clear()
  {
    for(unsigned int i = 0; i < this->ops.size(); i++)
      this->ops[i].clear();

    this->ops.clear();
  }
  
  size_t parse(const char *str)
  {
    std::string pipoChainStr = str;
    size_t pos = 0;
    
    this->clear();
    
    for(unsigned int i = 0; pos < std::string::npos; i++)
    {
      this->ops.resize(i + 1, i); 
      this->ops[i].parse(pipoChainStr, pos);
    }
    
    return this->ops.size();
  };
    
  bool instantiate(void)
  {
    if(this->ops.size() > 0)
    {
      for(unsigned int i = 0; i < this->ops.size(); i++)
      {
        if(!this->ops[i].instantiate(this->parent, this->moduleFactory))
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
      
      for(int i = this->ops.size() - 2; i >= 0; i--)
      {
        PiPo *pipo = this->ops[i].getPiPo();
        pipo->setReceiver(next);
        next = pipo;
      }
      
      return true;
    }
    
    return false;
  };
  
  void setReceiver(PiPo *receiver)
  {
    if(this->ops.size() > 0)
    {
      PiPo *tail = this->getTail();
      
      if(tail != NULL)
        tail->setReceiver(receiver);
    }
  };
  
  PiPo *getHead(void)
  {
    if(this->ops.size() > 0)
      return this->ops[0].getPiPo();
    
    return NULL;
  };
  
  PiPo *getTail(void)
  {
    if(this->ops.size() > 0)
      return this->ops[this->ops.size() - 1].getPiPo();
    
    return NULL;
  };
  
  int getIndex(const char *instanceName)
  {
    for(unsigned int i = 0; i < this->ops.size(); i++)
    {
      if(this->ops[i].isInstanceName(instanceName))
        return i;
    }
    
    return -1;
  };  
  
  PiPo *getPiPo(unsigned int index)
  {
    if(index < this->ops.size())
      return this->ops[index].getPiPo();
    
    return NULL;
  };  
  
  PiPo *getPiPo(const char *instanceName)
  {
    int index = getIndex(instanceName);
    
    if(index >= 0)
      return getPiPo(index);
    
    return NULL;
  };  
  
  const char *getInstanceName(unsigned int index) 
  { 
    if(index < this->ops.size())
      return this->ops[index].getInstanceName(); 
    
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
  
  int frames(double time, double weight, float *values, unsigned int size, unsigned int num)
  {
    PiPo *head = this->getHead();
    
    if(head != NULL)
      return head->frames(time, weight, values, size, num);
    
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
