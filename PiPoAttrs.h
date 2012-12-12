/**
 *
 * @file PiPoAttrs.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Plugin Interface for Processing Objects 0.1 (experimental)
 * 
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _PIPO_ATTRS_
#define _PIPO_ATTRS_

#include <vector>
using namespace std;

class PiPoAttr;

/***********************************************
 *
 *  PiPo Attribute Definition
 *
 */
class PiPoAttrDef
{
public:  
  enum Type { Undefined, Bool, Int, Float, Double, String };
  
  const char *name; /**< attribute name */
  enum Type type; /**< data type */
  unsigned int size; /**< max number of elements (O: unlimited) */
  const char *enumItems; /**< enum items */
  bool changesStream; /**< flag whether attribute changes stream attributes */
  const char *descr; /**< short description */
  
  PiPoAttrDef(const char *name, enum Type type, unsigned int size, const char *enumItems, bool changesStream, const char *descr)
  {
    this->name = name;
    this->type = type;
    this->size = size;
    this->enumItems = enumItems;
    this->changesStream = changesStream;
    this->descr = descr;
  };
  
  inline PiPoAttr *instantiate(void);
};

//static int
//getNumEnumItems(const char *enumItems)
//{
//  if(enumItems != NULL)
//  {
//    const char *item = enumItems;
//    int num = 0;
//    
//    while(item != NULL)
//    {
//      item = strchr(item + 1, ' ');
//      num++;
//    }
//    
//    return num;
//  }
//  
//  return 0;
//}

static int
getEnumItemIndex(const char *enumItems, const char *str, int returnWhenFailed = -1)
{
  if(enumItems != NULL)
  {
    const char *token = strstr(enumItems, str);
    
    if(token != NULL)
    {
      const char *item = enumItems;
      int index = 0;
      
      while(item != NULL && item != token)
      {
        item = strchr(item, ' ');
        index++;
        
        if(item != NULL)
          item++;        
      }
      
      if(item == token)
        return index;
    }
  }
  
  return -1;
}

/***********************************************
 *
 *  PiPo Attribute
 *
 */
class PiPoAttr
{
public:
  PiPoAttr(PiPoAttrDef *def) { this->def = def; };
  ~PiPoAttr(void) {  };
  
  const char *getName(void) { return def->name; };
  enum PiPoAttrDef::Type getType(void) { return def->type; };
  bool changesStream(void) { return def->changesStream; };
  const char *getEnum(void) { return def->enumItems; };
  const char *getDescr(void) { return def->descr; };
  
  virtual unsigned int getSize(void) = 0;
  virtual bool setSize(unsigned int size, double initVal) = 0;
  
  virtual bool set(int value) = 0;
  virtual bool set(double value) = 0;
  virtual bool set(const char *value) = 0;
  virtual bool set(vector<int> &value) = 0;
  virtual bool set(vector<double> &value) = 0;
  virtual bool set(vector<const char *> &value) = 0;

  virtual bool get(int &value) = 0;
  virtual bool get(double &value) = 0;
  virtual bool get(const char *&value) = 0;
  virtual bool get(vector<int> &value) = 0;
  virtual bool get(vector<double> &value) = 0;
  virtual bool get(vector<const char *> &value) = 0;
  
  //virtual bool get(bool &value) { return this->get((int)value); };
  //virtual bool get(unsigned int &value) { return this->get((int)value); };
  
  bool getBool() { int val; return (this->get(val))? (bool)val: false; };
  int getInt() { int val; return (this->get(val))? val: 0; };
  int getFloat() { double val; return (this->get(val))? (float)val: 0.0; };
  int getDouble() { double val; return (this->get(val))? val: 0.0; };
  const char *getString() { const char *val; return (this->get(val))? val: NULL; };

  void destroyDef(void) { delete this->def; };
  void rename(const char *name) { this->def->name = name; };

private:
  PiPoAttrDef *def;
};

/***********************************************
 *
 *  Scalar Attribute
 *
 */
/* int & float scalar */
template <typename TYPE>
class PiPoScalarAttribute : public PiPoAttr
{
private:
  TYPE value;
  
public:
  PiPoScalarAttribute(PiPoAttrDef *def) : PiPoAttr(def)
  {
    this->value = (TYPE)0;
  }
  
