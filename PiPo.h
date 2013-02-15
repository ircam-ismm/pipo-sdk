/**
 *
 * @file PiPo.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Plugin Interface for Processing Objects
 * 
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _PIPO_
#define _PIPO_

#include <string>
#include <vector>
#include <functional>
#include <cstring>
#include <typeinfo>
#include <map>

class PiPo
{
public:
  enum SegmentSignal { Start, End, CouldStart, CouldEnd, Abort };
  class Attr;
  
private:
  std::vector<PiPo *> receivers; /**< list of receivers */
  std::vector<Attr *> attrs; /**< list of attributes */
  float *weightPtr; /**< pointer to current weight (used in ) */
  
public:
  PiPo(PiPo *receiver = NULL) : receivers(), attrs() 
  { 
    if(receiver != NULL)
      this->receivers.push_back(receiver);
    
    this->weightPtr = NULL;
  };
  
  ~PiPo(void) { };
  
  /**
   * @brief Propagates a module's output stream attributes to its reciever.
   *
   * This method is called in the streamAttributes() method of a PiPo module.
   *
   * @param hasTimeTags a boolean representing whether the elements of the stream are time-tagged
   * @param rate the frame rate (highest average rate for time-tagged streams)
   * @param offset the lag of the output stream relative to the input
   * @param width the frame width (also number of channels or matrix columns)
   * @param size the frame size (or number of matrix rows)
   * @param labels optional labels for the frames' channels or columns
   * @param hasVarSize a boolean representing whether the frames have a variable size (respecting the given frame size as maximum)
   * @param domain extent of a frame in the given domain (e.g. duration or frequency range)
   * @param maxFrames maximum number of frames in a block exchanged between two modules
   * @return used as return value of the calling method
   */
  int propagateStreamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames) 
  { 
    int ret = -1;
    
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->streamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames);
      
      if(ret < 0)
        break;
    }
    
    return ret;
  };
  
  /**
   * @brief Propagates a segmentation module's segment attributes to its reciever.
   *
   * This method is called in the segmentAttributes() method of a PiPo module.
   *
   * @return used as return value of the calling method
   */
  int propagateSegmentAttributes(double maxRate) 
  { 
    int ret = -1;
    
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->segmentAttributes(maxRate);
      
      if(ret < 0)
        break;
    }
    
    return ret;
  };
  
  /**
   * @brief Propagates the reset control event.
   *
   * This method is called in the reset() method of a PiPo module.
   *
   * @return used as return value of the calling method
   */
  int propagateReset(void) 
  { 
    int ret = -1;
    
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->reset();
      
      if(ret < 0)
        break;
    }
    
    return ret;
  }
  
  /**
   * @brief Propagates a module's output frames to its reciever.
   *
   * This method is called in the frames() method of a PiPo module.
   *
   * @param time time-tag for a single frame or a block of frames
   * @param values interleaved frames values, row by row (interleaving channels or columns), frame by frame
   * @param size size of eaqch of all frames
   * @param num number of frames
   * @return used as return value of the calling method
   */
  int propagateFrames(double time, float *values, unsigned int size, unsigned int num) 
  { 
    int ret = -1;
    
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->frames(time, values, size, num); 
      
      if(ret < 0)
        break;
    }
    
    return ret;
  }
  
  /**
   * @brief Propagates a segmentation module's output segment start and end to its reciever.
   *
   * This method is called in the frames() method of a PiPo ssegmentation module.
   *
   * @param time time-tag for a single frame or a block of frames
   * @param start start (start=true) or end (start=false) segment
   * @return used as return value of the calling method
   */
  int propagateSegment(double time, enum SegmentSignal signal)
  { 
    int ret = -1;
    
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->segment(time, signal);
      
      if(ret < 0)
        break;
    }
    
    return ret;
  }
  
  /**
   * @brief Propagates the finalize control event.
   *
   * This method is called in the finalize() method of a PiPo module.
   *
   * @return used as return value of the calling method
   */
  int propagateFinalize(double inputEnd) 
  { 
    int ret = -1;
    
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->finalize(inputEnd);
      
      if(ret < 0)
        break;
    }
    
    return ret;
  }
  
  /**
   * @brief Gets a PiPo modules receiver (call only by the PiPo host)
   *
   * @return receiver PiPo module receiving this module's output stream
   */
  PiPo *getReceiver(unsigned int index = 0) 
  { 
    if(index < this->receivers.size())
      return this->receivers[index];
    
    return NULL;
  };
  
  /**
   * @brief Sets a PiPo modules receiver (call only by the PiPo host)
   *
   * @param receiver PiPo module receiving this module's output stream
   * @param add receiver (versus clear and set first)
   */
  virtual void setReceiver(PiPo *receiver, bool add = false) 
  { 
    if(add)
    {
      if(receiver != NULL)
        this->receivers.push_back(receiver);
    }
    else
    {
      this->receivers.clear();
      
      if(receiver != NULL)
        this->receivers.push_back(receiver);
      
    };
  }
  
  /**
   * @brief Adds a PiPo modules receiver (call only by the PiPo host)
   *
   * @param receiver PiPo module receiving this module's output stream
   */
  virtual void addReceiver(PiPo *receiver) 
  { 
  };
  
  /**
   * @brief Sets a PiPo modules weight pointer (for weighted accumulation/integration)
   *
   * @param ptr pointer to the current weight
   */
  virtual void setWeightPtr(float *ptr) 
  { 
    this->weightPtr = ptr;
  };
  
  /**
   * @brief Configures a PiPo module according to the input stream attributes and propagate output stream attributes
   *
   * PiPo module:
   * Any implementation of this method requires a propagateStreamAttributes() method call and returns its return value, typically like this:
   *
   * \code{return this->propagateStreamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames);}
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the final output stream attributes and usally returns 0.
   *
   * @param hasTimeTags   a boolean representing whether the elements of the stream are time-tagged
   * @param rate          the frame rate (highest average rate for time-tagged streams, sample rate for audio input)
   * @param offset        the lag of the output stream relative to the input
   * @param width         the frame width (also number of channels for audio or matrix columns)
   * @param size          the frame size (or number of matrix rows, always 1 for audio)
   * @param labels        optional labels for the frames' channels or columns
   * @param hasVarSize    a boolean representing whether the frames have a variable size (respecting the given frame size as maximum)
   * @param domain        extent of a frame in the given domain (e.g. duration or frequency range)
   * @param maxFrames     maximum number of frames in a block exchanged between two modules (window size for audio)
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int streamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int size, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames) = 0;
  
  /**
   * @brief Configures a PiPo module according to the input stream attributes and propagate output stream attributes
   *
   * PiPo module:
   * Any implementation of this method requires a propagateSegmentAttributes() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the final output segment attributes and usally returns 0.
   *
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int segmentAttributes(double maxRate) 
  { 
    return -1; 
  };
  
  /**
   * @brief Resets processing (optional)
   *
   * PiPo module:
   * Any implementation of this method requires a propagateReset() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host usally simply returns 0.
   *
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int reset(void) 
  { 
    return this->propagateReset(); 
  };
  
  /**
   * @brief Processes a single frame or a block of frames
   *
   * PiPo module:
   * Generally, an implementation of this method requires a propagateFrames() method call, typically like this:
   *
   * \code{return this->propagateFrames(time, values, size, num); }
   *
   * Exceptions are segmentation modules that propagate segments instead of frames 
   * as well as integration modules that accumulate frames over a segment and call propagateFrames() 
   * for an integrated frame in the segment() method.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the received frames and usally returns 0.
   *
   * @param time    time-tag for a single frame or a block of frames
   * @param values  interleaved frames values, row by row (interleaving channels or columns), frame by frame
   * @param size    size of each of all frames (number of channels for audio)
   * @param num     number of frames (number of samples for audio input)
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int frames(double time, float *values, unsigned int size, unsigned int num) = 0;
  
  /**
   * @brief Starts or ends a segment
   *
   * PiPo module:
   * Any implementation of this method requires a propagateFrames() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the received frames and usally returns 0.
   *
   * @param time time-tag for segment start or end
   * @param start start (start=true) or end (start=false) segment
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int segment(double time, enum SegmentSignal signal) 
  { 
    return -1;
  };
  
  /**
   * @brief Finalizes processing (optinal)
   *
   * PiPo module:
   * Any implementation of this method requires a propagateFinalize() method call and returns its return value.
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host usally simply returns 0.
   *
   * @param inputEnd end time of the finalized input stream
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int finalize(double inputEnd) 
  {
    return this->propagateFinalize(inputEnd); 
  };

  /**
   * @brief Signals that the output stream parameters of a given module have changed
   *
   * PiPo module:
   * This method is called (with 0) in method setting a module parameter that requires changing the output stream attributes.
   *
   * PiPo host:
   * The implementation of this method by the terminating receiver module provided by a PiPo host
   * would repropagate the input stream attributes by calling streamAttributes() of the first module.
   *
   * param unitId (for host) index of the module that initially called streamAttributesChanged().
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int streamAttributesChanged(unsigned int unitId = 0) 
  { 
    int ret = -1;
    
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->streamAttributesChanged(unitId + 1); 
      
      if(ret < 0)
        break;
    }
    
    return ret;
  }
  
  /***********************************************
   *
   *  PiPo Attributes
   *
   */
