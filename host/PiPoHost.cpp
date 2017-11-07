/**
 * @file PiPoHost.h
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

#ifndef _PIPO_HOST_
#define _PIPO_HOST_

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

bool
PiPoHost::setGraph(std::string name)
{
  if (this->graph != nullptr)
  {
    delete this->graph;
  }

  this->graph = PiPoCollection::create(name);

  if (this->graph != NULL)
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
PiPoHost::setInputStreamAttributes(PiPoStreamAttributes &sa, bool propagate = true)
{
  this->inputStreamAttrs = sa;

  if (propagate)
  {
    return this->propagateInputStreamAttributes();
  }

  return 0;
}

PiPoStreamAttributes
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

// virtual bool setAttr(const std::string &attrName, bool value);
// virtual bool setAttr(const std::string &attrName, int value);
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

bool
PiPoHost::setAttr(const std::string &attrName, const std::vector<double> &values)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    int iAttr = attr->getIndex();
    double vals[values.size()];
    unsigned int i = 0;

    for (auto &value : values)
    {
      vals[i] = value;
      i++;
    }

    return this->graph->setAttr(iAttr, &vals[0], static_cast<unsigned int>(values.size()));
  }

  return false;
}

bool
PiPoHost::setAttr(const std::string &attrName, const std::string &value) // for enums
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    // int iAttr = attr->getIndex();
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Enum)
    {
      std::vector<const char *> *list = attr->getEnumList();

      for (int i = 0; i < list->size(); i++)
      {
        if (strcmp(list->at(i), value.c_str()) == 0)
        {
          attr->set(0, i);
          return true;
        }
      }
    }
  }

  return false;
}

// virtual const std::vector<std::string>& getAttrNames();
// virtual bool isBoolAttr(const std::string &attrName);
// virtual bool isEnumAttr(const std::string &attrName);
// virtual bool isIntAttr(const std::string &attrName);
// virtual bool isIntArrayAttr(const std::string &attrName);
// virtual bool isFloatAttr(const std::string &attrName);
// virtual bool isFloatArrayAttr(const std::string &attrName);
// virtual bool isStringAttr(const std::string &attrName);

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

double
PiPoHost::getDoubleAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    // int iAttr = attr->getIndex();
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Double)
    {
      return attr->getDbl(0);
    }
  }

  return 0;
}

std::vector<double>
PiPoHost::getDoubleArrayAttr(const std::string &attrName)
{
  std::vector<double> res;
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL) {
    // int iAttr = attr->getIndex();
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Double)
    {
      for (int i = 0; i < attr->getSize(); ++i)
      {
        res.push_back(attr->getDbl(i));
      }
    }
  }

  return res;
}

std::string
PiPoHost::getEnumAttr(const std::string &attrName)
{
  PiPo::Attr *attr = this->graph->getAttr(attrName.c_str());

  if (attr != NULL)
  {
    // int iAttr = attr->getIndex();
    PiPo::Type type = attr->getType();

    if (type == PiPo::Type::Enum)
    {
      return attr->getStr(0);
    }
  }

  return "";
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
  if (labels != NULL) {
    int numLabels = width;

    if (numLabels > PIPO_MAX_LABELS)
    {
      numLabels = PIPO_MAX_LABELS;
    }

    for (unsigned int i = 0; i < numLabels; i++)
    {
      try {
        this->outputStreamAttrs.labels[i] = labels[i];
      } catch(std::exception e) {
        this->outputStreamAttrs.labels[i] = "unnamed";
      }
    }

    this->outputStreamAttrs.numLabels = numLabels;
  } else {
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

class PiPoOut : public PiPo {
private:
  PiPoHost *host;
  std::atomic<int> writeIndex, readIndex;
  std::vector<std::vector<PiPoValue>> ringBuffer;

public:
  PiPoOut(PiPoHost *host) :
  PiPo((PiPo::Parent *)host)
  {
    this->host = host;
    writeIndex = 0;
    readIndex = 0;
    ringBuffer.resize(PIPO_OUT_RING_SIZE);
  }

  ~PiPoOut() {}

  int streamAttributes(bool hasTimeTags,
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

  int frames(double time, double weight, PiPoValue *values,
             unsigned int size, unsigned int num) {

    if (num > 0)
    {
      for (int i = 0; i < num; ++i)
      {
        //*
        for (int j = 0; j < size; ++j)
        {
          ringBuffer[writeIndex][j] = values[i * size + j];
        }

        // atomic swap ?
        writeIndex = 1 - writeIndex;
        readIndex = 1 - writeIndex;

        // this->host->onNewFrame(time, ringBuffer[readIndex]);

        if (writeIndex + 1 == PIPO_OUT_RING_SIZE) {
          writeIndex = 0;
        } else {
          writeIndex++;
        }
        //*/

        this->host->onNewFrame(time, weight, values, size);
      }
    }

    return 0;
  }

  std::vector<PiPoValue> getLastFrame() {
      std::vector<PiPoValue> f;

      if (readIndex > -1) {
          f = ringBuffer[readIndex];
      }

      return f;
  }
};

