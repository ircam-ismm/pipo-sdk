#ifndef _PIPO_OP_
#define _PIPO_OP_

#include "PiPo.h"

#include <string>
#include <vector>

const float PIPO_MIN_SDK_VERSION_REQUIRED = 0.2;

/**
 * abstract base class for a container of a pipo module for PiPoModuleFactory
 */
class PiPoModule
{
public:
  PiPoModule(void) { };
  ~PiPoModule(void) { };
};

class PiPoModuleFactory
{
public:
  PiPoModuleFactory(void) { };
  ~PiPoModuleFactory(void) { };

  virtual PiPo *create(unsigned int index, const std::string &pipoName, const std::string &instanceName, PiPoModule *&module) = 0;
};

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
      this->pipo = moduleFactory->create(index, this->pipoName, this->instanceName, this->module);

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

  PiPo *getPiPo() { return this->pipo; };
  const char *getInstanceName() { return this->instanceName.c_str(); };
  bool isInstanceName(const char *str) { return (this->instanceName.compare(str) == 0); };
};

/** EMACS **
 * Local variables:
 * mode: c
 * c-basic-offset:2
 * End:
 */

#endif /* _PIPO_OP_ */
