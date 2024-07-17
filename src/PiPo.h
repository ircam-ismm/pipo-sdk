/**
 * @file PiPo.h
 * @author Norbert.Schnell@ircam.fr
 *
 * @brief Plugin Interface for Processing Objects
 *
 * @copyright
 * Copyright (c) 2012–2016 by IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 *
 * License (BSD 3-clause)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PIPO_
#define _PIPO_

#include <string>
#include <vector>
#include <functional>
#include <cstring>
#include <cstdio>

#include <typeinfo>
#include <map>

#ifdef WIN32
#define strcasecmp _stricmp
#define M_PI 3.14159265358979323846264338328 /**< pi */
#endif

//TODO: unify with maxpipohost.h
#define PIPO_MAX_LABELS 1024

#ifndef PIPO_SDK_VERSION
#define PIPO_SDK_VERSION 0.4

#endif

typedef float PiPoValue;


struct PiPoStreamAttributes
{
  int hasTimeTags;
  double rate;
  double offset;
  unsigned int dims[2]; // width, height (by pipo convention)
  const char **labels;
  unsigned int numLabels;
  bool hasVarSize;
  double domain;
  unsigned int maxFrames;
  int labels_alloc; ///< allocated size of labels, -1 for no (outside) allocation
  int ringTail;

  PiPoStreamAttributes (int numlabels = -1)
  {
    init(numlabels);
  }

  PiPoStreamAttributes (bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames, int ringTail = 0)
  {
    this->hasTimeTags = hasTimeTags;
    this->rate    = rate;
    this->offset  = offset;
    this->dims[0] = width;
    this->dims[1] = height;
    this->numLabels = width;
    this->hasVarSize  = hasVarSize;
    this->domain  = domain;
    this->maxFrames = maxFrames;
    this->ringTail      = ringTail;

    if (labels)
    { // copy label pointers array (but not strings, they're interned symbols!)
      this->labels = new const char *[width];
      this->labels_alloc = width;

      for (unsigned int i = 0; i < width; i++)
        this->labels[i] = labels[i];
    }
    else
    {
      this->labels = NULL;
      this->labels_alloc = -1; // signals external memory
    }
  }

  // copy ctor
  PiPoStreamAttributes (const PiPoStreamAttributes &other)
  {
    // printf("PiPoStreamAttributes copy ctor\n");
    *this = other;

    if (other.labels  &&  other.labels_alloc >= 0)
    { // copy label pointers array (but not strings, they're interned symbols!)
      this->labels = new const char *[other.labels_alloc];

      for (int i = 0; i < other.labels_alloc; i++)
        this->labels[i] = other.labels[i];

      //printf("  dup %d/%d labels %p -> %p\n", numLabels, labels_alloc, other.labels, labels);
    }
  }

  // copy assignment
  PiPoStreamAttributes &operator= (const PiPoStreamAttributes &other)
  {
    // printf("PiPoStreamAttributes copy assignment\n");

    if (this != &other) // self-assignment check expected
    {
      memcpy(this, &other, sizeof(other)); // shallow copy

      if (other.labels  &&  other.labels_alloc >= 0)
      { // copy label pointers array (but not strings, they're interned symbols!)
        this->labels = new const char *[other.labels_alloc];

        for (int i = 0; i < other.labels_alloc; i++)
          this->labels[i] = other.labels[i];

        //printf("  dup %d/%d labels %p -> %p\n", numLabels, labels_alloc, other.labels, labels);
      }
    }
    return *this;
  }

  void init (int _numlab = -1)
  {
    this->hasTimeTags = false;
    this->rate    = 1000.0;
    this->offset  = 0.0;
    this->dims[0] = 1;  // width
    this->dims[1] = 1;  // height
    this->labels  = NULL;
    this->numLabels = 0;
    this->labels_alloc  = _numlab;
    this->hasVarSize  = false;
    this->domain  = 0.0;
    this->maxFrames = 1;
    this->ringTail      = 0;

    if (_numlab >= 0)
    {
      this->labels = new const char *[_numlab];

      for (int i = 0; i < _numlab; i++)
        this->labels[i] = NULL;
    }
  };

  ~PiPoStreamAttributes()
  {
    if (labels  &&  labels_alloc >= 0)
      delete [] labels;
  };

  /**
   * append string pointer array at end of labels array
   * labels array must have allocated space for _width more elements (i.e. at least numLabels + _width)
   */
  void concat_labels (const char **_labels, unsigned int _width)
  {
    if (this->labels_alloc < 0)
    {
      printf("Warning: PiPoStreamAttributes::concat_labels: can't concat %d labels to char ** with %d labels allocated from the outside\n", _width, this->numLabels);
      _width = 0;
    }

    if ((int) (this->numLabels + _width) > this->labels_alloc)
    {
      printf("Warning: PiPoStreamAttributes::concat_labels: label overflow prevented (trying to concat %d to %d used of %d)\n", _width, this->numLabels, this->labels_alloc);
      _width = this->labels_alloc - this->numLabels;
    }

    if (_labels != NULL)
      memcpy(this->labels + this->numLabels, _labels, _width * sizeof(const char *));
    else
      for (unsigned int i = 0; i < _width; i++)
        //TODO: invent numbered column, beware of memory!
        this->labels[i + this->numLabels] = "unnamed";

    this->numLabels += _width;
  }

  // @return pointer to string, write number of characters written (excluding terminating \0) into len
  char *to_string (char *str, int *len) const
  {
    *len = snprintf(str, *len,
            "hasTimeTags\t= %d\n"
            "rate\t\t= %f\n"
            "offset\t\t= %f\n"
            "width\t\t= %d\n"
            "height\t\t= %d\n"
            "labels\t\t= %s%s%s (num %d)\n"
            "labels_alloc\t= %d\n"
            "hasVarSize\t= %d\n"
            "domain\t\t= %f\n"
            "maxFrames\t= %d\n"
            "ringTail\t= %d\n",
            (int) hasTimeTags, rate, offset, dims[0], dims[1],
            labels && numLabels > 0 && labels[0] != NULL  ?  labels[0]  :  "n/a",
	    numLabels > 1  ?  "..."  :  "",
	    numLabels > 1  ?  (labels && labels[numLabels - 1] != NULL  ?  labels[numLabels - 1]  :  "n/a")  :  "",
            numLabels, labels_alloc,
            (int) hasVarSize, domain, maxFrames, ringTail);
    
    return str;
  }

