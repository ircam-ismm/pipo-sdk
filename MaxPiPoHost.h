/**
 *
 * @file MaxPiPoHost.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Max PiPo utilities
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _MAX_PIPO_HOST_
#define _MAX_PIPO_HOST_

#include "PiPoHost.h"
#include "MaxPiPo.h"
#include "ext_systhread.h"
#include "z_dsp.h"

#include <string.h>
#include <vector>
#include <new>

using namespace std;

#define atom_isnum(a) ((a)->a_type == A_LONG || (a)->a_type == A_FLOAT)
#define atom_issym(a) ((a)->a_type == A_SYM)
#define atom_isobj(p) ((p)->a_type == A_OBJ) 
#define mysneg(s) ((s)->s_name)

#define atom_islong(p) ((p)->a_type == A_LONG) 
#define atom_isfloat(p) ((p)->a_type == A_FLOAT) 
#define atom_getnum(p) (((p)->a_type == A_FLOAT)? (double)((p)->a_w.w_float): (double)((p)->a_w.w_long))
#define atom_setvoid(p) ((p)->a_type = A_NOTHING)

typedef struct MaxPiPoHostSt MaxPiPoHostT;

class MaxPiPoHost
{
  class MaxPiPoModuleFactory : public PiPoModuleFactory
  { 
    MaxPiPoHost *host;
    
    class MaxPiPoModule : public PiPoModule
    {
      MaxPiPoT *maxPiPo;
      
    public:
      MaxPiPoModule(MaxPiPoT *maxPiPo) 
      { 
        this->maxPiPo = maxPiPo;
      };
      
      MaxPiPoModule(void) 
      { 
        if(this->maxPiPo != NULL)
          object_free((t_object *)this->maxPiPo);
      };
    };

  public:
    MaxPiPoModuleFactory(MaxPiPoHost *host)
    { 
      this->host = host;
    };
    
    ~MaxPiPoModuleFactory(void) { }
    
    PiPoModule *createModule(unsigned int index, string &pipoName, string &instanceName, PiPo *&pipo)
    {
      string pipoClassNameStr = "pipo." + pipoName;
      MaxPiPoT *maxPiPo = (MaxPiPoT *)object_new_typed(CLASS_NOBOX, gensym(pipoClassNameStr.c_str()), 0, NULL);
      
      if(maxPiPo != NULL)
      {
        pipo = maxPiPo->pipo;
        return new MaxPiPoModule(maxPiPo);
      }
      
      object_error((t_object *)this->host, "cannot find external module pipo.%s", pipoName.c_str());
      return NULL;
    }
  };
  
  t_systhread_mutex mutex;
  MaxPiPoModuleFactory moduleFactory;
  PiPoChainDescr chainDescr;
  PiPoChain chain;
  
public:
  MaxPiPoHost(void) : moduleFactory(this), chainDescr(), chain(&this->moduleFactory)
  {
    systhread_mutex_new(&this->mutex, SYSTHREAD_MUTEX_RECURSIVE);
  }

  ~MaxPiPoHost(void)
  {
    systhread_mutex_free(this->mutex);  
  }
  
  void lock(void) { systhread_mutex_lock(this->mutex); };
  void unlock(void) { systhread_mutex_unlock(this->mutex); };
  
  PiPo *setChainDescription(const char *str, PiPo *receiver);
  void copyPiPoAttributes(t_object *maxObj);
  PiPo *getPiPo(const char *pipoInstanceName);
  PiPoChain *getChain();
};

struct MaxPiPoHostSt
{
  t_pxobject head;
  MaxPiPoHost host;
};

#define maxPiPoHost_lock(h) ((MaxPiPoHostT *)(h))->host.lock()
#define maxPiPoHost_unlock(h) ((MaxPiPoHostT *)(h))->host.unlock()

void maxPiPoHost_init(MaxPiPoHostT *self);
void maxPiPoHost_deinit(MaxPiPoHostT *self);

#endif