public:
  enum Type { Undefined, Bool, Enum, Int, Float, Double, String };
  enum Enumerate { };
  
  class Attr
  {
  private:
    unsigned int index;
    const char *name; /**< attribute name */
    const char *descr; /**< short description */
    enum Type type;
    bool changesStream;
    
  public:
    /**
     * PiPo attribute base class
     */
    Attr(PiPo *pipo, const char *name, const char *descr, const std::type_info *type, bool changesStream) 
    { 
      this->index = pipo->attrs.size();
      this->name = name;
      this->descr = descr;
      
      if(type == &typeid(bool))
        this->type = Bool;
      else if(type == &typeid(enum Enumerate))
        this->type = Enum;
      else if(type == &typeid(int))
        this->type = Int;
      else if(type == &typeid(float))
        this->type = Float;
      else if(type == &typeid(double))
        this->type = Double;
      else if(type == &typeid(std::string) || type == &typeid(const char *))
        this->type = String;
      else
        this->type = Undefined;

      this->changesStream = changesStream;

      pipo->attrs.push_back(this);
    };
    
    ~Attr(void) { };
    
    void setIndex(unsigned int index) { this->index = index; };
    void setName(const char *name) { this->name = name; };
    void setDescr(const char *descr) { this->descr = descr; };
    
    unsigned int getIndex(void) { return this->index; };
    const char *getName(void) { return this->name; };
    const char *getDescr(void) { return this->descr; };
    enum Type getType(void) { return this->type; };
    bool doesChangeStream(void) { return this->changesStream; };
    
    virtual void clone(Attr *other) = 0;

    virtual unsigned int setSize(unsigned int size) = 0;
    virtual unsigned int getSize(void) = 0;
    
    virtual void set(unsigned int i, int val) = 0;
    virtual void set(unsigned int i, double val) = 0;
    virtual void set(unsigned int i, const char *val) = 0;
    virtual int getInt(unsigned int i) = 0;
    virtual double getDbl(unsigned int i) = 0;
    virtual const char *getStr(unsigned int i) = 0;
    
    virtual std::vector<const char *> *getEnumList(void) { return NULL; };
    
    void rename(const char *name) { this->name = name; };
  };
  
  /**
   * PiPo enumerator attribute base class
   */
  class EnumAttr : public Attr
  {
    struct strCompare : public std::binary_function<const char *, const char *, bool> { 
      bool operator() (const char *str1, const char *str2) const { return std::strcmp(str1, str2) < 0; } 
    };
    
    unsigned int numEnumItems;
    std::vector<const char *>enumList;
    std::map<const char *, unsigned int, strCompare> enumMap;
    
  public:
    EnumAttr(PiPo *pipo, const char *name, const char *descr, const std::type_info *type, bool changesStream) :
    Attr(pipo, name, descr, type, changesStream)
    {
    };
    
    void addEnumItem(const char *item)
    {
      unsigned int idx = this->enumList.size();
      
      this->enumList.push_back(item);
      this->enumMap[item] = idx;
    };
    
    std::vector<const char *> *getEnumList(void) 
    { 
      return &this->enumList; 
    };
    
    int getEnumIndex(const char *tag)
    { 
      if(tag != NULL && this->enumMap.find(tag) != this->enumMap.end())
        return this->enumMap[tag];
      
      return 0;
    };    

    const char *getEnumTag(unsigned int idx)
    { 
      if(idx < this->numEnumItems)
        return this->enumList[idx];
      
      return NULL;
    };  
    
  protected:
    int clipEnumIndex(int index)
    {
      if(index < 0)
        index = 0;
      else if(index >= (int)this->enumList.size())
        index = this->enumList.size() - 1;
      
      return index;
    };
  };
  
  /**
   * @brief Gets number of attributes
   *
   * @return number of attributes
   */
  void addAttr(PiPo *pipo, const char *name, const char *descr, Attr *attr, bool clear = false) 
  { 
    if(clear)
      this->attrs.clear();
    
    /* overwrite index, name, and description */
    attr->setIndex(pipo->attrs.size());
    attr->setName(name);
    attr->setDescr(descr);
    
    /* add to attr list */
    this->attrs.push_back(attr);
  };
  
  /**
   * @brief Gets PiPo attribute by index
   *
   * @param index attribute index
   * @return reference to PiPo attribute (NULL for invalid attribute index)
   */
  Attr *getAttr(unsigned int index)
  {
    if(index < this->attrs.size())
      return this->attrs[index];
    
    return NULL;
  };
  
  /**
   * @brief Gets PiPo attribute by name
   *
   * @param name attribute name
   * @return reference to PiPo attribute (NULL for invalid attribute name)
   */
  Attr *getAttr(const char *name)
  {
    for(unsigned int i = 0; i < this->attrs.size(); i++)
    {
      if(strcasecmp(this->attrs[i]->getName(), name) == 0)
        return this->attrs[i];
    }
    
    return NULL;
  };
  
  bool setAttr(unsigned int index, int value)
  {
    Attr *attr = getAttr(index);
    
    if(attr != NULL)
    {
      attr->set(0, value);
      
      if(attr->doesChangeStream())
        this->streamAttributesChanged();
      
      return true;
    }
    
    return false;
  }
  
  bool setAttr(unsigned int index, int *values, unsigned int numValues)
  {
    Attr *attr = getAttr(index);
    
    if(attr != NULL)
    {
      unsigned int size = attr->getSize();
      
      if(numValues > size)
        numValues = size;
      
      for(unsigned int i = 0; i < numValues; i++)
        attr->set(i, values[i]);
      
      if(attr->doesChangeStream())
        this->streamAttributesChanged();
      
      return true;
    }
    
    return false;
  }
  
  bool setAttr(unsigned int index, double val)
  {
    Attr *attr = getAttr(index);
    
    if(attr != NULL)
    {
      attr->set(0, val);
      
      if(attr->doesChangeStream())
        this->streamAttributesChanged();
      
      return true;
    }
    
    return false;
  }
  
  bool setAttr(unsigned int index, double *values, unsigned int numValues)
  {
    Attr *attr = getAttr(index);
    
    if(attr != NULL)
    {
      unsigned int size = attr->getSize();
      
      if(numValues > size)
        numValues = size;
      
      for(unsigned int i = 0; i < numValues; i++)
        attr->set(i, values[i]);
      
      if(attr->doesChangeStream())
        this->streamAttributesChanged();
      
      return true;
    }
    
    return false;
  }
  
  /**
   * @brief Gets number of attributes
   *
   * @return number of attributes
   */
  unsigned int getNumAttrs(void)
  {
    return this->attrs.size();
  };
  
  /**
   * @brief Copies current value(s) of all attributes
   *
   * @param other PiPo to clone
   */
  void cloneAttrs(PiPo *other)
  {
    for(unsigned int i = 0; i < other->attrs.size(); i++)
      this->attrs[i]->clone(other->attrs[i]);
  };
  
  /**
   * @brief Copies current value(s) of given attribute
   *
   * @param other PiPo to clone
   */
  void cloneAttr(PiPo::Attr *attr)
  {
    unsigned int index = attr->getIndex();
    
    this->attrs[index]->clone(attr);
  };  
};