  char *to_string (char *str, int len) const
  {
    return to_string(str, &len);
  }

  const std::string to_string () const
  {
    int buflen = 12 * 32 - 1;  // estimated max
    std::string str;
    str.resize(buflen + 1);
    to_string(&str[0], &buflen);
    str.resize(buflen);
    return str;
  }
}; // end class PiPoStreamAttributes



class PiPo
{

/** @mainpage

PiPo is a simple plugin API for modules processing streams of multi-dimensional data such as audio, audio descriptors, or gesture and motion data. The current version of the interface is limited to unary operations. Each PiPo module receives and produces a single stream. The elements of a stream are time-tagged or regularly sampled scalars, vectors, or two-dimensional matrices.

More information http://ismm.ircam.fr/PiPo

\section sec_api  PiPo API Overview

The PiPo API consists of an abstract class of a few virtual methods for propagating stream attributes (see below), frames, and additional processing control through a series of modules:

- Propagating stream attributes
- Propagating frames
- Reset stream processing
- Finalize stream processing
- Propagate the change of a parameter requiring redefining the output stream attributes


\subsection sec_impl Implementation of a New PiPo Module

The minimal module must derive from the class PiPo and implement at least the \ref streamAttributes and \ref frames methods:

- In \ref streamAttributes, all initialisation can be done, as all input stream parameters (attributes) are known. The output stream parameters are passed on to the receiving module via \ref propagateStreamAttributes.
- In \ref frames, only data processing and, when needed, buffering should be done.  Output frames are passed on with \ref propagateFrames.

If the module can produce additional output data after the end of the input data, it must implement \ref finalize, from within which more calls to \ref propagateFrames can be made, followed by a mandatory call to \ref propagateFinalize.

If the module keeps internal state or buffering, it should implement the \ref reset method to put itself into a clean state.

The utility function \ref signalError can be used to pass an error message to the host.

The utility function \ref signalWarning can be used to pass a warning message to the host.


\subsection sec_attr Module Attributes or Parameters

The template class PiPo::Attr permits to define scalar, enum, or variable or fixed size vector attributes of a pipo module that are exposed to the host environment.

The are initialised in the module constructor with a short name, a description, a flag if a change of value means the fundamental stream parameters must be reset (if true, \ref streamAttributes will be called again for the whole chain), and a default value.

Their value can be queried in \ref streamAttributes or \ref frames (in real-time hosts, an attributes value can change over time) with PiPo::Attr::get().

\subsection sec_example Example of a Minimal PiPo Module


\code

class PiPoGain : public PiPo
{
private:
  std::vector<PiPoValue> buffer_;
  unsigned int           framesize_;    // cache max frame size

public:
  PiPoScalarAttr<double> factor_attr_;

  PiPoGain (Parent *parent, PiPo *receiver = NULL)
  : PiPo(parent, receiver),
    factor_attr_(this, "factor", "Gain Factor", false, 1.0)
  { }

  ~PiPoGain (void)
  { }

  // Configure PiPo module according to the input stream attributes and propagate output stream attributes.
  // Note: For audio input, one PiPo frame corresponds to one sample frame, i.e. width is the number of channels, height is 1, maxFrames is the maximum number of (sample) frames passed to the module, rate is the sample rate, and domain is 1 / sample rate.
  //
  int streamAttributes (bool hasTimeTags, double rate, double offset,
                        unsigned int width, unsigned int height,
                        const char **labels, bool hasVarSize,
                        double domain, unsigned int maxFrames)
  {
    // we need to store the max frame size in case hasVarSize is true
    framesize_ = width * height; 

    // A general pipo can not work in place, we need to create an output buffer
    buffer_.resize(framesize_ * maxFrames);

    // we will produce the same stream layout as the input
    return propagateStreamAttributes(hasTimeTags, rate, offset, width, height,
                                     labels, hasVarSize, domain, maxFrames);
  }

  int frames (double time, double weight, PiPoValue *values,
              unsigned int size, unsigned int num)
  {
    double     f      = factor_attr_.get(); // get gain factor here, as it could change while running
    PiPoValue *outptr = &buffer_[0];

    for (unsigned int i = 0; i < num; i++)
    {
      for (unsigned int j = 0; j < size; j++)
        outptr[j] = values[j] * f;

      outptr += framesize_;
      values += framesize_;
    }

    return propagateFrames(time, weight, &buffer_[0], size, num);
  }
};
\endcode


\section sec_api_details PiPo API Details

\subsection sec_stream_attr PiPo Stream Attributes

PiPo streams are a sequences of frames characterized by a set of attributes. A PiPo module defines the attributes of its output stream when receiving the attributes of the input stream.

Each module can configure its internal state depending on the attributes of the input stream (e.g. memory allocation and pre-calculated state variables) before propagating its output stream attributes to the next module.

This way, the attributes of the input stream are propagated through a series of PiPo modules before starting the actual stream processing.

In summary, a PiPo stream is described by the following attributes:

- a boolean representing whether the elements of the stream are time-tagged
- frame rate (highest average rate for time-tagged streams)
- lag of the output stream relative to the input
- frame width (also number of channels or data matrix columns)
- frame height (or number of matrix rows)
- labels (for the frame channels or columns)
- a boolean representing whether the frames have a variable height (respecting the given frame height as maximum)
- extent of a frame in the given domain (e.g. duration or frequency range)
- maximum number of frames in a block exchanged between two modules

*/


public:
  class Attr;
  template<class, std::size_t> class AttrArray; // declare this helper template as PiPo::AttrArray

  /***********************************************
   *
   *  PiPo Parent
   * TODO: rename PiPoHost ?
   */

  class Parent
  {
  public:
    /** called by pipo when an attribute with "changesstream" is set */
    virtual void streamAttributesChanged(PiPo *pipo, PiPo::Attr *attr) { };