  unsigned int getSize(void) { return 1; };
  bool setSize(unsigned int size, double initVal) { return false;  };
  
  bool set(int value)
  {
    this->value = (TYPE)value;
    return true;
  };
  
  bool set(double value)
  {
    this->value = (TYPE)value;
    return true;
  };
  
  bool set(const char *value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = getEnumItemIndex(enumItems, value);
      
      if(index >= 0)
      {
        this->value = (unsigned int)index;
        return true;
      }    
    }
    
    return false;
  };
  
  bool set(vector<int> &value)
  {
    if(value.size() > 0)
    {
      this->value = (TYPE)value[0];
      return true;
    }
    
    return false;
  }
  
  bool set(vector<double> &value)
  {
    if(value.size() > 0)
    {
      this->value = (TYPE)value[0];
      return true;
    }
    
    return false;
  }
  
  bool set(vector<const char *> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = getEnumItemIndex(enumItems, value[0]);
      
      if(index >= 0)
      {
        this->value = (TYPE)index;
        return true;
      }
    }

    return false;
  }  
  
  bool get(int &value)
  {
    value = (int)this->value;
    return true;
  }
  
  bool get(double &value)
  {
    value = (double)this->value;
    return true;
  }
  
  bool get(const char *&value) { return false; };
  
  bool get(vector<int> &value)
  {
    value.assign(1, (int)this->value);
    return true;
  }
  
  bool get(vector<double> &value)
  {
    value.assign(1, (double)this->value);
    return true;
  }
  
  bool get(vector<const char *> &value) { return false; };
};

/* const char * scalar */
template <>
class PiPoScalarAttribute <const char *> : public PiPoAttr
{
private:
  const char *value;
  
public:
  PiPoScalarAttribute(PiPoAttrDef *def) : PiPoAttr(def)
  {
    this->value = NULL;
  }

  unsigned int getSize(void) { return 1; };
  bool setSize(unsigned int size, double initVal) { return false;  };
  
  bool set(int value) { return false; }; /* cannot set string from number */
  bool set(double value) { return false; }; /* cannot set string from number */
  
  bool set(const char *value)
  {
    this->value = value;
    return true;
  };
  
  bool set(vector<int> &value) { return false; }; /* cannot set string from number */
  bool set(vector<double> &value) { return false; }; /* cannot set string from number */
  
  bool set(vector<const char *> &value)
  {
    if(value.size() > 0)
    {
      this->value = value[0];
      return true;
    }
    
    return false;
  }  
  
  bool get(int &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = getEnumItemIndex(enumItems, this->value);
      
      if(index >= 0)
      {
        value = (int)index;
        return true;
      }    
    }
    
    return false;
  }
  
  bool get(double &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = getEnumItemIndex(enumItems, this->value);
      
      if(index >= 0)
      {
        value = (double)index;
        return true;
      }    
    }
    
    return false;
  }
  
  bool get(const char *&value)
  {
    value = this->value;
    return false;
  }
  
  bool get(vector<int> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = getEnumItemIndex(enumItems, this->value);
      
      if(index >= 0)
      {
        value.assign(1, (int)index);
        return true;
      }    
    }
    
    return false;
  }
  
  bool get(vector<double> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = getEnumItemIndex(enumItems, this->value);
      
      if(index >= 0)
      {
        value.assign(1, (double)index);
        return true;
      }    
    }
    
    return false;
  }
  
  bool get(vector<const char *> &value)  
  {
    value.assign(1, this->value);
    return true;
  }    
};

/***********************************************
 *
 *  Array Attribute
 *
 */
/* int & float array */
template <typename TYPE>
class PiPoArrayAttribute : public PiPoAttr
{
private:
  TYPE *value;
  unsigned int size;
  
public:
  PiPoArrayAttribute(PiPoAttrDef *def) : PiPoAttr(def)
  {
    unsigned int size = def->size;
    
    if(size < 1)
      size = 1;
    
    this->value = (TYPE *)malloc(size * sizeof(TYPE));
    this->size = size;
    
    for(unsigned int i = 0; i < size; i++)
      this->value[i] = (TYPE)0;
  }
  
