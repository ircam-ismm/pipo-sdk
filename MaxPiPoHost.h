/**
 *
 * @file MaxPiPoHost.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Max PiPo utilities
 * Copyright (C) 2012-2014 by IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _MAX_PIPO_HOST_
#define _MAX_PIPO_HOST_

#include "PiPoHost.h"
#include "MaxPiPo.h"
#include "ext.h"
#include "ext_obex.h"
#include "ext_systhread.h"
#include "z_dsp.h"
#include "ext_systhread.h"
#include "z_dsp.h"

#include <string>
#include <vector>

#define atom_isnum(a) ((a)->a_type == A_LONG || (a)->a_type == A_FLOAT)
#define atom_issym(a) ((a)->a_type == A_SYM)
#define atom_isobj(p) ((p)->a_type == A_OBJ) 
#define mysneg(s) ((s)->s_name)

#define atom_islong(p) ((p)->a_type == A_LONG) 
#define atom_isfloat(p) ((p)->a_type == A_FLOAT) 
#define atom_getnum(p) (((p)->a_type == A_FLOAT)? (double)((p)->a_w.w_float): (double)((p)->a_w.w_long))
#define atom_setvoid(p) ((p)->a_type = A_NOTHING)

class MaxPiPoHost : public PiPo::Parent
{
  class MaxPiPoModuleFactory : public PiPoModuleFactory
  { 
    t_object *ext;
    
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
    MaxPiPoModuleFactory(t_object *ext)
    { 
      this->ext = ext;
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
      
      object_error((t_object *)this->ext, "cannot find external module pipo.%s", pipoName.c_str());
      return NULL;
    }
  };
  
#define PIPO_MAX_LABELS 64
  
  class PiPoStreamAttributes
  {
  public:
    int hasTimeTags;
    double rate;
    double offset;
    unsigned int dims[2];
    t_symbol *labels[PIPO_MAX_LABELS];
    unsigned int numLabels;
    bool hasVarSize;
    double domain;
    unsigned int maxFrames;
    
    PiPoStreamAttributes(void) : labels()
    {
      this->hasTimeTags = false;
      this->rate = 1000.0;
      this->offset = 0.0;
      this->dims[0] = 1;
      this->dims[1] = 1;
      this->numLabels = 0;
      this->hasVarSize = false;
      this->domain = 0.0;
      this->maxFrames = 1;
    };
  };
  
  t_object *ext;
  MaxPiPoModuleFactory moduleFactory;
  PiPoChain chain;
  PiPoStreamAttributes inputStreamAttrs;
  PiPoStreamAttributes outputStreamAttrs;
  t_systhread_mutex mutex;

public:
  MaxPiPoHost(t_object *ext);
  ~MaxPiPoHost(void);

  void lock(void);
  bool trylock(void);
  void unlock(void);

  PiPoChain *getChain(void) { return &this->chain; };
  PiPo *setChainDescription(const char *str, PiPo *receiver);
  
  typedef t_max_err (*MaxAttrGetterT)(t_object *ext, void *attr, long *pac, t_atom **pat);
  typedef t_max_err (*MaxAttrSetterT)(t_object *ext, void *attr, long ac, t_atom *at);
  void copyPiPoAttributes(MaxAttrGetterT getAttrMeth, MaxAttrSetterT setAttrMeth);

  void getMaxAttr(const char *attrName, long *pac, t_atom **pat, PiPoChain *chain = NULL);
  void setMaxAttr(const char *attrName, long ac, t_atom *at, PiPoChain *chain = NULL, bool silently = false);
  
  void propagateInputAttributes(void);
  void setOutputAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames);

  void streamAttributesChanged(PiPo *pipo, PiPo::Attr *attr);
  void signalError(PiPo *pipo, std::string errorMsg);
  void signalWarning(PiPo *pipo, std::string errorMsg);
  
  void setInputDims(int width, int size, bool propagate = true);
  void setInputLabels(long ac, t_atom *at, bool propagate = true);
  void setInputHasTimeTags(int hasTimeTags, bool propagate = true);
  void setInputFrameRate(double sampleRate, bool propagate = true);
  void setInputFrameOffset(double sampleOffset, bool propagate = true);
  void setInputMaxFrames(int maxFrames, bool propagate = true);
  
  void getInputDims(int &width, int &size);
  void getInputLabels(int &num, t_atom *array);
  bool getInputHasTimeTags(void);
  double getInputFrameRate(void);
  double getInputFrameOffset(void);

  void getOutputDims(int &width, int &size);
  void getOutputLabels(int &num, t_atom *array);
  bool getOutputHasTimeTags(void);
  double getOutputFrameRate(void);
  double getOutputFrameOffset(void);
  int getOutputMaxFrames();
};

#endif
