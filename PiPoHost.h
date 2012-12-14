/**
 *
 * @file PiPoHost.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Plugin Interface for Processing Objects 0.1 (experimental)
 * 
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _PIPO_HOST_
#define _PIPO_HOST_

#include "MaxPiPo.h"
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
    this->module = other.module;
  };
  
  ~PiPoOp(void)
  {
    if(this->module != NULL)
      delete this->module;
    
    if(this->pipo != NULL)
      delete pipo;
  }

  PiPo *instantiate(PiPoModuleFactory *moduleFactory, unsigned int index, PiPoDecr &descr)
  {
    this->descr = &descr;
    this->module = moduleFactory->createModule(index, descr.pipoName, descr.instanceName, this->pipo);    
    return this->pipo;
  };
  
  PiPo *getPiPo()
  {
    return this->pipo;
  }
};

class PiPoChain : public vector<PiPoOp>
{
  PiPoModuleFactory *moduleFactory;

public:
  PiPoChain(PiPoModuleFactory *moduleFactory) : vector<PiPoOp>()
  { 
    this->moduleFactory = moduleFactory;
  };  
  
  ~PiPoChain(void) { };
  
  PiPo *instantiate(PiPoChainDescr chainDescr, PiPo *receiver)
  {
    PiPo *pipo = NULL;
    
    this->clear();
    
    if(chainDescr.size() > 0)
    {
      this->resize(chainDescr.size());
      
      for(int i = this->size() - 1; i >= 0; i--)
      {
        pipo = (*this)[i].instantiate(this->moduleFactory, i, chainDescr[i]);
        
        if(pipo == NULL)
          return NULL;        
        
        if(receiver != NULL)
        {
          pipo->setReceiver(receiver);
          receiver = pipo;
        }
      }
    }
    
    return pipo;
  }  
};

#endif