  ~PiPoArrayAttribute(void)
  {
    free(this->value);
  }
  
  unsigned int getSize(void) { return this->size; }
  
  bool setSize(unsigned int size, double initVal) 
  { 
    this->value = (TYPE *)realloc(this->value, size * sizeof(TYPE));
    this->size = size;
    
    return true;
  };
  
  bool set(int value)
  {
    this->value[0] = (TYPE)value;
    return true;
  };
  
  bool set(double value)
  {
    this->value[0] = (TYPE)value;
    return true;
  };
  
  bool set(const char *value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = (TYPE)getEnumItemIndex(enumItems, value);
      
      if(index >= 0)
      {
        this->value[0] = (TYPE)index;
        return true;
      }
    }
    
    return false;
  };
  
  bool set(vector<int> &value)
  {
    unsigned int num = value.size();
    
    if(num > this->size)
      num = this->size;
    
    for(unsigned int i = 0; i < num; i++)
      this->value[i] = (TYPE)value[i];

    return true;
  }
  
  bool set(vector<double> &value)
  {
    unsigned int num = value.size();
    
    if(num > this->size)
      num = this->size;
    
    for(unsigned int i = 0; i < num; i++)
      this->value[i] = (TYPE)value[i];

    return true;
  }
  
  bool set(vector<const char *> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      unsigned int num = value.size();
      
      if(num > this->size)
        num = this->size;
      
      for(unsigned int i = 0; i < num; i++)
        this->value[i] = (TYPE)getEnumItemIndex(enumItems, value[i]);

      return true;
    }
    
    return false;
  }
  
  bool get(int &value)
  {
    value = (int)this->value[0];
    return true;
  }
  
  bool get(double &value)
  {
    value = (double)this->value[0];
    return true;
  }
  
  bool get(const char *&value) { return false; };
  
  bool get(vector<int> &value)
  {
    value.resize(this->size);
    
    for(unsigned int i = 0; i < this->size; i++)
      value[i] = (int)this->value[i];
    
    return true;
  }
  
  bool get(vector<double> &value)
  {
    value.resize(this->size);
    
    for(unsigned int i = 0; i < this->size; i++)
      value[i] = (double)this->value[i];
    
    return true;
  }
  
  bool get(vector<const char *> &value) { return false; };
};

/* const char * array */
template <>
class PiPoArrayAttribute <const char *> : public PiPoAttr
{
private:
  const char **value;
  unsigned int size;
  
public:
  PiPoArrayAttribute(PiPoAttrDef *def) : PiPoAttr(def)
  {
    unsigned int size = def->size;
    
    if(size < 1)
      size = 1;
    
    this->value = (const char **)malloc(size * sizeof(const char *));
    this->size = size;
    
    for(unsigned int i = 0; i < size; i++)
      this->value[i] = NULL;
  }
  
  ~PiPoArrayAttribute(void)
  {
    free(this->value);
  }
  
  unsigned int getSize(void) { return this->size; }
  
  bool setSize(unsigned int size, double initVal) 
  { 
    this->value = (const char **)realloc(this->value, size * sizeof(const char *));
    this->size = size;
    
    return true;
  };
  
  bool set(int value) { return false; }; /* cannot set string from number */
  bool set(double value) { return false; }; /* cannot set string from number */
  
  bool set(const char *value)
  {
    this->value[0] = value;
    return true;
  };
  
  bool set(vector<int> &value) { return false; }; /* cannot set string from number */
  bool set(vector<double> &value) { return false; }; /* cannot set string from number */
  
  bool set(vector<const char *> &value)
  {
    unsigned int num = value.size();
    
    if(num > this->size)
      num = this->size;
    
    for(unsigned int i = 0; i < num; i++)
      this->value[i] = value[i];

    return true;
  }  
  
  bool get(int &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = getEnumItemIndex(enumItems, this->value[0]);
      
      if(index >= 0)
      {
        value = (int)index;
        return true;
      }    
    }
    