/***********************************************
 *
 *  Scalar Attribute
 *
 */
template <typename TYPE>
class PiPoScalarAttr : public PiPo::Attr
{
private:
  TYPE value;
  
public:
  PiPoScalarAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream,
                      TYPE initVal = (TYPE)0) : 
  Attr(pipo, name, descr, &typeid(TYPE), changesStream)
  {
    this->value = initVal;
  }
  
  void set(TYPE value) { this->value = value; };
  TYPE get(void) { return this->value; };
  
  void clone(Attr *other) { *this = *(static_cast<PiPoScalarAttr<TYPE> *>(other)); };
  
  unsigned int setSize(unsigned int size) { return this->getSize(); };
  unsigned int getSize(void) { return 1; };
  
  void set(unsigned int i, int val) { this->value = (TYPE)val; };
  void set(unsigned int i, double val) { this->value = (TYPE)val; };
  void set(unsigned int i, const char *val) { };
  
  int getInt(unsigned int i = 0) { return (int)this->value; };
  double getDbl(unsigned int i = 0) { return (double)this->value; };  
  const char *getStr(unsigned int i = 0) { return NULL; };
};

template <>
class PiPoScalarAttr<enum PiPo::Enumerate> : public PiPo::EnumAttr
{
private:
  unsigned int value;
  
public:
  PiPoScalarAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream,
                      unsigned int initVal = NULL) :
  EnumAttr(pipo, name, descr, &typeid(enum PiPo::Enumerate), changesStream)
  {    
    this->value = initVal;
  };
  
  void set(unsigned int value) { this->value = clipEnumIndex(value); };
  void set(const char *value) { this->value = this->getEnumIndex(value); };
  unsigned int get(void) { return this->value; };
  
  void clone(Attr *other) { *this = *(static_cast<PiPoScalarAttr<enum PiPo::Enumerate> *>(other)); };
  
  unsigned int setSize(unsigned int size) { return this->getSize(); };
  unsigned int getSize(void) { return 1; };  

  void set(unsigned int i, int val) { this->value = clipEnumIndex((unsigned int)val); };
  void set(unsigned int i, double val) { this->value = clipEnumIndex((unsigned int)val); };
  void set(unsigned int i, const char *val) { this->value = getEnumIndex(val); };
  
  int getInt(unsigned int i = 0) { return (int)this->value; };
  double getDbl(unsigned int i = 0) { return (double)this->value; };
  const char *getStr(unsigned int i = 0) { return this->getEnumTag(this->value); };
};

