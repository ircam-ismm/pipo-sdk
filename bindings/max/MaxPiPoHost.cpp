/**
 *
 * @file MaxPiPoHost.cpp
 * @author Norbert.Schnell@ircam.fr
 *
 * @brief Max PiPo utilities
 * Copyright (C) 2012-2014 by IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 *
 */

#include <assert.h>

#include "MaxPiPoHost.h"

#include <string.h>
#include <vector>
#include "ext.h"	// for ext_dictionary.h and
#include "ext_dictobj.h"

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
  const char *dot = strrchr(attrName, '.');
  
  if(dot != NULL)
  {
    unsigned int pipoAttrNameLen = (unsigned int)(dot - attrName);
    
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
    case PiPo::Int:
    {
      for(unsigned int i = 0; i < attr->getSize(); i++)
        atom_setlong((*pat) + i, attr->getInt(i));
      
      break;
    }

    case PiPo::Enum:
    {
      if(attr->getIsArray() || attr->getIsVarSize())
      {
        vector<const char *> *enumList = attr->getEnumList();
        if(enumList != NULL && enumList->size() > 0)
          for(unsigned int i = 0; i < attr->getSize(); i++)
          {
            const char *enumStr = attr->getStr(i);
            if (enumStr)
              atom_setsym((*pat) + i, gensym(enumStr));
            else // no enum item string, leave as int
              atom_setlong((*pat) + i, attr->getInt(i));
          }
        else
          for(unsigned int i = 0; i < attr->getSize(); i++)
            atom_setlong((*pat) + i, attr->getInt(i));
      }
      else
      {
        for(unsigned int i = 0; i < attr->getSize(); i++)
          atom_setlong((*pat) + i, attr->getInt(i));
      }
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
    case PiPo::Dictionary:
    {
      for(unsigned int i = 0; i < attr->getSize(); i++)
        atom_setsym((*pat) + i, attr->getStr(i) ? gensym(attr->getStr(i)) : gensym("")); // why didn't this crash before the NULL check?
      
      break;
    }
      
    case PiPo::AtomType:
    {
      int attrsize = attr->getSize();

      for (int i = 0; i < attrsize; i++)
      {
        PiPo::Atom elem = attr->getAtom(i); // put attr element (of any type) into pipo Atom, either copying atom, or wrapping int or string
        // todo: define TYPE get(int) method for all pipo::attr derivations

        switch (elem.getType())
        {
          case PiPo::Double:
            atom_setfloat((*pat) + i, elem.getDouble());
            break;

          case PiPo::Int:
            atom_setlong((*pat) + i, elem.getInt());
            break;

          case PiPo::String:
            atom_setsym((*pat) + i, elem.getString() ? gensym(elem.getString()) : gensym("")); // why didn't this crash before the NULL check?
            break;

          default:
            break;
        }
      }


      /*
       if (attr->getIsArray() || attr->getIsVarSize())
      {
        PiPo::Atom *values =  attr->getPtr();

        for (unsigned int i = 0; i < attr->getSize(); i++)
        {

        PiPo::Type atomtype = attr->get(i)->getType();

        atom_setsym((*pat) + i, attr->getStr(i) ? gensym(attr->getStr(i)) : gensym("")); // why didn't this crash before the NULL check?

*/
    }
    break;

    default:
      break;
  }
}

/*********************************************************
 *
 *  Max PiPo Host Class
 *
 */
MaxPiPoHost::MaxPiPoHost(t_object *ext, const char *prefix)
: moduleFactory(ext, prefix), inputStreamAttrs(), outputStreamAttrs(),
  chain(this, &this->moduleFactory)
{
  this->ext = ext;
  systhread_mutex_new(&this->mutex, SYSTHREAD_MUTEX_RECURSIVE);
}

MaxPiPoHost::~MaxPiPoHost(void)
{
  systhread_mutex_free(this->mutex);
}

PiPo *MaxPiPoHost::setChainDescription(const char *str, PiPo *receiver)
{
  this->chain.clear();

#if USE_PIPO_GRAPH
  if (chain.create(str, false)) // does instantiate and wire to connect graph pipos
  {
    chain.setReceiver(receiver); // connect last module to receiving pipo
    return chain.getPiPo();
  }
#else
  if(this->chain.parse(str) > 0 && this->chain.instantiate() && this->chain.connect(receiver))
    return this->chain.getHead();
#endif
  return NULL;
}

