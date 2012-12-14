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
#include "MaxPiPoHost.h"

#include <string.h>
#include <vector>

using namespace std;

/*********************************************************
 *
 *  Max PiPo Host Utils
 *
 */
static const unsigned int maxWordLen = 256;

static bool
getPiPoInstanceAndAttrName(const char *attrName, char *instanceName, char *pipoAttrName)
{
  char *dot = strrchr(attrName, '.');
  
  if(dot != NULL)
  {
    unsigned int pipoAttrNameLen = dot - attrName;
    
    strcpy(pipoAttrName, dot + 1);
    
    if(pipoAttrNameLen > maxWordLen)
      pipoAttrNameLen = maxWordLen;
    
    strncpy(instanceName, attrName, pipoAttrNameLen);
    instanceName[pipoAttrNameLen] = '\0';
    
    return true;
  }
  
  return false;
}

static void
getMaxAttributeList(PiPo *pipo, unsigned int attrId, long *pac, t_atom **pat)
{
  enum PiPoAttrDef::Type type = pipo->getAttrType(attrId);
  
  switch(type)
  {
    case PiPoAttrDef::Undefined:
      break;
      
    case PiPoAttrDef::Bool:
    case PiPoAttrDef::Int:
    {
      vector<int> values;
      pipo->getAttrVal(attrId, values);
      
      for(unsigned int i = 0; i < values.size(); i++)
        atom_setlong((*pat) + i, values[i]);
      
      break;
    }
    case PiPoAttrDef::Float:
    case PiPoAttrDef::Double:
    {
      vector<double> values;
      pipo->getAttrVal(attrId, values);
      
      for(unsigned int i = 0; i < values.size(); i++)
        atom_setfloat((*pat) + i, values[i]);
      
      break;
    }
      
    case PiPoAttrDef::String:
    {
      vector<const char *> values;
      pipo->getAttrVal(attrId, values);
      
      for(unsigned int i = 0; i < values.size(); i++)
      {
        if(values[i] != NULL)
          atom_setsym((*pat) + i, gensym(values[i]));
        else
          atom_setsym((*pat) + i, gensym(""));
      }
      
      break;
    }
      
    default:
      break;      
  }
}

static t_max_err 
getPiPoAttr(MaxPiPoHostT *self, void *attr, long *pac, t_atom **pat)
{
  if(pac != NULL && pat != NULL) 
  {
    const char *attrName = mysneg((t_symbol *)object_method((t_object *)attr, gensym("getname")));
    char instanceName[maxWordLen];
    char pipoAttrName[maxWordLen];
    
    *pac = 0;
    
    if(getPiPoInstanceAndAttrName(attrName, instanceName, pipoAttrName))
    {
      PiPo *pipo = self->host.getPiPo(instanceName);
      
      if(pipo != NULL)
      {
        int attrId = pipo->getAttrId(pipoAttrName);
        
        if(attrId >= 0)
        {
          maxPiPoHost_lock(self);
          
          unsigned int attrSize = pipo->getAttrSize(attrId);
          char alloc;
          
          if(atom_alloc_array(attrSize, pac, pat, &alloc) == MAX_ERR_NONE)
          {
            getMaxAttributeList(pipo, attrId, pac, pat);
            *pac = attrSize;
          }
          
          maxPiPoHost_unlock(self);
        }
      }
    }
  }
  
  return MAX_ERR_NONE;
}
  
static t_max_err 
setPiPoAttr(MaxPiPoHostT *self, void *attr, long ac, t_atom *at)
{
  const char *attrName = mysneg((t_symbol *)object_method((t_object *)attr, gensym("getname")));
  char instanceName[maxWordLen];
  char pipoAttrName[maxWordLen];
  
  if(getPiPoInstanceAndAttrName(attrName, instanceName, pipoAttrName))
  {
    PiPo *pipo = self->host.getPiPo(instanceName);
    
    if(pipo != NULL)
    {
      int attrId = pipo->getAttrId(pipoAttrName);
      
      if(attrId >= 0)
      {
        if(ac > 0)
        {
          maxPiPoHost_lock(self);
          
          if(atom_isnum(at))
          {
            vector<double> values(ac);
            
            for(int i = 0; i < ac; i++)
            {
              if(atom_isnum(at + i))
                values[i] = atom_getfloat(at + i);
            }
            
            pipo->setAttrVal(attrId, values);
            
            if(pipo->attrChangesStream(attrId))
              pipo->streamAttributesChanged();
          }
          else if(atom_issym(at))
          {
            vector<const char *> values(ac);
            
            for(int i = 0; i < ac; i++)
            {
              if(atom_issym(at + i))
                values[i] = (const char *)mysneg(atom_getsym(at + i));
            }
            
            pipo->setAttrVal(attrId, values);
            
            if(pipo->attrChangesStream(attrId))
              pipo->streamAttributesChanged();
          }
          else
            object_error((t_object *)self, "invalid argument for attribute %s", attrName);
          
          maxPiPoHost_unlock(self);
        }
        else
          object_error((t_object *)self, "missing argument for attribute %s", attrName);
        
      }
    }
  }
  
  return MAX_ERR_NONE;
}
  