/***********************************************
 *
 *  Array Attribute
 *
 */
/* waiting for C++11 */
template< class TYPE, std::size_t SIZE>
class array
{
  TYPE values[SIZE];
  static const int size = SIZE;
  
public:
  TYPE const& operator [] (unsigned int index) const { return this->values[index]; };
  TYPE& operator [] (unsigned int index) { return &this->values[index]; };
};

template <typename TYPE, unsigned int SIZE>
class PiPoArrayAttr : public PiPo::Attr, public array<TYPE, SIZE>
{
public:
  PiPoArrayAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream,
                     TYPE initVal = (TYPE)0) : 
  Attr(pipo, name, descr, &typeid(TYPE), changesStream), 
  array<TYPE, SIZE>()
  {
    for(unsigned int i = 0; i < SIZE; i++)
      (*this)[i] = initVal;
  }
  
  void clone(Attr *other) { *this = *(static_cast<PiPoArrayAttr<TYPE, SIZE> *>(other)); };
  
  unsigned int setSize(unsigned int size) { return this->getSize(); };
  unsigned int getSize(void) { return SIZE; }

  void set(unsigned int i, int val)
  { 
    if(i < SIZE)
      (*this)[i] = (TYPE)val;
  };
  
  void set(unsigned int i, double val) 
  { 
    if(i < SIZE)
      (*this)[i] = (TYPE)val;
  };
  
  void set(unsigned int i, const char *val) { };
  
  int getInt(unsigned int i) 
  { 
    if(i >= SIZE)
      i = SIZE - 1;
    
    return (*this)[i]; 
  };
  
  double getDbl(unsigned int i) 
  { 
    if(i >= SIZE)
      i = SIZE - 1;
    
    return (double)(*this)[i]; 
  };
  
  const char *getStr(unsigned int i) 
  { 
    if (i < SIZE)
      i = SIZE - 1;
    
    return (double)(*this)[i]; 
  };
};