/** declare one pipo module's attributes as max attributes */
void MaxPiPoHost::declarePiPoAttributes (PiPo *pipo, unsigned int iPiPo, const char *instanceName, MaxAttrGetterT getAttrMeth, MaxAttrSetterT setAttrMeth)
{
  unsigned int numAttrs = pipo->getNumAttrs();

  for (unsigned int iAttr = 0; iAttr < numAttrs; iAttr++)
  {
    PiPo::Attr *attr = pipo->getAttr(iAttr);

    if(attr != NULL && attr->getName() != NULL)
    {
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
        case PiPo::Dictionary:
          typeSym = USESYM(symbol);
          break;

        default:
          break;
      }
      bool isArray = attr->getIsArray();
      bool isVarSize = attr->getIsVarSize();

      t_object *maxAttr = NULL;
      if(!(isArray || isVarSize))
        maxAttr = attribute_new(attrName.c_str(), typeSym, 0, (method)getAttrMeth, (method)setAttrMeth);
      else
      {
        if(type == PiPo::Enum) typeSym = USESYM(atom);
        maxAttr = attr_offset_array_new(attrName.c_str(), typeSym, 1024, 0, (method)getAttrMeth, (method)setAttrMeth, 0, 0);
      }
      object_addattr(this->ext, maxAttr);

      if(type == PiPo::Bool)
        object_attr_addattr_parse(this->ext, attrName.c_str(), "style", USESYM(symbol), 0, "onoff");
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

          if(!(isArray || isVarSize))
          {
            object_attr_addattr_parse(this->ext, attrName.c_str(), "style", USESYM(symbol), 0, "enumindex");
            object_attr_addattr_parse(this->ext, attrName.c_str(), "enumvals", USESYM(symbol), 0, enumStr.c_str());
          }
        }
      }

      object_attr_addattr_format(this->ext, attrName.c_str(), "label", USESYM(symbol), 0, "s", gensym(label.c_str()));

      t_atom a;
      atom_setlong(&a, (iPiPo + 1) * 256 + iAttr);
      object_attr_addattr_atoms(this->ext, attrName.c_str(), "order", USESYM(long), 0, 1, &a);
    }
  }
} // end declarePiPoAttributes()

/** declare all pipo modules' attributes as max attributes */
void MaxPiPoHost::copyPiPoAttributes (MaxAttrGetterT getAttrMeth, MaxAttrSetterT setAttrMeth)
{
  for (unsigned int iPiPo = 0; iPiPo < this->chain.getSize(); iPiPo++)
  {
    PiPo *pipo = this->chain.getPiPo(iPiPo);
    const char *instanceName = this->chain.getInstanceName(iPiPo);

    declarePiPoAttributes(pipo, iPiPo, instanceName, getAttrMeth, setAttrMeth);
  }
}

void
MaxPiPoHost::getMaxAttr(const char *attrName, long *pac, t_atom **pat, PiPoChain *chain)
{
  if(pac != NULL && pat != NULL)
  {
    char instanceName[maxWordLen];
    char pipoAttrName[maxWordLen];
    
    this->lock();
    
    if(chain == NULL)
      chain = &this->chain;
    
    *pac = 0;
    
    if(getPiPoInstanceAndAttrName(attrName, instanceName, pipoAttrName))
    {
      PiPo *pipo = chain->getPiPo(instanceName);
      
      if(pipo != NULL)
      {
        PiPo::Attr *attr = pipo->getAttr(pipoAttrName);
        
        if(attr != NULL)
        {
          unsigned int attrSize = attr->getSize();
          char alloc;
          
          if(atom_alloc_array(attrSize, pac, pat, &alloc) == MAX_ERR_NONE)
          {
            getMaxAttributeList(pipo, attr->getIndex(), pac, pat);
            *pac = attrSize;
          }
        }
      }
    }
    
    this->unlock();
  }
}