    /** called by pipo to signal error in parameters */
    virtual void signalError(PiPo *pipo, std::string errorMsg) { };

    /** called by pipo to signal warning in parameters */
    virtual void signalWarning(PiPo *pipo, std::string errorMsg) { };
  };

protected:
  Parent *parent;
  std::vector<PiPo *> receivers; /**< list of receivers */

private:
  std::vector<Attr *> attrs; /**< list of attributes */
#if __cplusplus >= 201103L  &&  !defined(WIN32)
  constexpr static const float sdk_version = PIPO_SDK_VERSION; /**< pipo SDK version (for inspection) */
#endif

public:
  PiPo(Parent *parent, PiPo *receiver = NULL)
  : receivers(), attrs()
  {
    this->parent = parent;

    if(receiver != NULL)
      this->receivers.push_back(receiver);
  }

  PiPo(const PiPo &other)
  {
    this->parent = other.parent;
  }

  virtual ~PiPo(void)
  {
    
  }

  /**
   * Get version of SDK as a major.minor float (so that host can
   * check if a pipo was compiled with correct version of PiPo.h)
   */
#ifdef PIPO_TESTING
  virtual float getVersion() // only for unit tests: allow to override version to simulate an out-of-date pipo module
#else
  static  float getVersion()
#endif
  {
#if __cplusplus >= 201103L  &&  !defined(WIN32)
    printf("pipo::getVersion -> %f\n", PiPo::sdk_version);
    return PiPo::sdk_version;
#else
    printf("pipo::getVersion -> %f\n", PIPO_SDK_VERSION);
    return PIPO_SDK_VERSION;
#endif
  }

  /**
   * @brief Sets PiPo parent.
   *
   * @param parent PiPo parent
   */
  virtual void setParent(Parent *parent) { this->parent = parent; }

    /**
   * @brief Configures a PiPo module according to the input stream attributes and propagate output stream attributes
   *
   * Note: For audio input, one PiPo frame corresponds to one sample frame, i.e. width is the number of channels, height is 1, and maxFrames is the maximum number of (sample) frames passed to the module. Also, rate is the sample rate and domain is 1 / sample rate.
   *
   * PiPo module:
   * Any implementation of this method requires a propagateStreamAttributes() method call and returns its return value, typically like this:
   *
   * \code
   *  return this->propagateStreamAttributes(hasTimeTags, rate, offset, width, height, labels, hasVarSize, domain, maxFrames);
   * \endcode
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the final output stream attributes and usally returns 0.
   *
   * @param hasTimeTags a boolean representing whether the elements of the stream are time-tagged
   * @param rate        the frame rate (highest average rate for time-tagged streams, sample rate for audio input)
   * @param offset      the lag of the output stream relative to the input
   * @param width       the frame width (number of channels for audio or data matrix columns)
   * @param height      the frame height (or number of matrix rows, always 1 for audio)
   * @param labels      optional labels for the frames' channels or columns (can be NULL)
   * @param hasVarSize  a boolean representing whether the frames have a variable height (respecting the given frame height as maximum)
   * @param domain      extent of a frame in the given domain (e.g. duration or frequency range)
   * @param maxFrames   maximum number of frames in a block exchanged between two modules (window size for audio)
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int streamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames) = 0;

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
  }

  /**
   * @brief Processes a single frame or a block of frames
   *
   * PiPo module:
   * An implementation of this method may call propagateFrames(), typically like this:
   *
   * \code
   *  return this->propagateFrames(time, weight, values, size, num);
   * \endcode
   *
   * PiPo host:
   * A terminating receiver module provided by a PiPo host handles the received frames and usally returns 0.
   *
   * @param time        time-tag for a single frame or a block of frames
   * @param weight      weight associated to frame or block
   * @param values      interleaved frames values, row by row (interleaving channels or columns), frame by frame
   TODO: should be const!!!
   * @param size        actual number of elements in each frame (number of channels for audio, can differ from width * height for varsize frames!)
   * @param num         number of frames (number of sample framess for audio input)
   * @return            0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int frames (double time, double weight, PiPoValue *values, unsigned int size, unsigned int num) = 0;

  /**
   * @brief Signals segment start or end
   *
   * A PiPo module implementing this method calls propagateFrames()
   * with the segment description at the end of the segment, and does
   * NOT pass it on.
   *
   * In the case of two sucessive calls to segment with start = true, the second call implicitly ends the last segment.
   *
   * \code

   if (this->started)
   {
     // do what is to be done to finalize the segment description
     this->propagateFrames(time, weight, segment_values, size, 1);
     this->started = false;
   }

   if (start)
   {
     // do what is to be done to initialize the segment description
     this->started = true;
   }

   return 0;  // we don't pass on the segment() call, since here we did handle it

   * \endcode
   *
   * @param time time of segment start or end
   * @param start flag, true for segment start, false for segment end
   * @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
   */
  virtual int segment(double time, bool start)
  {
    // the default implementation passes the segmentation call on to any module that knows what to do with it, notably a temporal modeling module.
    return propagateSegment(time, start);
  }

  /**
   * @brief Finalizes processing (optional)
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
    return propagateFinalize(inputEnd);
  }

  
  /**
   * @brief Propagates a module's output stream attributes to its receiver.
   *
   * This method is called in the streamAttributes() method of a PiPo module.
   *
   * @param hasTimeTags a boolean representing whether the elements of the stream are time-tagged
   * @param rate the frame rate (highest average rate for time-tagged streams)
   * @param offset the lag of the output stream relative to the input
   * @param width the frame width (also number of channels or data matrix columns)
   * @param height the frame height (or number of matrix rows)
   * @param labels optional labels for the frames' channels or columns
   * @param hasVarSize a boolean representing whether the frames have a variable height (respecting the given frame height as maximum)
   * @param domain extent of a frame in the given domain (e.g. duration or frequency range)
   * @param maxFrames maximum number of frames in a block exchanged between two modules
   * @return used as return value of the calling method
   */
  int propagateStreamAttributes(bool hasTimeTags, double rate, double offset, unsigned int width, unsigned int height, const char **labels, bool hasVarSize, double domain, unsigned int maxFrames)
  {
    int ret = 0;

    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->streamAttributes(hasTimeTags, rate, offset, width, height, labels, hasVarSize, domain, maxFrames);

      if(ret < 0)
        break;
    }