    return false;
  }
  
  bool get(double &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      int index = getEnumItemIndex(enumItems, this->value[0]);
      
      if(index >= 0)
      {
        value = (double)index;
        return true;
      }    
    }
    
    return false;
  }
  
  bool get(const char *&value)
  {
    value = this->value[0];
    return true;
  }
  
  bool get(vector<int> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      value.resize(this->size);
      
      for(unsigned int i = 0; i < this->size; i++)
      {
        int index = getEnumItemIndex(enumItems, this->value[i]);
        
        if(index < 0)
          index = 0;
        
        value[i] = (int)index;
        return true;
      }
    }
    
    return false;
  }
  
  bool get(vector<double> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      value.resize(this->size);
      
      for(unsigned int i = 0; i < this->size; i++)
      {
        int index = getEnumItemIndex(enumItems, this->value[i]);
        
        if(index < 0)
          index = 0;
        
        value[i] = (double)index;
        
        return true;
      }
    }
    
    return false;
  }
  
  bool get(vector<const char *> &value)  
  {
    value.resize(this->size);
    
    for(unsigned int i = 0; i < this->size; i++)
      value[i] = this->value[i];
    
    return true;
  }  
};

/***********************************************
 *
 *  Var Size Attribute
 *
 */
/* int & float vector */
template <typename TYPE>
class PiPoVarSizeAttribute : public PiPoAttr
{
private:
  vector<TYPE> value;
  
public:
  PiPoVarSizeAttribute(PiPoAttrDef *def) : PiPoAttr(def), value()
  {
  }

  unsigned int getSize(void)
  {
    return this->value.size();
  }
  
  bool setSize(unsigned int size, double initVal)
  {
    this->value.resize(size, (TYPE)initVal);
    return true;
  }
  
  bool set(int value)
  {
    this->value.assign(1, (TYPE)value);
    return true;
  };
  
  bool set(double value)
  {
    this->value.assign(1, (TYPE)value);
    return true;
  };
  
  bool set(const char *value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      this->value.assign(1, (TYPE)getEnumItemIndex(enumItems, value));
      return true;
    }
    
    return false;
  };
  
  bool set(vector<int> &value)
  {
    unsigned int num = value.size();
    
    this->value.resize(num);
    
    for(unsigned int i = 0; i < num; i++)
      this->value[i] = (TYPE)value[i];

    return true;
  }
  
  bool set(vector<double> &value)
  {
    unsigned int num = value.size();
    
    this->value.resize(num);
    
    for(unsigned int i = 0; i < num; i++)
      this->value[i] = (TYPE)value[i];

    return true;
  }
  
  bool set(vector<const char *> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      unsigned int num = value.size();
      
      this->value.resize(num);
            
      for(unsigned int i = 0; i < num; i++)
        this->value[i] = (TYPE)getEnumItemIndex(enumItems, value[i]);
      
      return true;
    }
    
    return false;
  }  
  
  bool get(int &value)
  {
    value = (int)this->value[0];
    return true;
  }
  
  bool get(double &value)
  {
    value = (double)this->value[0];
    return true;
  }
  
  bool get(const char *&value) { return false; };
  
  bool get(vector<int> &value)
  {
    unsigned int num = this->value.size();
    
    value.resize(num);
    
    for(unsigned int i = 0; i < num; i++)
      value[i] = (int)this->value[i];
    
    return true;
  }
  
  bool get(vector<double> &value)
  {
    unsigned int num = this->value.size();
    
    value.resize(num);
    
    for(unsigned int i = 0; i < num; i++)
      value[i] = (double)this->value[i];
    
    return true;
  }
  
  bool get(vector<const char *> &value) { return false; };
};

/* const char * vector */
template <>
class PiPoVarSizeAttribute <const char *> : public PiPoAttr
{
private:
  vector<const char *> value;
  
public:
  PiPoVarSizeAttribute(PiPoAttrDef *def) : PiPoAttr(def), value()
  {
  }
  
  unsigned int getSize(void)
  {
    return this->value.size();
  }
  
  bool setSize(unsigned int size, double initVal)
  {
    this->value.resize(size, "");
    return true;
  }
  
