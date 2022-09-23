/**
 * @file PiPoHost.cpp
 * @author Norbert.Schnell@ircam.fr
 *
 * @brief Plugin Interface for Processing Objects host classes.
 *
 * A PiPo host is built around the class PiPoChain that represents a sequence of
 * PiPo modules piping data into each other. See details of the steps there.
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

#define PIPO_OUT_RING_SIZE 2

#include <iostream>

#include "PiPoHost.h"
#include "PiPoCollection.h"

//================================= PiPoHost =================================//

// this class is meant to be a base class, child classes should override the
// "onNewFrame" method

PiPoHost::PiPoHost() :
inputStreamAttrs(PIPO_MAX_LABELS),
outputStreamAttrs(PIPO_MAX_LABELS)
{
  PiPoCollection::init();
  this->out = new PiPoOut(this);
  this->graph = nullptr;
}

PiPoHost::~PiPoHost()
{
  this->clearGraph();
  delete this->out;
}

// implementation of PiPo::Parent methods
void
PiPoHost::streamAttributesChanged(PiPo *pipo, PiPo::Attr *attr)
{
  this->propagateInputStreamAttributes();
}

void
PiPoHost::signalError(PiPo *pipo, std::string errorMsg)
{
  std::cout << "Error : " << errorMsg << std::endl;
}

void
PiPoHost::signalWarning(PiPo *pipo, std::string errorMsg)
{
  std::cout << "Warning : " << errorMsg << std::endl;
}

// own methods
bool
PiPoHost::setGraph(std::string name)
{
  if (this->graph != nullptr)
  {
    delete this->graph;
  }

  this->graph = PiPoCollection::create(name, static_cast<PiPo::Parent *>(this));

  if (this->graph != nullptr)
  {
    this->graphName = name;
    this->graph->setReceiver((PiPo *)this->out);
    return true;
  }

  this->graph = nullptr;
  this->graphName = "undefined";
  return false;
}

void
PiPoHost::clearGraph()
{
  if (this->graph != nullptr)
  {
    delete this->graph;
    this->graph = nullptr;
  }
}

// override this method when inheriting !!!
// void
// PiPoHost::onNewFrame(std::function<void (double, double, PiPoValue *, unsigned int)> f)
// {
//   this->frameCallback = f;
// }

// override this method when inheriting !!!
void
PiPoHost::onNewFrame(double time, double weight, PiPoValue *values, unsigned int size)
{
  std::cout << time << std::endl;
  std::cout << "please override this method" << std::endl;
}

std::vector<PiPoValue>
PiPoHost::getLastFrame()
{
  return this->out->getLastFrame();
}

int
PiPoHost::setInputStreamAttributes(const PiPoStreamAttributes &sa, bool propagate)
{
  this->inputStreamAttrs = sa;

  if (propagate)
  {
    return this->propagateInputStreamAttributes();
  }

  return 0;
}

PiPoStreamAttributes&
PiPoHost::getOutputStreamAttributes()
{
  return this->outputStreamAttrs;
}

int
PiPoHost::frames(double time, double weight, PiPoValue *values, unsigned int size,
                 unsigned int num)
{
  return this->graph->frames(time, weight, values, size, num);
}

std::vector<std::string>
PiPoHost::getAttrNames()
{
  std::vector<std::string> res;

  for (unsigned int i = 0; i < this->graph->getNumAttrs(); ++i)
  {
    res.push_back(this->graph->getAttr(i)->getName());
  }

  return res;
}


PiPo::Attr *PiPoHost::getAttr (const std::string &attrName)
{
  return this->graph->getAttr(attrName.c_str());
}

bool
PiPoHost::setAttr(const std::string &attrName, bool value)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    int iAttr = attr->getIndex();
    return this->graph->setAttr(iAttr, value ? 1 : 0);
  }

  return false;
}


bool PiPoHost::setAttr(const std::string &attrName, const std::string &value) // for enums and strings
{
  return setAttr(attrName, value.c_str());
}

bool PiPoHost::setAttr(const std::string &attrName, const char *value) // for enums and strings
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Enum)
    {
      std::vector<const char *> *list = attr->getEnumList();

      for (int i = 0; i < list->size(); i++)
      {
        if (strcmp(list->at(i), value) == 0)
        {
          attr->set(0, i);
          return true;
        }
      }
    }
    else if (type == PiPo::Type::String)
    {
      attr->set(0, value);
    }
  }

  return false;
}

bool
PiPoHost::setAttr(const std::string &attrName, int value)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    int iAttr = attr->getIndex();
    return this->graph->setAttr(iAttr, value);
  }

  return false;
}

bool
PiPoHost::setAttr(const std::string &attrName, double value)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    int iAttr = attr->getIndex();
    return this->graph->setAttr(iAttr, value);
  }

  return false;
}

#if 1

template<typename TYPE>
bool PiPoHost::setAttr(const std::string &attrName, const std::vector<TYPE> &values)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    int iAttr = attr->getIndex();
    TYPE vals[values.size()];

    for (unsigned int i = 0; i < values.size(); ++i)
    {
      vals[i] = values[i];
    }

    return this->graph->setAttr(iAttr, &vals[0], static_cast<unsigned int>(values.size()));
  }

  return false;
}

#else

bool
PiPoHost::setAttr(const std::string &attrName, const std::vector<int> &values)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    int iAttr = attr->getIndex();
    int vals[values.size()];

    for (unsigned int i = 0; i < values.size(); ++i)
    {
      vals[i] = values[i];
    }

    return this->graph->setAttr(iAttr, &vals[0], static_cast<unsigned int>(values.size()));
  }

  return false;
}

bool
PiPoHost::setAttr(const std::string &attrName, const std::vector<double> &values)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    int iAttr = attr->getIndex();
    double vals[values.size()];

    for (unsigned int i = 0; i < values.size(); ++i)
    {
      vals[i] = values[i];
    }

    return this->graph->setAttr(iAttr, &vals[0], static_cast<unsigned int>(values.size()));
  }

  return false;
}

bool
PiPoHost::setAttr(const std::string &attrName, const std::vector<const char *> &values)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    int iAttr = attr->getIndex();
    double vals[values.size()];

    for (unsigned int i = 0; i < values.size(); ++i)
    {
      vals[i] = values[i];
    }

    return this->graph->setAttr(iAttr, &vals[0], static_cast<unsigned int>(values.size()));
  }

  return false;
}
#endif

//--------------------------- TYPE INTROSPECTION -----------------------------//

bool
PiPoHost::isBoolAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    return (attr->getType() == PiPo::Type::Bool);
  }

  return false;
}

bool
PiPoHost::isEnumAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    return (attr->getType() == PiPo::Type::Enum);
  }

  return false;
}

std::vector<std::string>
PiPoHost::getAttrEnumList(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Enum)
    {
      std::vector<const char *> *list = attr->getEnumList();
      std::vector<std::string> res(list->size());

      for (int i = 0; i < list->size(); i++)
      {
        res[i] = std::string(list->at(i));
      }

      return res;
    }
  }

#if (__STDC_VERSION__ >= 201103L)	/* C++11 */
  return { "" };	  
