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

#include "PiPo.h"

class PiPoOut;

//================================= PiPoHost =================================//

// this class is meant to be a base class, child classes should override the
// "onNewFrame" method

class PiPoHost : public PiPo::Parent {
  friend class PiPoOut;

private:
  std::string graphName;
  PiPo *graph;
  PiPoOut *out;

  PiPoStreamAttributes inputStreamAttrs;
  PiPoStreamAttributes outputStreamAttrs;

public:
  PiPoHost();
  ~PiPoHost();

  virtual bool setGraph(std::string name);
  virtual void clearGraph();

  // override this method when inheriting !!!
  virtual void onNewFrame(double time, double weight, PiPoValue *values, unsigned int size);

  virtual std::vector<PiPoValue> getLastFrame();

  virtual int setInputStreamAttributes(PiPoStreamAttributes &sa, bool propagate = true);

  virtual PiPoStreamAttributes getOutputStreamAttributes();

  virtual int frames(double time, double weight, PiPoValue *values, unsigned int size,
                     unsigned int num);

  // virtual bool setAttr(const std::string &attrName, bool value);
  // virtual bool setAttr(const std::string &attrName, int value);
  virtual bool setAttr(const std::string &attrName, double value);
  virtual bool setAttr(const std::string &attrName, const std::vector<double> &values);
  virtual bool setAttr(const std::string &attrName, const std::string &value); // for enums

  // virtual const std::vector<std::string>& getAttrNames();
  // virtual bool isBoolAttr(const std::string &attrName);
  // virtual bool isEnumAttr(const std::string &attrName);
  // virtual bool isIntAttr(const std::string &attrName);
  // virtual bool isIntArrayAttr(const std::string &attrName);
  // virtual bool isFloatAttr(const std::string &attrName);
  // virtual bool isFloatArrayAttr(const std::string &attrName);
  // virtual bool isStringAttr(const std::string &attrName);

  virtual std::vector<std::string> getAttrNames();

  virtual double getDoubleAttr(const std::string &attrName);

  virtual std::vector<double> getDoubleArrayAttr(const std::string &attrName);

  virtual std::string getEnumAttr(const std::string &attrName);

private:
  int propagateInputStreamAttributes();

  void setOutputStreamAttributes(bool hasTimeTags, double rate, double offset,
                           unsigned int width, unsigned int height,
                           const char **labels, bool hasVarSize,
                           double domain, unsigned int maxFrames);
};

#endif /* _PIPO_HOST_*/
