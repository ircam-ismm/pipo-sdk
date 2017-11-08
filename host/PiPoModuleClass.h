/**
 *
 * @file PiPoModuleClass.h
 *
 * @brief extension of PiPo for use in host apps
 *
 * Copyright (C) 2012-2016 by IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 *
 */

#ifndef _PIPO_MODULE_CLASS_
#define _PIPO_MODULE_CLASS_

#include "PiPo.h"
#include "PiPoCollection.h"

// this allows to load and use modules compiled as dynammic libraries :

#define PIPO_MODULE_CLASS(pipoName, pipoClass) \
  extern "C" const char *getPiPoName(void); \
  extern "C" PiPoCreatorBase *getPiPoCreator(); \
  const char *getPiPoName() { return pipoName; }\
  PiPoCreatorBase *getPiPoCreator() { return new PiPoCreator<pipoClass>; }

#endif /* _PIPO_MODULE_CLASS_ */