#else				/* fallback for C++98 */  
  std::vector<std::string> empty;
  empty.push_back("");
  return empty;
#endif
}

bool
PiPoHost::isStringAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    return (attr->getType() == PiPo::Type::String);
  }

  return false;
}

bool
PiPoHost::isIntAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    return (attr->getType() == PiPo::Type::Int);
  }

  return false;
}

bool
PiPoHost::isDoubleAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    return (attr->getType() == PiPo::Type::Double);
  }

  return false;
}

//------------------------------ VALUE GETTERS -------------------------------//

bool
PiPoHost::getBoolAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Bool)
    {
      return attr->getInt(0) != 0;
    }
  }

  return false;
}

std::string
PiPoHost::getEnumAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Enum)
    {
      return std::string(attr->getStr(0));
    }
  }

  return "";
}

std::string
PiPoHost::getStringAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::String)
    {
      return std::string(attr->getStr(0));
    }
  }

  return "";
}

int
PiPoHost::getIntAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Int)
    {
      return attr->getInt(0);
    }
  }

  return 0;
}

double
PiPoHost::getDoubleAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Double)
    {
      return attr->getDbl(0);
    }
  }

  return 0;
}

std::vector<int>
PiPoHost::getIntArrayAttr(const std::string &attrName)
{
  std::vector<int> res;
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL) {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Int)
    {
      res.resize(attr->getSize());
      for (int i = 0; i < attr->getSize(); ++i)
      {
        res[i] = attr->getInt(i);
      }
    }
  }

  return res;
}

std::vector<double>
PiPoHost::getDoubleArrayAttr(const std::string &attrName)
{
  std::vector<double> res;
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL) {
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Double)
    {
      res.resize(attr->getSize());
      for (int i = 0; i < attr->getSize(); ++i)
      {
        res[i] = attr->getDbl(i);
      }
    }
  }

  return res;
}