void MaxPiPoHost::setMaxAttr(const char *attrName, long ac, t_atom *at, PiPoChain *chain, bool silently)
{
  char instanceName[maxWordLen];
  char pipoAttrName[maxWordLen];
  
  this->lock();
  
  if(chain == NULL)
    chain = &this->chain;
  
  if(getPiPoInstanceAndAttrName(attrName, instanceName, pipoAttrName))
  {
    PiPo *pipo = chain->getPiPo(instanceName);
    
    if(pipo != NULL)
    {
      PiPo::Attr *attr = pipo->getAttr(pipoAttrName);
      
      if(attr != NULL)
      {
        for(int i = 0; i < ac; i++)
        {
          if(!atom_isnum(at + i) && !atom_issym(at + i))
            ac = i;
        }
        
        if (ac > 0  ||  (attr->getIsVarSize()  &&  ac >= 0)) // check for at least one arg when not varsize, accept 0 args when varsize (empty list)
        {
          attr->setSize((unsigned int)ac);
          
          if (ac == 0  ||  atom_isnum(at)  ||  atom_issym(at))
          {
            for(int i = 0; i < ac; i++) {
              if(atom_issym(at + i)) {
                attr->set(i, (const char *)mysneg(atom_getsym(at + i)), true);
              } else if(atom_islong(at + i)) {
                attr->set(i, (int)atom_getlong(at + i), true);
              } else if(atom_isfloat(at + i)) {
                attr->set(i, (double)atom_getfloat(at + i), true);
              } else {
                attr->set(i, 0, true);
              }
            }
            
            if (attr->getType() == PiPo::Dictionary)
            { // check if attr value is valid max dictionary and convert into json string
              //assert(attr->getSize() == 1); //YAGNI: make a list of json strings
              PiPoDictionaryAttr *dictattr = dynamic_cast<PiPoDictionaryAttr *>(attr);
              
              t_dictionary *dict = dictobj_findregistered_retain(atom_getsym(at));
              if (dict != NULL)
              {
                t_object    *jsonwriter = (t_object *) object_new(_sym_nobox, _sym_jsonwriter);
                t_handle     json;
                
                object_method(jsonwriter, _sym_writedictionary, dict);
                object_method(jsonwriter, _sym_getoutput, &json);
                
                // now const char *str = *json contains our JSON serialization of the t_dictionary
                dictattr->setJson(*json);
                
                object_free(jsonwriter);
                dictobj_release(dict);
              }
              else // attr. value is not a dictionary, we assume it is json and copy it
                dictattr->setJson(mysneg(atom_getsym(at)));
            }
            
            attr->changed(silently);
          }
          else
            object_error(this->ext, "invalid argument for attribute %s", attrName);
        }
        else
          object_error(this->ext, "missing argument for attribute %s", attrName);
      }
    }
  }
  
  this->unlock();
}

void MaxPiPoHost::propagateInputAttributes(void)
{
  this->lock();
  
  PiPo *head = this->chain.getHead();
  
  if(head != NULL)
  {
    const char *colNameStr[PIPO_MAX_LABELS];
    const char **labels = NULL;
    unsigned int numCols = this->inputStreamAttrs.dims[0];
    unsigned int numLabels = this->inputStreamAttrs.numLabels;
    
    if(numLabels > PIPO_MAX_LABELS)
      numLabels = PIPO_MAX_LABELS;
    
    if(numLabels > numCols)
      numLabels = numCols;
    
    if(numLabels > 0)
    {
      for(unsigned int i = 0; i < numLabels; i++)
        colNameStr[i] = mysneg(this->inputStreamAttrs.labels[i]);
      
      for(unsigned int i = numLabels; i < numCols; i++)
        colNameStr[i] = "unnamed";
      
      labels = colNameStr;
    }
    
    head->streamAttributes(this->inputStreamAttrs.hasTimeTags,
                           this->inputStreamAttrs.rate,
                           this->inputStreamAttrs.offset,
                           this->inputStreamAttrs.dims[0],
                           this->inputStreamAttrs.dims[1],
                           labels,
                           this->inputStreamAttrs.hasVarSize,
                           this->inputStreamAttrs.domain,
                           this->inputStreamAttrs.maxFrames);
  }
  
  this->unlock();
}

void MaxPiPoHost::setOutputAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
{
  this->lock();
  
  if(labels != NULL)
  {
    int numLabels = width;
    
    if(numLabels > PIPO_MAX_LABELS)
      numLabels = PIPO_MAX_LABELS;
    
    for(int i = 0; i < numLabels; i++)
      this->outputStreamAttrs.labels[i] = gensym(labels[i]);
    
    this->outputStreamAttrs.numLabels = width;
  }
  else
    this->outputStreamAttrs.numLabels = 0;
  
  this->outputStreamAttrs.hasTimeTags = hasTimeTags;
  this->outputStreamAttrs.rate = rate;
  this->outputStreamAttrs.offset = offset;
  this->outputStreamAttrs.dims[0] = width;
  this->outputStreamAttrs.dims[1] = size;
  this->outputStreamAttrs.hasVarSize = hasVarSize;
  this->outputStreamAttrs.domain = domain;
  this->outputStreamAttrs.maxFrames = maxFrames;
  
  this->unlock();
}

void MaxPiPoHost::streamAttributesChanged(PiPo *pipo, PiPo::Attr *attr)
{
  this->propagateInputAttributes();
}

void MaxPiPoHost::signalError(PiPo *pipo, std::string errorMsg)
{
  // std::string msg ="error in pipo chain " + chain.getString() + ": " + errorMsg; // don't know if it is overkill to print the chain text, one can double click to get there
  std::string msg ="error in PiPo chain: " + errorMsg;
  object_error(this->ext, msg.c_str()); 
}