template <unsigned int SIZE>
class PiPoArrayAttr<enum PiPo::Enumerate, SIZE> : public PiPo::EnumAttr, public array<unsigned int, SIZE>
{
public:
  PiPoArrayAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream,
                     unsigned int initVal = NULL) : 
  EnumAttr(pipo, name, descr, &typeid(enum PiPo::Enumerate), changesStream),
  array<unsigned int, SIZE>()
  {    
    for(unsigned int i = 0; i < this->size; i++)
      this->value[i] = initVal;
  }
    
  ~PiPoArrayAttr(void) { free(this->value); }
  
  void clone(Attr *other) { *this = *(static_cast<PiPoArrayAttr<enum PiPo::Enumerate, SIZE> *>(other)); };
  
  unsigned int setSize(unsigned int size) { return this->getSize(); };
  unsigned int getSize(void) { return SIZE; }

  void set(unsigned int i, int val)
  { 
    if(i < SIZE)
      (*this)[i] = (unsigned int)val;
  };
  
  void set(unsigned int i, double val) 
  { 
    if(i < SIZE)
      (*this)[i] = (unsigned int)val;
  };
  
  void set(unsigned int i, const char *val) 
  { 
    if(i < SIZE)
      (*this)[i] = getEnumIndex(val); 
  };

  int getInt(unsigned int i) 
  { 
    if(i >= SIZE)
      i = SIZE - 1;
    
    return (int)(*this)[i]; 
  };
  
  double getDbl(unsigned int i) 
  { 
    if(i >= SIZE)
      i = SIZE - 1;
    
    return (double)(*this)[i]; 
  };
  
  const char *getStr(unsigned int i) 
  { 
    if (i < SIZE)
      return this->getEnumTag(this->value[i]); 
    
    return NULL;
  };
};

