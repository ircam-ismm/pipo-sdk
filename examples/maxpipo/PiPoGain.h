/**
 *
 * @file PiPoGain.h
 * @author Diemo.Schwarz@ircam.fr
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

  /* Configure PiPo module according to the input stream attributes and propagate output stream attributes.
   * Note: For audio input, one PiPo frame corresponds to one sample frame, i.e. width is the number of channels, height is 1, maxFrames is the maximum number of (sample) frames passed to the module, rate is the sample rate, and domain is 1 / sample rate.
   */
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

#endif