void MaxPiPoHost::signalWarning(PiPo *pipo, std::string errorMsg)
{
  // std::string msg ="warning in pipo chain " + chain.getString() + ": " + errorMsg;  // don't know if it is overkill to print the chain text
  std::string msg ="warning in PiPo chain: " + errorMsg;
  object_warn(this->ext, msg.c_str());
}

void MaxPiPoHost::setInputDims(int width, int size, bool propagate)
{
  this->lock();
  
  this->inputStreamAttrs.dims[0] = width;
  this->inputStreamAttrs.dims[1] = size;
  
  if(propagate)
    this->propagateInputAttributes();
  
  this->unlock();
}

void MaxPiPoHost::setInputLabels(long ac, t_atom *at, bool propagate)
{
  this->lock();
  
  if(ac > PIPO_MAX_LABELS)
    ac = PIPO_MAX_LABELS;
  
  this->inputStreamAttrs.numLabels = (unsigned int)ac;
  
  for(int i = 0; i < ac; i++)
  {
    if(atom_issym(at + i))
      this->inputStreamAttrs.labels[i] = atom_getsym(at + i);
    else
    {
      this->inputStreamAttrs.numLabels = i;
      break;
    }
  }
  
  if(propagate)
    this->propagateInputAttributes();
  
  this->unlock();
}

void MaxPiPoHost::setInputHasTimeTags(int hasTimeTags, bool propagate)
{
  this->lock();
  
  this->inputStreamAttrs.hasTimeTags = hasTimeTags;
  
  if(propagate)
    this->propagateInputAttributes();
  
  this->unlock();
}

#define MIN_SAMPLERATE (1.0 / 31536000000.0) /* once a year */
#define MAX_SAMPLERATE (96000000000.0)

void MaxPiPoHost::setInputFrameRate(double sampleRate, bool propagate)
{
  if(sampleRate <= MIN_SAMPLERATE)
    this->inputStreamAttrs.rate = MIN_SAMPLERATE;
  else if(sampleRate >= MAX_SAMPLERATE)
    this->inputStreamAttrs.rate = MAX_SAMPLERATE;
  else
    this->inputStreamAttrs.rate = sampleRate;
  
  if(propagate)
    this->propagateInputAttributes();
}

void MaxPiPoHost::setInputFrameOffset(double sampleOffset, bool propagate)
{
  this->inputStreamAttrs.offset = sampleOffset;
  
  if(propagate)
    this->propagateInputAttributes();
}

void MaxPiPoHost::setInputFrameDomain(double domain, bool propagate)
{
  this->inputStreamAttrs.domain = domain;

  if(propagate)
    this->propagateInputAttributes();
}

void MaxPiPoHost::setInputMaxFrames(int maxFrames, bool propagate)
{
  this->inputStreamAttrs.maxFrames = maxFrames;
  
  if(propagate)
    this->propagateInputAttributes();
}

void MaxPiPoHost::getInputDims(int &width, int &size)
{
  width = this->inputStreamAttrs.dims[0];
  size = this->inputStreamAttrs.dims[1];
}

void MaxPiPoHost::getInputLabels(int &num, t_atom *array)
{
  int numLabels = this->inputStreamAttrs.numLabels;
  
  if(num > numLabels)
    num = numLabels;
  
  for(int i = 0; i < num; i++)
    atom_setsym(array + i, this->inputStreamAttrs.labels[i]);
}

bool MaxPiPoHost::getInputHasTimeTags()
{
  return this->inputStreamAttrs.hasTimeTags;
}

double MaxPiPoHost::getInputFrameRate(void)
{
  return this->inputStreamAttrs.rate;
}

double MaxPiPoHost::getInputFrameOffset(void)
{
  return this->inputStreamAttrs.offset;
}

double MaxPiPoHost::getInputFrameDomain(void)
{
  return this->inputStreamAttrs.domain;
}

void MaxPiPoHost::getOutputDims(int &width, int &size)
{
  width = this->outputStreamAttrs.dims[0];
  size = this->outputStreamAttrs.dims[1];
}

void MaxPiPoHost::getOutputLabels(int &num, t_atom *array)
{
  int numLabels = this->outputStreamAttrs.numLabels;
  
  if(num > numLabels)
    num = numLabels;
  
  for(int i = 0; i < num; i++)
    atom_setsym(array + i, this->outputStreamAttrs.labels[i]);
}

bool MaxPiPoHost::getOutputHasTimeTags()
{
  return this->outputStreamAttrs.hasTimeTags;
}

double MaxPiPoHost::getOutputFrameRate(void)
{
  return this->outputStreamAttrs.rate;
}

double MaxPiPoHost::getOutputFrameOffset(void)
{
  return this->outputStreamAttrs.offset;
}

int MaxPiPoHost::getOutputMaxFrames()
{
  return this->outputStreamAttrs.maxFrames;
}

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset:2
 * End:
 */