/***********************************************
 *
 *  Var Size Attribute
 *
 */
template <typename TYPE>
class PiPoVarSizeAttr : public PiPo::Attr, public std::vector<TYPE>
{
public:
  PiPoVarSizeAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, 
                       unsigned int size = 0, TYPE initVal = (TYPE)0) : 
  Attr(pipo, name, descr, &typeid(TYPE), changesStream), 
  std::vector<TYPE>(size, initVal)
  {
  }
  
  void clone(Attr *other) { *this = *(static_cast<PiPoVarSizeAttr<TYPE> *>(other)); };

  unsigned int setSize(unsigned int size) { this->resize(size, (TYPE)0); return size; };
  unsigned int getSize(void) { return this->size(); }
  
  void set(unsigned int i, int val)
  { 
    if(i < this->size())
      (*this)[i] = (TYPE)val;
  };
  
  void set(unsigned int i, double val) 
  { 
    if(i < this->size())
      (*this)[i] = (TYPE)val;
  };
  
  void set(unsigned int i, const char *val) { };

  int getInt(unsigned int i) 
  { 
    if(i >= this->size())
      i = this->size() - 1;
    
    return (int)(*this)[i]; 
  };
  
  double getDbl(unsigned int i) 
  { 
    if(i >= this->size())
      i = this->size() - 1;
    
    return (double)(*this)[i]; 
  };

  const char *getStr(unsigned int i) { return NULL; };
};

template <>
class PiPoVarSizeAttr<enum PiPo::Enumerate> : public PiPo::EnumAttr, public std::vector<unsigned int>
{
public:
  PiPoVarSizeAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, 
                       unsigned int size = 0, unsigned int initVal = NULL) : 
  EnumAttr(pipo, name, descr, &typeid(enum PiPo::Enumerate), changesStream), 
  std::vector<unsigned int>(size, 0)
  {
    for(unsigned int i = 0; i < this->size(); i++)
      (*this)[i] = initVal;
  };
  
  void clone(Attr *other) { *this = *(static_cast<PiPoVarSizeAttr<enum PiPo::Enumerate> *>(other)); };

  unsigned int setSize(unsigned int size) { this->resize(size, 0); return size; };
  unsigned int getSize(void) { return this->size(); }

  void set(unsigned int i, int val)
  { 
    if(i < this->size())
      (*this)[i] = (unsigned int)val;
  };
  
  void set(unsigned int i, double val) 
  { 
    if(i < this->size())
      (*this)[i] = (unsigned int)val;
  };
  
  void set(unsigned int i, const char *val) 
  { 
    if(i < this->size())
      (*this)[i] = getEnumIndex(val); 
  };
  
  int getInt(unsigned int i) 
  { 
    if(i >= this->size())
      i = this->size() - 1;
    
    return (int)(*this)[i]; 
  };
  
  double getDbl(unsigned int i) 
  { 
    if(i >= this->size())
      i = this->size() - 1;
    
    return (double)(*this)[i]; 
  };

  const char *getStr(unsigned int i) 
  { 
    if (i < this->size())
      return this->getEnumTag((*this)[i]); 
    
    return NULL;
  };
};

#endif
