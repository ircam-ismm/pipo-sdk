/**
 *
 * @file PiPoModule.h
 *
 * @brief extension of PiPo for JuceOscPiPoApp
 * 
 * Copyright (C) 2012-2016 by IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _PIPO_MODULE_
#define _PIPO_MODULE_

#include "PiPo.h"
#include "PiPoCollection.h"

#define PIPO_MODULE_CLASS(pipoName, pipoClass) \
     extern "C" const char *getPiPoName(void); \
     extern "C" PiPoCreatorBase *getPiPoCreator(); \
     const char *getPiPoName() { return pipoName; }\
     PiPoCreatorBase *getPiPoCreator() { return new PiPoCreator<pipoClass>; }

#endif