    return ret;
  }

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
   * @brief Propagates a module's output frames to its receiver.
   *
   * This method is called in the frames() method of a PiPo module.
   *
   * @param time  time-tag for a single frame or a block of frames
   * @param weight  weight for this frame (currently unused)
   * @param values  interleaved frames values, row by row (interleaving channels or columns), frame by frame
   * @param size  total size of each frame (number of values = width * height)
   * @param num   number of frames
   * @return    used as return value of the calling method
   */
  int propagateFrames(double time, double weight, PiPoValue *values, unsigned int size, unsigned int num)
  {
    int ret = -1;

    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = this->receivers[i]->frames(time, weight, values, size, num);

      if(ret < 0)
        break;
    }

    return ret;
  }

  /**
   * @brief Propagates the segment control event.
   *
   * This method is called in the segment() method of a PiPo module.
   *
   * @return used as return value of the calling method
   */
  int propagateSegment (double time, bool start)
  {
    int ret = -1;

    for (unsigned int i = 0; i < receivers.size(); i++)
    {
      ret = receivers[i]->segment(time, start);

      if (ret < 0)
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
  virtual PiPo *getReceiver(unsigned int index = 0)
  {
    if(index < this->receivers.size())
      return this->receivers[index];

    return NULL;
  }

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
    }
  }
  
  /** section: internal methods
   */

  void streamAttributesChanged(Attr *attr)
  {
    if(this->parent != NULL)
      this->parent->streamAttributesChanged(this, attr);
  }

  /**
   * Signal error message to be output by the host.
   */
  void signalError(std::string errorMsg)
  {
    if(this->parent != NULL)
      this->parent->signalError(this, errorMsg);
    else
      printf("PiPo::signalError (not parent): %s\n", errorMsg.c_str());
  }

  /**
   * Signal warning message to be output by the host.
   */
  void signalWarning(std::string errorMsg)
  {
    if(this->parent != NULL)
      this->parent->signalWarning(this, errorMsg);
    else
      printf("PiPo::signalWarning (not parent): %s\n", errorMsg.c_str());
  }



  /***********************************************
   *
   *  PiPo Attributes
   *
   */
public:
  enum Type { Undefined, Bool, Enum, Int, Float, Double, String, Function, Dictionary };

  // dummy enum used for specialization of templates
  enum Enumerate { };

  // meta-type a la Max :
  class Atom
  {
  private:
    PiPo::Type type;
    union _data {
        const char *str;
        double dbl;
        int itg;
    } data;
  public:
    Atom(const char *s)   { this->type = String; this->data.str = s; }
    Atom(double d)        { this->type = Double; this->data.dbl = d; }
    Atom(int i)           { this->type = Int; this->data.itg = i; }
    friend bool operator==(Atom &at1, Atom &at2)
    {
        return ((at1.isNumber() && at2.isNumber() && at1.getDouble() == at2.getDouble())
                || (at1.type == String && at1.type == at2.type && strcmp(at1.getString(), at2.getString()) == 0));
    }
    friend bool operator!=(Atom &at1, Atom &at2)
    {
        return !(at1 == at2);
    }
    bool          isNumber()  { return type == Int || type == Double; }
    bool          isString()  { return type == String || type == Dictionary; }
    PiPo::Type    getType()   { return type; }
    int           getInt()    { return ((type == Int) ? this->data.itg : ((type == Double) ? (int)(this->data.dbl) : 0)); }
    double        getDouble() { return ((type == Double) ? this->data.dbl : ((type == Int) ? (double)(this->data.itg) : 0.)); }
    const char *  getString() { return (isString() ? this->data.str : ""); }
  }; // class PiPo::Atom

  class Attr
  {
  private:
    PiPo *pipo; /**< owner PiPo */
    unsigned int index;
    const char *name; /**< attribute name */
    const char *descr; /**< short description */
    bool changesStream;
    bool isArray;
    bool isVarSize;
    bool has_changed; /// changed from outside

  protected:
    enum Type type;

  public:
    /**
     * PiPo attribute base class
     */
    Attr(PiPo *pipo, const char *name, const char *descr, const std::type_info *type, bool changesStream, bool isArray = false, bool isVarSize = false)
    {
      this->pipo = pipo;
      this->index = (unsigned int) pipo->attrs.size();
      this->name = name;
      this->descr = descr;
      this->isArray = isArray;
      this->isVarSize = isVarSize;

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
    }

    ~Attr(void) { }

    void setIndex(unsigned int index) { this->index = index; }
    void setName(const char *name) { this->name = name; }
    void setDescr(const char *descr) { this->descr = descr; }

    unsigned int getIndex(void) { return this->index; }
    const char *getName(void) { return this->name; }
    const char *getDescr(void) { return this->descr; }
    enum Type getType(void) { return this->type; }
    bool doesChangeStream(void) { return this->changesStream; }
    bool getIsArray(void) {return this->isArray;}
    bool getIsVarSize(void) {return this->isVarSize;}
    
    virtual void clone(Attr *other) = 0;

    virtual unsigned int setSize(unsigned int size) = 0;
    virtual unsigned int getSize(void) = 0;

    virtual void set(unsigned int i, int val, bool silently = false) = 0;
    virtual void set(unsigned int i, double val, bool silently = false) = 0;
    virtual void set(unsigned int i, const char *val, bool silently = false) = 0;
    virtual int getInt(unsigned int i) = 0;
    virtual double getDbl(unsigned int i) = 0;
    virtual const char *getStr(unsigned int i) = 0;
   
    virtual std::vector<const char *> *getEnumList(void) { return NULL; }

    void changed(bool silently = false) { this->has_changed = true; if (!silently && this->changesStream) this->pipo->streamAttributesChanged(this); }
    bool hasChanged() { return this->has_changed; }
    void resetChanged() { this->has_changed = false; }
    void rename(const char *name) { this->name = name; }
  }; // end class PiPo::Attr

  
  /**
   * PiPo enumerator attribute base class
   */
  class EnumAttr : public Attr
  {
#if __cplusplus < 201703L // c++11 or c++14
    struct strCompare : public std::binary_function<const char *, const char *, bool>
#else // from c++17 on, std::binary_function is no longer necessary and has been removed
    struct strCompare
#endif
    {
      bool operator() (const char *str1, const char *str2) const { return std::strcmp(str1, str2) < 0; }
    };

    std::vector<const char *>enumList;
    std::vector<const char *>enumListDoc;
    std::map<const char *, unsigned int, strCompare> enumMap;

  public:
    EnumAttr(PiPo *pipo, const char *name, const char *descr, const std::type_info *type, bool changesStream, bool isArray = false, bool isVarSize = false) :
    Attr(pipo, name, descr, type, changesStream, isArray, isVarSize),
    enumList(), enumListDoc(), enumMap()
    {
    }

    void addEnumItem(const char *item, const char *doc = "undocumented")
    {
      unsigned int idx = (unsigned int) this->enumList.size();

      this->enumList.push_back(item);
      this->enumListDoc.push_back(doc);
      this->enumMap[item] = idx;
    }

    std::vector<const char *> *getEnumList(void)
    {
      return &this->enumList;
    }

    int getEnumIndex(const char *tag)
    {
      if(tag != NULL && this->enumMap.find(tag) != this->enumMap.end())
        return this->enumMap[tag];

      return -1;
    }

    const char *getEnumTag(unsigned int idx)
    {
      if (idx < this->enumList.size())
        return this->enumList[idx];

      return NULL;
    }

  protected:
    int clipEnumIndex(int index)
    {
      if(index < 0)
        index = 0;
      else if(index >= (int)this->enumList.size())
        index = (unsigned int) this->enumList.size() - 1;

      return index;
    }
  }; // end class PiPo::EnumAttr

  /**
   * @brief Add attribute.  Input attr's index, name, descr fields will be overwritten.
   */
  void addAttr(PiPo *pipo, const char *name, const char *descr, Attr *attr, bool clear = false)
  {
    if(clear)
      this->attrs.clear();

    /* overwrite index, name, and description */
    //NB: this is a pretty serious side effect, that the input attr is modified
    attr->setIndex((unsigned int) pipo->attrs.size());
    attr->setName(name);
    attr->setDescr(descr);

    /* add to attr list */
    this->attrs.push_back(attr);
  }

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
  }

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
  }

  /**
   * @brief Gets PiPo attribute by qualified name
   *
   * @param piponame pipo module name in pipo chain
   * @param name attribute name
   * @return reference to PiPo attribute (NULL for invalid attribute name)
   */
  Attr *getAttr(const char *piponame, const char *name)
  {
    std::string qname(piponame);

    qname += ".";
    qname += name;

    return getAttr(qname.c_str());
  }

