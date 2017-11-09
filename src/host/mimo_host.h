/**
 * @file mimo_host.h
 * @author Diemo.Schwarz@ircam.fr
 *
 * @brief Modular Interface for Multi-modal Objects. Extends PiPo possibilities.
 *
 * Every mimo module inherits the basic stream description and data
 * passing methods (streamAttributes() and frames()) from PiPo, but
 * ignores real-time oriented methods (segment()), and adds iteration
 * for training.
 *
 * @copyright
 * Copyright (c) 2016-2017 by IRCAM – Centre Pompidou, Paris, France.
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

#include "PiPoChain.h"

class mimo_host : public PiPoChain
{
  /* for mimo, the streamAttributes() method should give the size of the input data as maxframes */

  /* push data through mimo modules
   */
  int run_training (PiPoValue *indata, int numframes)
  {
    int itercount = 0;
    size = width_ * height_;

    // first iteration on imput data, modules output passed to our mimo_receiver
    int status = iterate(0, indata, numframes);

    while ((this->maxiter() == 0  ||  itercount < this->maxiter())  &&  status == 0)
    {
        itercount++;
        status = iterate(itercount, indata, numframes);
    }
  }

  int iterate (int itercount, PiPoValue *data, numframes)
  {
    int status = 0;

    for (int buf = 0; buf < numbuffers; buf++)
      for (int track = 0; track < numtracks; track++)
        if ((status = getHead()->train(itercount, buf, track, numframes, data, timetags, varsize)) != 0)
          return status;

    return status;
  }
}


/*
  reveives data iterated upon by training mimo modules
 */
class mimo_receiver : mimo
{
  PiPoValue *outputdata_;

  int frames (PiPoValue *values)
  {
    outputdata_ = values;
  }

  // store or merge input data transformed by one iteration of training
  int train ()
  {
  }
}
