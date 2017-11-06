/**
 * @file PiPoOut.h
 * @author joseph.larralde@ircam.fr
 *
 * @brief PiPo specific to PiPoHost that acts as a new frame observer.
 *
 * @copyright
 * Copyright (c) 2017 by IRCAM – Centre Pompidou, Paris, France.
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

#ifndef _PIPO_OUT_
#define _PIPO_OUT_

#define PIPO_OUT_RING_SIZE 2

#include "PiPoHost.h"

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

#endif /* _PIPO_OUT_ */