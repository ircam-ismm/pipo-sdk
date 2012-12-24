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

#define atom_isnum(a) ((a)->a_type == A_LONG || (a)->a_type == A_FLOAT)
#define atom_issym(a) ((a)->a_type == A_SYM)
#define atom_isobj(p) ((p)->a_type == A_OBJ) 
#define mysneg(s) ((s)->s_name)

#define atom_islong(p) ((p)->a_type == A_LONG) 
#define atom_isfloat(p) ((p)->a_type == A_FLOAT) 
#define atom_getnum(p) (((p)->a_type == A_FLOAT)? (double)((p)->a_w.w_float): (double)((p)->a_w.w_long))
#define atom_setvoid(p) ((p)->a_type = A_NOTHING)

typedef struct MaxPiPoHostSt MaxPiPoHostT;
typedef void (*MaxPiPoAttrChangedMethodT)(MaxPiPoHostT *maxPiPoHost, unsigned int pipoIndex, PiPo::Attr *attr);

class MaxPiPoHost
{
  class MaxPiPoModuleFactory : public PiPoModuleFactory
  { 
    MaxPiPoHostT *maxPiPoHost;
    MaxPiPoAttrChangedMethodT attrChangedMethod;
    
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
    MaxPiPoModuleFactory(MaxPiPoHostT *maxPiPoHost)
    { 
      this->maxPiPoHost = maxPiPoHost;
      this->attrChangedMethod = (MaxPiPoAttrChangedMethodT)object_getmethod(maxPiPoHost, gensym("attrChanged"));
    };
    
    ~MaxPiPoModuleFactory(void) { }
    
    PiPo *create(unsigned int index, const std::string &pipoName, const std::string &instanceName, PiPoModule *&module)
    {
      std::string pipoClassNameStr = "pipo." + pipoName;
      MaxPiPoT *maxPiPo = (MaxPiPoT *)object_new_typed(CLASS_NOBOX, gensym(pipoClassNameStr.c_str()), 0, NULL);
      
      if(maxPiPo != NULL)
      {
        module = new MaxPiPoModule(maxPiPo);
        return maxPiPo->pipo;
      }
      
      object_error((t_object *)this->maxPiPoHost, "cannot find external module pipo.%s", pipoName.c_str());
      return NULL;
    }
    
    void attrChanged(unsigned int pipoIndex, PiPo::Attr *attr) 
    { 
      if(this->attrChangedMethod != NULL)
        (*this->attrChangedMethod)(this->maxPiPoHost, pipoIndex, attr); 
    };
  };
  
  MaxPiPoHostT *maxPiPoHost;
  MaxPiPoModuleFactory moduleFactory;
  PiPoChain chain;
  
public:
  MaxPiPoHost(MaxPiPoHostT *maxPiPoHost) : moduleFactory(maxPiPoHost), chain(&this->moduleFactory)
  {
    this->maxPiPoHost = maxPiPoHost;
  }

  PiPo *setChainDescription(const char *str, PiPo *receiver);
  
  typedef t_max_err (*MaxAttrGetterT)(MaxPiPoHostT *self, void *attr, long *pac, t_atom **pat);
  typedef t_max_err (*MaxAttrSetterT)(MaxPiPoHostT *self, void *attr, long ac, t_atom *at);
  void copyPiPoAttributes(MaxAttrGetterT getAttrMeth, MaxAttrSetterT setAttrMeth);

  void getMaxAttr(PiPoChain *chain, const char *attrName, long *pac, t_atom **pat);
  void setMaxAttr(PiPoChain *chain, const char *attrName, long ac, t_atom *at);
  
  PiPoChain *getChain();
};

struct MaxPiPoHostSt
{
  t_pxobject head;
  MaxPiPoHost host;
  t_systhread_mutex mutex;
};

void maxPiPoHost_init(MaxPiPoHostT *self);
void maxPiPoHost_deinit(MaxPiPoHostT *self);

void maxPiPoHost_lock(MaxPiPoHostT *self);
bool maxPiPoHost_trylock(MaxPiPoHostT *self);
void maxPiPoHost_unlock(MaxPiPoHostT *self);

#endif
