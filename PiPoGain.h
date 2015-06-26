/**
 *
 * @file PiPoGain.h
 * @author Norbert.Schnell@ircam.fr
 *
 * @brief PiPo gain data stream
 *
 * Copyright (C) 2012-2014 by IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 *
 */
#ifndef _PIPO_GAIN_
#define _PIPO_GAIN_

#include "PiPo.h"

class PiPoGain : public PiPo
{
public:
  PiPoScalarAttr<bool> keeporig;
  PiPoScalarAttr<double> factor;
  
  PiPoGain (Parent *parent, PiPo *receiver = NULL)
  : PiPo(parent, receiver), 
    keeporig(this, "keeporig", "Keep Original Value", true, false),
    factor(this, "factor", "Gain Factor", false, 1.0)
  { }
  
  ~PiPoGain (void)
  { }
  
  int streamAttributes (bool hasTimeTags, double rate, double offset,
			unsigned int width, unsigned int size,
			const char **labels, bool hasVarSize,
			double domain, unsigned int maxFrames)
  {
    unsigned int frame_size = width * size;
      
    return propagateStreamAttributes(hasTimeTags, rate, offset, width, size, labels, hasVarSize, domain, maxFrames);
  }
  
  int frames (double time, double weight, PiPoValue *values, unsigned int size, unsigned int num)
  {
    double f = factor.get();    // get gain factor here, as it could change while running
    PiPoValue *ptr = values;    // work in-place
      
    for (unsigned int i = 0; i < num; i++)
    {
      for (unsigned int j = 0; j < size; j++)
	ptr[j] = ptr[j] * f;
	  
      ptr += size;
    }
    
    return propagateFrames(time, weight, values, size, num);
  }
};

#endif