#if 1
  template<typename TYPE>
  bool setAttr (unsigned int index, TYPE value, bool silently = false)
  {
    Attr *attr = getAttr(index);

    if (attr != NULL)
    {
      attr->set(0, value, silently);

      return true;
    }

    return false;
  }

  template<typename TYPE>
  bool setAttr (unsigned int index, TYPE *values, unsigned int numValues, bool silently = false)
  {
    Attr *attr = getAttr(index);

    if (attr != NULL)
    {
      // unused: unsigned int size = (unsigned int) attr->getSize();

      for (unsigned int i = 0; i < numValues; i++)
        attr->set(i, values[i], silently);

      return true;
    }

    return false;
  }

#else

  bool setAttr(unsigned int index, int value, bool silently = false)
  {
    Attr *attr = getAttr(index);

    if(attr != NULL)
    {
      attr->set(0, value, silently);

      return true;
    }

    return false;
  }

  bool setAttr(unsigned int index, int *values, unsigned int numValues, bool silently = false)
  {
    Attr *attr = getAttr(index);

    if(attr != NULL)
    {
      // unused: unsigned int size = (unsigned int) attr->getSize();

      for(unsigned int i = 0; i < numValues; i++)
        attr->set(i, values[i], silently);

      return true;
    }

    return false;
  }

  bool setAttr(unsigned int index, double val, bool silently = false)
  {
    Attr *attr = getAttr(index);

    if(attr != NULL)
    {
      attr->set(0, val, silently);

      return true;
    }

    return false;
  }

  bool setAttr(unsigned int index, double *values, unsigned int numValues, bool silently = false)
  {
    Attr *attr = getAttr(index);

    if(attr != NULL)
    {
      // unsigned int size = attr->getSize();

      for(unsigned int i = 0; i < numValues; i++)
        attr->set(i, values[i], true);

      attr->changed(silently);

      return true;
    }

    return false;
  }
