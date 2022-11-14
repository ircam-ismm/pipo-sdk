/**
 * @file PiPoOp.h
 * @author Norbert.Schnell@ircam.fr
 *
 * @brief Wrapper class providing a level of abstraction for the instanciation
 * of PiPo operators.
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

#ifndef _PIPO_OP_
#define _PIPO_OP_

#include "PiPo.h"
#include "PiPoModule.h"

#include <string>
#include <vector>

const float PIPO_MIN_SDK_VERSION_REQUIRED = 0.2;

/**
 * element of pipo chain, points to pipo pipo
 */
class PiPoOp
{
  unsigned int index;
  std::string pipoName; /// pipo class name
  std::string instanceName;
  PiPo *pipo;
  PiPoModule *module;

public:
  PiPoOp(unsigned int index = 0) : pipoName(), instanceName()
  {
    this->index = index;
    this->pipo = NULL;
    this->module = NULL;
  }

//  PiPoOp(const PiPoOp &other) : pipoName(), instanceName()
//  {
//    this->index = other.index;
//    this->pipoName = other.pipoName;
//    this->instanceName = other.instanceName;
//    this->pipo = other.pipo;
//    this->module = other.module;
//  };

  ~PiPoOp(void) { };

  void setParent(PiPo::Parent *parent)
  {
    if(this->pipo != NULL)
      this->pipo->setParent(parent);
  }

  void clear(void)
  {
    if(this->module != NULL)
      delete this->module;

    this->module = NULL;

    if(this->pipo != NULL)
      delete pipo;

    this->pipo = NULL;
  }

  /**
   * parse one pipo name, and optional instance name in '(' ')'
   */
  void parse(std::string str, size_t &pos)
  {
    this->clear();

    size_t end = str.find_first_of(':', pos);
    size_t open = str.find_first_of('(', pos);
    size_t closed = str.find_first_of(')', pos);

    if(open < std::string::npos && closed < std::string::npos)
    {
      this->pipoName = str.substr(pos, open - pos);
      this->instanceName = str.substr(open + 1, closed - open - 1);
    }
    else
    {
      this->pipoName = str.substr(pos, end - pos);
      this->instanceName = this->pipoName;
    }

    if(end < std::string::npos)
      pos = end + 1;
    else
      pos = std::string::npos;
  }

  bool instantiate(PiPo::Parent *parent, PiPoModuleFactory *moduleFactory)
  {
    this->pipo = NULL;
    this->module = NULL;

    if(moduleFactory != NULL)
      this->pipo = moduleFactory->create(index, this->pipoName, this->instanceName, this->module, parent);

    if(this->pipo != NULL)
    { // check if version of created pipo is compatible with host
      if (this->pipo->getVersion() < PIPO_MIN_SDK_VERSION_REQUIRED)
      {
        printf("PiPo Host ERROR: created PiPo %s version %f is smaller than minimum required version %f\n",
        this->pipoName.c_str(), this->pipo->getVersion(), PIPO_MIN_SDK_VERSION_REQUIRED);
        //TODO: clean up: destroy unusable pipo
        return false;
      }

      this->pipo->setParent(parent);
      return true;
    }

    return false;
  }

  void set(unsigned int index, PiPo::Parent *parent, PiPoModuleFactory *moduleFactory, const PiPoOp &other)
  {
    this->index = index;
    this->pipoName = other.pipoName;
    this->instanceName = other.instanceName;

    this->instantiate(parent, moduleFactory);

    if(this->pipo != NULL)
      this->pipo->cloneAttrs(other.pipo);
  }

  PiPo *getPiPo() const { return this->pipo; }
  const char *getInstanceName() const { return this->instanceName.c_str(); }
  bool isInstanceName(const char *str) const { return (this->instanceName.compare(str) == 0); }
};

/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset:2
 * End:
 */

#endif /* _PIPO_OP_ */
