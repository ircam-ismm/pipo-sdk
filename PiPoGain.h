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
  PiPoScalarAttr<double> factor;
  PiPoScalarAttr<const char *> colname;
  
  PiPoGain (Parent *parent, PiPo *receiver = NULL)
  : PiPo(parent, receiver), 
    factor(this, "factor", "Gain Factor", false, 1.0),
    colname(this, "suffix", "Output Column Name Suffix", true, "Amplified")
  { }
  
  ~PiPoGain (void)
  { }
  
  int streamAttributes (bool hasTimeTags, double rate, double offset,
			unsigned int width, unsigned int size,
			const char **labels, bool hasVarSize,
			double domain, unsigned int maxFrames)
  {
    // the following shows how to set output labels depending on input labels
    // (although this is a rather contrived example)
    // if labels don't need changing, they can just be passed on
    std::string  *outstr     = new std::string[width];
    const char  **outlabels  = new const char*[width];
    std::string   suffix_str = std::string(colname.get());

    for (unsigned int i = 0; i < width; i++)
    {
        outstr[i]    = std::string(labels ? labels[i] : ""  + suffix_str);
	outlabels[i] = outstr[i].c_str();
    }
      
    int ret = propagateStreamAttributes(hasTimeTags, rate, offset, width, size, outlabels, hasVarSize, domain, maxFrames);
    
    delete [] outlabels;
    delete [] outstr;
      
    return ret;
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