#endif
  
  /**
   * @brief Gets number of attributes
   *
   * @return number of attributes
   */
  unsigned int getNumAttrs(void) const
  {
    return (unsigned int) this->attrs.size();
  }

  /**
   * @brief Copies current parent and attributes values
   *
   * @param other PiPo to clone
   */
  void cloneAttrs(PiPo *other)
  {
    for(unsigned int i = 0; i < other->attrs.size(); i++)
      this->attrs[i]->clone(other->attrs[i]);
  }

  /**
   * @brief Copies current value(s) of given attribute
   *
   * @param attr attribute to clone from
   */
  void cloneAttr(PiPo::Attr *attr)
  {
    unsigned int index = attr->getIndex();

    this->attrs[index]->clone(attr);
  }

  
  /// utility function to get column indices from an int or string PiPo::Attr
  //
  // checks that index is < max_num, looks up strings in given column names, returns int array of valid indices
  // N.B.: no sorting, double columns are allowed
  //
  // @param attr		pipo attribute to get column index or name from, can be any PiPo::Attr subclass that has operator[] defined (fixed or var size list, not yet scalar)
  // @param max_num	highest valid index + 1
  // @param labels	array of max_num column names for lookup from string attr
  // @return vector<int> of valid column indices ( 0 .. max_num - 1), empty attr list returns vector of all indices 0 .. max_num - 1

  template<typename ATTRTYPE>
  static std::vector<unsigned int> lookup_column_indices (ATTRTYPE &attr, int max_num, const char **labels = NULL)
  {
    int attrsize = attr.getSize();
    std::vector<unsigned int> checked;
    checked.reserve(attrsize);    // make space for maximum size
    
    for (int i = 0; i < attrsize; i++)
    {
      PiPo::Atom elem(attr[i]); // put attr element (of any type) into pipo Atom, either copying atom, or wrapping int or string
      // todo: define TYPE get(int) method for all pipo::attr derivations

      switch (elem.getType())
      {
        case PiPo::Double:
        case PiPo::Int:
	{
	  int res = elem.getInt();
	  if (res >= 0  &&  static_cast<unsigned int>(res) < max_num)
	    checked.push_back(res);
	}
	break;

        case PiPo::String:
	  if (labels != NULL) // lookup string atom by search (TBD: make dict lookup?)
	    for (unsigned int j = 0; j < max_num; j++)
	      if (labels[j] != NULL  &&  std::strcmp(elem.getString(), labels[j]) == 0)
		checked.push_back(j);
	  break;

        default: 
	  break;
      }
    }

    if (checked.size() == 0)
    {
      // fill with all indices
      checked.resize(max_num);
      for (unsigned int i = 0; i < max_num; ++i)
	checked[i] = i;
    }

    return checked;
  } // end lookup_column_indices

}; // end class PiPo



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
  PiPoScalarAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, TYPE initVal = (TYPE)0) :
  Attr(pipo, name, descr, &typeid(TYPE), changesStream)
  {
    this->value = initVal;
  }

  void set(TYPE value, bool silently = false) { this->value = value; this->changed(silently); }
  TYPE get(void) { return this->value; }

  void clone(Attr *other) { this->value = (dynamic_cast<PiPoScalarAttr<TYPE> *>(other))->value; }

  unsigned int setSize(unsigned int size) { return this->getSize(); }
  unsigned int getSize(void) { return 1; }

  void set(unsigned int i, int val, bool silently = false) { if(i == 0) this->value = (TYPE)val; this->changed(silently); }
  void set(unsigned int i, double val, bool silently = false) { if(i == 0) this->value = (TYPE)val; this->changed(silently); }
  void set(unsigned int i, const char *val, bool silently = false) { }

  int getInt(unsigned int i = 0) { return (int)this->value; }
  double getDbl(unsigned int i = 0) { return (double)this->value; }
  const char *getStr(unsigned int i = 0) { return NULL; }
};

template <>
class PiPoScalarAttr<const char *> : public PiPo::Attr
{
private:
  const char * value;

public:
  PiPoScalarAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream,
		 const char *initVal = (const char *) 0)
  : Attr(pipo, name, descr, &typeid(const char *), changesStream)
  {
    this->value = initVal;
  }

  void set(const char * value) { this->value = value; }
  const char *get(void) { return this->value; }

  void clone(Attr *other) { *this = *(static_cast<PiPoScalarAttr<const char *> *>(other)); }

  unsigned int setSize(unsigned int size) { return this->getSize(); }
  unsigned int getSize(void) { return 1; }

  void set(unsigned int i, int val, bool silently = false) { }
  void set(unsigned int i, double val, bool silently = false) { }
  void set(unsigned int i, const char *val, bool silently = false) { if(i == 0) this->value = val; this->changed(silently); }

  int getInt(unsigned int i = 0) { return 0; }
  double getDbl(unsigned int i = 0) { return 0; }
  const char *getStr(unsigned int i = 0) { return this->value; }
};

template <>
class PiPoScalarAttr<enum PiPo::Enumerate> : public PiPo::EnumAttr
{
private:
  unsigned int value;

public:
  PiPoScalarAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, unsigned int initVal = 0) :
  EnumAttr(pipo, name, descr, &typeid(enum PiPo::Enumerate), changesStream)
  {
    this->value = initVal;
  }

  void set(unsigned int value, bool silently = false) { this->value = clipEnumIndex(value); this->changed(silently); }
  void set(const char *value, bool silently = false) { this->value = this->getEnumIndex(value); this->changed(silently); }
  unsigned int get(void) { return this->value; }

  void clone(Attr *other) { this->value = (dynamic_cast<PiPoScalarAttr<enum PiPo::Enumerate> *>(other))->value; }

  unsigned int setSize(unsigned int size) { return this->getSize(); }
  unsigned int getSize(void) { return 1; }

  void set(unsigned int i, int val, bool silently = false) { if(i == 0) this->value = clipEnumIndex((unsigned int)val); this->changed(silently); }
  void set(unsigned int i, double val, bool silently = false) { if(i == 0) this->value = clipEnumIndex((unsigned int)val); this->changed(silently); }
  void set(unsigned int i, const char *val, bool silently = false) { if(i == 0) this->value = getEnumIndex(val); this->changed(silently); }

  int getInt(unsigned int i = 0) { return (int)this->value; }
  double getDbl(unsigned int i = 0) { return (double)this->value; }
  const char *getStr(unsigned int i = 0) { return this->getEnumTag(this->value); }
};


/** specialisation of string attr that can receive a dictionary structure from the host and transmits this as a json string to the pipo module.
    The string value of the attr is the external id of the dictionary and shouldn't be changed.
 */
class PiPoDictionaryAttr : public PiPoScalarAttr<const char *>
{
public:
  PiPoDictionaryAttr (PiPo *pipo, const char *name, const char *descr, bool changesStream, const char * initVal = (const char *) 0)
  : PiPoScalarAttr<const char *>(pipo, name, descr, changesStream, initVal), json_string(NULL)
  {
    this->type = PiPo::Dictionary;
  }

  ~PiPoDictionaryAttr ()
  {
    if (json_string)
      delete json_string;
  }

  const char *getJson ()
  {
    return json_string  ?  const_cast<const char *>(json_string)  :  "";
  }

  // must only be called by host
  void setJson (const char *str)
  {
    if (json_string)
      delete json_string;

    json_string = new char [strlen(str) + 1];
    strcpy(json_string, str);
  }
  
private:
  char *json_string; // std::string crashes again...
};


/***********************************************
 *
 *  Fixed Size Array Attribute
 *
 */