  bool set(int value) { return false; }; /* cannot set string from number */
  bool set(double value) { return false; }; /* cannot set string from number */
  
  bool set(const char *value)
  {
    this->value.assign(1, value);
    return true;
  };
  
  bool set(vector<int> &value) { return false; }; /* cannot set string from number */
  bool set(vector<double> &value) { return false; }; /* cannot set string from number */
  
  bool set(vector<const char *> &value)
  {
    unsigned int num = value.size();
    
    this->value.resize(num);
    
    for(unsigned int i = 0; i < num; i++)
      this->value[i] = value[i];

    return true;
  }  
  
  bool get(int &value)
  {
    if(this->value.size() > 0)
    {
      const char *enumItems = this->getEnum();
      
      if(enumItems != NULL)
      {
        int index = getEnumItemIndex(enumItems, this->value[0]);
        
        if(index >= 0)
        {
          value = (int)index;
          return true;
        }    
      }
    }
    
    return false;
  }
  
  bool get(double &value)
  {
    if(this->value.size() > 0)
    {
      const char *enumItems = this->getEnum();
      
      if(enumItems != NULL)
      {
        int index = getEnumItemIndex(enumItems, this->value[0]);
        
        if(index >= 0)
        {
          value = (double)index;
          return true;
        }    
      }
    }
    
    return false;
  }
  
  bool get(const char *&value)
  {
    if(this->value.size() > 0)
    {
      value = this->value[0];
      return true;
    }
    
    return false;
  }
  
  bool get(vector<int> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      unsigned int num = this->value.size();
      
      value.resize(num);
      
      for(unsigned int i = 0; i < num; i++)
      {
        int index = getEnumItemIndex(enumItems, this->value[i]);
        
        if(index < 0)
          index = 0;
        
        value[i] = (int)index;
        
        return true;
      }
    }
    
    return false;
  }
  
  bool get(vector<double> &value)
  {
    const char *enumItems = this->getEnum();
    
    if(enumItems != NULL)
    {
      unsigned int num = this->value.size();
      
      value.resize(num);
            
      for(unsigned int i = 0; i < num; i++)
      {
        int index = getEnumItemIndex(enumItems, this->value[i]);
        
        if(index < 0)
          index = 0;
        
        value[i] = (double)index;
        
        return true;
      }
    }
    
    return false;
  }
  
  bool get(vector<const char *> &value)  
  {
    unsigned int num = this->value.size();
    
    value.resize(num);
    
    for(unsigned int i = 0; i < num; i++)
      value[i] = this->value[i];
    
    return true;
  }  
};

PiPoAttr *
PiPoAttrDef::instantiate(void)
{
  switch (this->type) 
  {
    case Undefined:
      return NULL;
      
    case Bool:
    {
      if(this->size == 1)
        return new PiPoScalarAttribute<int>(this);
      else if(this->size == 0)
        return new PiPoVarSizeAttribute<int>(this);
      else
        return new PiPoArrayAttribute<int>(this);
    }
      
    case Int:
    {
      if(this->size == 1)
        return new PiPoScalarAttribute<int>(this);
      else if(this->size == 0)
        return new PiPoVarSizeAttribute<int>(this);
      else
        return new PiPoArrayAttribute<int>(this);
    }
      
    case Float:
    {
      if(this->size == 1)
        return new PiPoScalarAttribute<float>(this);
      else if(this->size == 0)
        return new PiPoVarSizeAttribute<float>(this);
      else
        return new PiPoArrayAttribute<float>(this);
    }
      
    case Double:
    {
      if(this->size == 1)
        return new PiPoScalarAttribute<double>(this);
      else if(this->size == 0)
        return new PiPoVarSizeAttribute<double>(this);
      else
        return new PiPoArrayAttribute<double>(this);
    }
      
    case String:
    {
      if(this->size == 1)
        return new PiPoScalarAttribute<const char *>(this);
      else if(this->size == 0)
        return new PiPoVarSizeAttribute<const char *>(this);
      else
        return new PiPoArrayAttribute<const char *>(this);
    }
      
    default:
      break;
  }
  
  return NULL;
}

#endif
