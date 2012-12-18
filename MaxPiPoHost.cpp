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
  PiPo::Attr *attr = pipo->getAttr(attrId);
  enum PiPo::Type type = attr->getType();
  
  switch(type)
  {
    case PiPo::Undefined:
      break;
      
    case PiPo::Bool:
    case PiPo::Enum:
    case PiPo::Int:
    {
      for(unsigned int i = 0; i < attr->getSize(); i++)
        atom_setlong((*pat) + i, attr->getInt(i));
      
      break;
    }
    case PiPo::Float:
    case PiPo::Double:
    {
      for(unsigned int i = 0; i < attr->getSize(); i++)
        atom_setfloat((*pat) + i, attr->getDbl(i));
      
      break;
    }
    case PiPo::String:
    {
      for(unsigned int i = 0; i < attr->getSize(); i++)
        atom_setsym((*pat) + i, gensym(attr->getStr(i)));
      
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
        PiPo::Attr *attr = pipo->getAttr(pipoAttrName);
        
        if(attr != NULL)
        {
          maxPiPoHost_lock(self);
          
          unsigned int attrSize = attr->getSize();
          char alloc;
          
          if(atom_alloc_array(attrSize, pac, pat, &alloc) == MAX_ERR_NONE)
          {
            getMaxAttributeList(pipo, attr->getIndex(), pac, pat);
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
      PiPo::Attr *attr = pipo->getAttr(pipoAttrName);
      
      if(attr != NULL)
      {
        if(ac > 0)
        {
          maxPiPoHost_lock(self);
          
          attr->setSize(ac);
          
          if(atom_isnum(at))
          {
            for(int i = 0; i < ac; i++)
            {
              if(atom_islong(at + i))
                attr->set(i, (int)atom_getlong(at + i));
              else if(atom_isfloat(at + i))
                attr->set(i, (double)atom_getfloat(at + i));
              else                
                attr->set(i, 0);
            }
            
            if(attr->doesChangeStream())
              pipo->streamAttributesChanged();
          }
          else if(atom_issym(at))
          {
            for(int i = 0; i < ac; i++)
            {
              if(atom_issym(at + i))
                attr->set(i, (const char *)mysneg(atom_getsym(at + i)));
              else
                attr->set(i, NULL);
            }
            
            if(attr->doesChangeStream())
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
    PiPo::Attr *attr = pipo->getAttr(i);
    
    /* attribute name */
    string attrName = instanceName;
    attrName += ".";
    attrName += attr->getName();
    
    /* description */
    string label = attr->getDescr();
    label += " (";
    label += instanceName;
    label += ")";

    enum PiPo::Type type = attr->getType();
    t_symbol *typeSym = NULL;

    switch(type)
    {
      case PiPo::Undefined:
        break;
        
      case PiPo::Bool:
      case PiPo::Enum:
      case PiPo::Int:
        typeSym = USESYM(long);
        break;
        
      case PiPo::Float:
        typeSym = USESYM(float32);
        break;
        
      case PiPo::Double:
        typeSym = USESYM(float64);
        break;
        
      case PiPo::String:
        typeSym = USESYM(symbol);
        break;
        
      default:
        break;
    }
    
    t_object *maxAttr = attribute_new(attrName.c_str(), typeSym, 0, (method)getPiPoAttr, (method)setPiPoAttr);
    object_addattr(obj, maxAttr);
    
    if(type == PiPo::Bool)
      object_attr_addattr_parse(obj, attrName.c_str(), "style", USESYM(symbol), 0, "onoff");
    else if(type == PiPo::Enum)
    {      
      vector<const char *> *enumList = attr->getEnumList();
      
      if(enumList != NULL && enumList->size() > 0)
      {
        string enumStr = (*enumList)[0];
        
        for(unsigned int i = 1; i < enumList->size(); i++)
        {
          enumStr += " ";
          enumStr += (*enumList)[i];
        }
        
        object_attr_addattr_parse(obj, attrName.c_str(), "style", USESYM(symbol), 0, "enumindex");
        object_attr_addattr_parse(obj, attrName.c_str(), "enumvals", USESYM(symbol), 0, enumStr.c_str());
      }
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
  
  if(this->chainDescr.parse(str) > 0 && this->chain.set(&this->chainDescr, receiver))
     return this->chain.getHead();
  
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

PiPoChain *
MaxPiPoHost::getChain(void)
{
  return &this->chain;
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