template< class TYPE, std::size_t SIZE>
class PiPo::AttrArray
{
  TYPE values[SIZE];
  static const int size = SIZE;

public:
  TYPE const& operator [] (unsigned int index) const { return this->values[index]; }
  TYPE& operator [] (unsigned int index) { return &this->values[index]; }
};

template <typename TYPE, unsigned int SIZE>
class PiPoArrayAttr : public PiPo::Attr, public PiPo::AttrArray<TYPE, SIZE>
{
public:
  PiPoArrayAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, TYPE initVal = (TYPE)0) :
  Attr(pipo, name, descr, &typeid(TYPE), changesStream, true, false),
  PiPo::AttrArray<TYPE, SIZE>()
  {
    for(unsigned int i = 0; i < SIZE; i++)
      (*this)[i] = initVal;
  }
  void clone(Attr *other) { *(dynamic_cast<PiPo::AttrArray<TYPE, SIZE> *>(this)) = *(dynamic_cast<PiPo::AttrArray<TYPE, SIZE> *>(other)); }

  unsigned int setSize(unsigned int size) { return this->getSize(); }
  unsigned int getSize(void) { return SIZE; }

  void set(unsigned int i, int val, bool silently = false)
  {
    if(i < SIZE)
      (*this)[i] = (TYPE)val;

    this->changed(silently);
  }

  void set(unsigned int i, double val, bool silently = false)
  {
    if(i < SIZE)
      (*this)[i] = (TYPE)val;

    this->changed(silently);
  }

  void set(unsigned int i, const char *val, bool silently = false) { }

  int getInt(unsigned int i)
  {
    if(i >= SIZE)
      i = SIZE - 1;

    return (int)(*this)[i];
  }

  double getDbl(unsigned int i)
  {
    if(i >= SIZE)
      i = SIZE - 1;

    return (double)(*this)[i];
  }

  const char *getStr(unsigned int i)
  {
    if (i < SIZE)
      i = SIZE - 1;

    return (const char *)(*this)[i];
  }
};

template <unsigned int SIZE>
class PiPoArrayAttr<enum PiPo::Enumerate, SIZE> : public PiPo::EnumAttr, public PiPo::AttrArray<unsigned int, SIZE>
{
public:
  PiPoArrayAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, unsigned int initVal = 0) :
  EnumAttr(pipo, name, descr, &typeid(enum PiPo::Enumerate), changesStream, true, false),
  PiPo::AttrArray<unsigned int, SIZE>()
  {
    for(unsigned int i = 0; i < this->size; i++)
      this->value[i] = initVal;
  }

  ~PiPoArrayAttr(void) { free(this->value); }

  void clone(Attr *other) { *(dynamic_cast<PiPo::AttrArray<unsigned int, SIZE> *>(this)) = *(dynamic_cast<PiPo::AttrArray<unsigned int, SIZE> *>(other)); }

  unsigned int setSize(unsigned int size) { return this->getSize(); }
  unsigned int getSize(void) { return SIZE; }

  void set(unsigned int i, int val, bool silently = false)
  {
    if(i < SIZE)
      (*this)[i] = (unsigned int)val;

    this->changed(silently);
  }

  void set(unsigned int i, double val, bool silently = false)
  {
    if(i < SIZE)
      (*this)[i] = (unsigned int)val;

    this->changed(silently);
  }

  void set(unsigned int i, const char *val, bool silently = false)
  {
    if(i < SIZE)
      (*this)[i] = getEnumIndex(val);

    this->changed(silently);
  }

  int getInt(unsigned int i)
  {
    if(i >= SIZE)
      i = SIZE - 1;

    return (int)(*this)[i];
  }

  double getDbl(unsigned int i)
  {
    if(i >= SIZE)
      i = SIZE - 1;

    return (double)(*this)[i];
  }

  const char *getStr(unsigned int i)
  {
    if (i < SIZE)
      return this->getEnumTag(this->value[i]);

    return NULL;
  }
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
  PiPoVarSizeAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, unsigned int size = 0, TYPE initVal = (TYPE)0) :
  Attr(pipo, name, descr, &typeid(TYPE), changesStream, false, true),
  std::vector<TYPE>(size, initVal)
  {
  }

  void clone(Attr *other) { *(dynamic_cast<std::vector<TYPE> *>(this)) = *(dynamic_cast<std::vector<TYPE> *>(other)); }

  unsigned int setSize(unsigned int size) { this->resize(size, (TYPE)0); return size; }
  unsigned int getSize(void) { return (unsigned int) this->size(); }

  void set(unsigned int i, int val, bool silently = false)
  {
    if (i >= this->size())
      setSize(i + 1);

    (*this)[i] = (TYPE)val;

    this->changed(silently);
  }

  void set(unsigned int i, double val, bool silently = false)
  {
    if (i >= this->size())
      setSize(i + 1);

    (*this)[i] = static_cast<TYPE>(val);

    this->changed(silently);
  }

  void set(unsigned int i, const char *val, bool silently = false)
  { /* conversion from string not implemented */ }

  int getInt(unsigned int i)
  {
    if(i >= this->size())
      i = (unsigned int) this->size() - 1;

    return static_cast<int>((*this)[i]);
  }

  double getDbl(unsigned int i)
  {
    if(i >= this->size())
      i = (unsigned int) this->size() - 1;

    return static_cast<double>((*this)[i]);
  }

  const char *getStr(unsigned int i) { return NULL; }

  TYPE *getPtr()  // return pointer to first data element
  {
    return &((*this)[0]);
  }

  using std::vector<TYPE>::size;
  using std::vector<TYPE>::begin;
  using std::vector<TYPE>::erase;

  void remove (int pos) // remove element at index pos
  {
    if (pos >= 0  &&  pos < size())    
      erase(begin() + pos);
  }
}; // template class PiPoVarSizeAttr<TYPE>