static void
copyPiPoAttributesToMaxObj(t_object *obj, PiPo *pipo, const char *instanceName, unsigned int pipoIndex)
{
  unsigned int numAttrs = pipo->getNumAttrs();
  
  for(unsigned int i = 0; i < numAttrs; i++)
  {
    /* attribute name */
    string attrName = instanceName;
    attrName += ".";
    attrName += pipo->getAttrName(i);
    
    /* description */
    string label = pipo->getAttrDescr(i);
    label += " (";
    label += instanceName;
    label += ")";

    enum PiPoAttrDef::Type type = pipo->getAttrType(i);
    t_symbol *typeSym = NULL;

    switch(type)
    {
      case PiPoAttrDef::Undefined:
        break;
        
      case PiPoAttrDef::Bool:
        typeSym = USESYM(long);
        break;
        
      case PiPoAttrDef::Int:
        typeSym = USESYM(long);
        break;
        
      case PiPoAttrDef::Float:
        typeSym = USESYM(float32);
        break;
        
      case PiPoAttrDef::Double:
        typeSym = USESYM(float64);
        break;
        
      case PiPoAttrDef::String:
        typeSym = USESYM(symbol);
        break;
        
      default:
        break;      
    }
    
    t_object *attr = attribute_new(attrName.c_str(), typeSym, 0, (method)getPiPoAttr, (method)setPiPoAttr);
    object_addattr(obj, attr);
    
    const char *attrEnum = pipo->getAttrEnum(i);

    if(type == PiPoAttrDef::Bool)
      object_attr_addattr_parse(obj, attrName.c_str(), "style", USESYM(symbol), 0, "onoff");
    else if(type == PiPoAttrDef::Int && attrEnum != NULL)
    {
      object_attr_addattr_parse(obj, attrName.c_str(), "style", USESYM(symbol), 0, "enumindex");
      object_attr_addattr_parse(obj, attrName.c_str(), "enumvals", USESYM(symbol), 0, attrEnum);
    }
    
    object_attr_addattr_format(obj, attrName.c_str(), "label", USESYM(symbol), 0, "s", gensym(label.c_str()));
    
    t_atom a;
    atom_setlong(&a, (pipoIndex + 1) * 256 + i);
    object_attr_addattr_atoms(obj, attrName.c_str(), "order", USESYM(long), 0, 1, &a);
  }
}

/*********************************************************
 *
 *  Max PiPo Host Class
 *
 */
PiPo * 
MaxPiPoHost::setChainDescription(const char *str, PiPo *receiver)
{
  this->chain.clear();
  
  if(this->chainDescr.parse(str) > 0)
    return this->chain.instantiate(this->chainDescr, receiver);
  
  return NULL;
}

void
MaxPiPoHost::copyPiPoAttributes(t_object *maxObj)
{
  for(unsigned int i = 0; i < this->chain.size(); i++)
  {
    PiPo *pipo = this->chain[i].getPiPo();
    const char *instanceName = this->chainDescr[i].getInstanceName();
    
    copyPiPoAttributesToMaxObj(maxObj, pipo, instanceName, i);
  }
}

PiPo *
MaxPiPoHost::getPiPo(const char *instanceName)
{
  int index = this->chainDescr.getIndex(instanceName);
  
  if(index >= 0)
    return this->chain[index].getPiPo();
  
  return NULL;
}

/*********************************************************
 *
 *  Max PiPo Host External
 *
 */
void
maxPiPoHost_init(MaxPiPoHostT *self)
{
  new(&self->host) MaxPiPoHost();
}

void
maxPiPoHost_deinit(MaxPiPoHostT *self)
{
  self->host.~MaxPiPoHost();
}