int
PiPoHost::propagateInputStreamAttributes()
{
  if (this->graph != nullptr)
  {
    return this->graph->streamAttributes(this->inputStreamAttrs.hasTimeTags,
                                         this->inputStreamAttrs.rate,
                                         this->inputStreamAttrs.offset,
                                         this->inputStreamAttrs.dims[0],
                                         this->inputStreamAttrs.dims[1],
                                         this->inputStreamAttrs.labels,
                                         this->inputStreamAttrs.hasVarSize,
                                         this->inputStreamAttrs.domain,
                                         this->inputStreamAttrs.maxFrames);
  }

  return 0;
}

void
PiPoHost::setOutputStreamAttributes(bool hasTimeTags, double rate, double offset,
                                    unsigned int width, unsigned int height,
                                    const char **labels, bool hasVarSize,
                                    double domain, unsigned int maxFrames)
{
  if (labels != NULL)
  {
    int numLabels = width;

    if (numLabels > PIPO_MAX_LABELS)
    {
      numLabels = PIPO_MAX_LABELS;
    }

    for (unsigned int i = 0; i < this->outputStreamAttrs.numLabels; ++i)
    {
      // free previously allocated memory
      delete[] this->outputStreamAttrs.labels[i];
    }

    for (unsigned int i = 0; i < numLabels; i++)
    {
      this->outputStreamAttrs.labels[i] = new char[PIPO_MAX_LABELS];
      const char * label = labels[i] != NULL ? labels[i] : "";
      std::strcpy(const_cast<char *>(this->outputStreamAttrs.labels[i]), label);
    }

    this->outputStreamAttrs.numLabels = numLabels;
  }
  else
  {
    this->outputStreamAttrs.numLabels = 0;
  }

  this->outputStreamAttrs.hasTimeTags = hasTimeTags;
  this->outputStreamAttrs.rate = rate;
  this->outputStreamAttrs.offset = offset;
  this->outputStreamAttrs.dims[0] = width;
  this->outputStreamAttrs.dims[1] = height;
  this->outputStreamAttrs.hasVarSize = hasVarSize;
  this->outputStreamAttrs.domain = domain;
  this->outputStreamAttrs.maxFrames = maxFrames;
}

//================================= PiPoOut ==================================//

PiPoOut::PiPoOut(PiPoHost *host) :
PiPoOut::PiPo((PiPo::Parent *)host)
{
  this->host = host;
  writeIndex = 0;
  readIndex = 0;
  ringBuffer.resize(PIPO_OUT_RING_SIZE);
}

PiPoOut::~PiPoOut() {}

int
PiPoOut::streamAttributes(bool hasTimeTags,
                          double rate, double offset,
                          unsigned int width, unsigned int height,
                          const char **labels, bool hasVarSize,
                          double domain, unsigned int maxFrames)
{
  this->host->setOutputStreamAttributes(hasTimeTags, rate, offset, width, height,
                                        labels, hasVarSize, domain, maxFrames);

  for (int i = 0; i < PIPO_OUT_RING_SIZE; ++i)
  {
    ringBuffer[i].resize(width * height);
  }

  return 0;
}

int
PiPoOut::frames(double time, double weight, PiPoValue *values,
                unsigned int size, unsigned int num)
{
  if (num > 0)
  {
    for (int i = 0; i < num; ++i)
    {
      this->host->onNewFrame(time, weight, values, size);
      // this->host->frameCallback(time, weight, values, size);

      /*
      ////////// TODO : write a real lock-free queue using atomic_swap
      ////////// for pipo graphs fed by audio frames

      for (int j = 0; j < size; ++j)
      {
        ringBuffer[writeIndex][j] = values[i * size + j];
      }

      // atomic swap ?
      writeIndex = 1 - writeIndex;
      readIndex = 1 - writeIndex;
      this->host->onNewFrame(time, ringBuffer[readIndex]);

      if (writeIndex + 1 == PIPO_OUT_RING_SIZE)
      {
        writeIndex = 0;
      }
      else
      {
        writeIndex++;
      }
      //*/
    }
  }

  return 0;
}

std::vector<PiPoValue>
PiPoOut::getLastFrame()
{
  std::vector<PiPoValue> f;

  if (readIndex > -1)
  {
    f = ringBuffer[readIndex];
  }

  return f;
}

// instantiate setAttr member function templates for commly used types
template bool PiPoHost::setAttr(const std::string &, const std::vector<int> &);
template bool PiPoHost::setAttr(const std::string &, const std::vector<double> &);
template bool PiPoHost::setAttr(const std::string &, const std::vector<const char *> &);