// specialisation of PiPoVarSizeAttr template for c-strings
template <>
class PiPoVarSizeAttr<const char *> : public PiPo::Attr, public std::vector<const char *>
{
public:
  PiPoVarSizeAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, unsigned int size = 0, const char *initVal = 0) :
  Attr(pipo, name, descr, &typeid(const char *), changesStream, false, true),
  std::vector<const char *>(size, initVal)
  {
    for(unsigned int i = 0; i < this->size(); i++)
      (*this)[i] = initVal;
  }

  void clone(Attr *other) { *(dynamic_cast<std::vector<const char *> *>(this)) = *(dynamic_cast<std::vector<const char *> *>(other)); }

  unsigned int setSize(unsigned int size) { this->resize(size, 0); return size; }
  unsigned int getSize(void) { return (unsigned int) this->size(); }

  void set(unsigned int i, int val, bool silently = false)
  {
    if (i >= this->size())
      setSize(i + 1);

    (*this)[i] = NULL; // todo: itoa

    this->changed(silently);
  }

  void set(unsigned int i, double val, bool silently = false)
  {
    if (i >= this->size())
      setSize(i + 1);

    (*this)[i] = NULL; // todo: ftoa

    this->changed(silently);
  }

  void set(unsigned int i, const char *val, bool silently = false)
  {
    if (i >= this->size())
      setSize(i + 1);

    (*this)[i] = val;

    this->changed(silently);
  }

  int getInt(unsigned int i)
  {
    if(i >= this->size())
      i = (unsigned int) this->size() - 1;

    return 0; // todo: atoi
  }

  double getDbl(unsigned int i)
  {
    if(i >= this->size())
      i = (unsigned int) this->size() - 1;

    return 0; // todo: atof
  }

  const char *getStr(unsigned int i)
  {
    if (i < this->size())
      return (*this)[i];
    
    return NULL;
  }

  void remove (int pos) // remove element at index pos
  {
    if (pos >= 0  &&  pos < (int) size())
      erase(begin() + pos);
  }
};


// specialisation of PiPoVarSizeAttr template for pipo enum type
template <>
class PiPoVarSizeAttr<enum PiPo::Enumerate> : public PiPo::EnumAttr, public std::vector<unsigned int>
{
public:
  PiPoVarSizeAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, unsigned int size = 0, unsigned int initVal = 0) :
  EnumAttr(pipo, name, descr, &typeid(enum PiPo::Enumerate), changesStream, false, true),
  std::vector<unsigned int>(size, 0)
  {
    for(unsigned int i = 0; i < this->size(); i++)
      (*this)[i] = initVal;
  }

  void clone(Attr *other) { *(dynamic_cast<std::vector<unsigned int> *>(this)) = *(dynamic_cast<std::vector<unsigned int> *>(other)); }

  unsigned int setSize(unsigned int size) { this->resize(size, 0); return size; }
  unsigned int getSize(void) { return (unsigned int) this->size(); }

  void set(unsigned int i, int val, bool silently = false)
  {
    if (i >= this->size())
      setSize(i + 1);

    (*this)[i] = (unsigned int)val;

    this->changed(silently);
  }

  void set(unsigned int i, double val, bool silently = false)
  {
    if (i >= this->size())
      setSize(i + 1);

    (*this)[i] = (unsigned int)val;

    this->changed(silently);
  }

  void set(unsigned int i, const char *val, bool silently = false)
  {
    if (i >= this->size())
      setSize(i + 1);

    (*this)[i] = getEnumIndex(val);

    this->changed(silently);
  }

  int getInt(unsigned int i)
  {
    if(i >= this->size())
      i = (unsigned int) this->size() - 1;

    return (int)(*this)[i];
  }

  double getDbl(unsigned int i)
  {
    if(i >= this->size())
      i = (unsigned int) this->size() - 1;

    return (double)(*this)[i];
  }

  const char *getStr(unsigned int i)
  {
    if (i < this->size())
      return this->getEnumTag((*this)[i]);

    return NULL;
  }

  //TODO: use base class or member-only specialization to reuse PiPoVarSizeAttr::remove
  void remove (int pos) // remove element at index pos
  {
    if (pos >= 0  &&  pos < (int) size())
      erase(begin() + pos);
  }
};


// specialisation of PiPoVarSizeAttr template for pipo atom type
template <>
class PiPoVarSizeAttr<PiPo::Atom> : public PiPo::Attr, public std::vector<PiPo::Atom>
{
public:
    PiPoVarSizeAttr(PiPo *pipo, const char *name, const char *descr, bool changesStream, unsigned int size = 0, int initVal = 0) :
    Attr(pipo, name, descr, &typeid(const char *), changesStream, false, true)
    {
        for(unsigned int i = 0; i < this->size(); i++)
            (*this)[i] = PiPo::Atom(initVal);
    }

    void clone(Attr *other) { *(dynamic_cast<std::vector<PiPo::Atom> *>(this)) = *(dynamic_cast<std::vector<PiPo::Atom> *>(other)); }

    unsigned int setSize(unsigned int size) { this->resize(size, PiPo::Atom(0)); return size; }
    unsigned int getSize(void) { return (unsigned int) this->size(); }

    void set(unsigned int i, int val, bool silently = false)
    {
      if (i >= this->size())
        setSize(i + 1);

      (*this)[i] = PiPo::Atom(val);

      this->changed(silently);
    }

    void set(unsigned int i, double val, bool silently = false)
    {
      if (i >= this->size())
        setSize(i + 1);

      (*this)[i] = PiPo::Atom(val);

      this->changed(silently);
    }

    void set(unsigned int i, const char *val, bool silently = false)
    {
      if (i >= this->size())
        setSize(i + 1);

      (*this)[i] = PiPo::Atom(val);

      this->changed(silently);
    }

    int getInt(unsigned int i)
    {
      if(i >= this->size())
        i = (unsigned int) this->size() - 1;

      return (*this)[i].getInt();
    }

    double getDbl(unsigned int i)
    {
      if(i >= this->size())
        i = (unsigned int) this->size() - 1;

      return (*this)[i].getDouble();
    }

    const char *getStr(unsigned int i)
    {
      if(i >= this->size())
        i = (unsigned int) this->size() - 1;

      return (*this)[i].getString();
    }

    PiPo::Atom *getPtr()  // return pointer to first data element
    {
      return &((*this)[0]);
    }
}; // end class PiPoVarSizeAttr<PiPo::Atom>

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset:2
 * End:
 */

#endif
