
#ifndef _MIMO_H_INCLUDED_
#define _MIMO_H_INCLUDED_

#include "PiPo.h"
#include <memory>



/** interface for a class that holds the mimo module-specific model parameters resulting from training
*/
class mimo_model_data
{
public:
  /** output as json string into out
      Throws an exception if string would exceed size.

      @param out	string buffer of length \p{size}
      @param size	size of string buffer
      @return		returns the out pointer for convenience

      N.B.: the mimo module might be loaded from a dynamic library, so
      we can't return a complex object such as std::string that
      internatlly uses heap allocations, since the dylib's heap is a
      separate one from the main app */
  virtual char *to_json (char *out, int size) throw() = 0;

  /** get model from json string */
  virtual int from_json (const char *json_string) = 0;

  // virtual const int foo() = 0;
};



/** base class for a Mimo module (modular interface for modeling objects)

    every mimo module inherits the basic stream description and data
    passing methods (streamAttributes() and frames()) from PiPo, but
    ignores real-time oriented methods (segment()), and adds iteration
    for training.
*/

class Mimo : public PiPo
{
public:
  // constructor
  Mimo (PiPo::Parent *parent, Mimo *receiver = NULL)
  : PiPo(parent, receiver)
  {  }
  
  Mimo (const Mimo &other)
  : PiPo(other)
  {  }
  
  /** the PiPo::frames() method performs decoding */
  //virtual int frames (...);

  /** prepare for training

      @param numbuffers	number of buffers with training data
      @param numtracks	number of tracks per input buffer with training data
      @param bufsizes	array[numbuffers] of numbers of frames for each input buffer
      @param streamattr	array[numtracks] attributes of input data for each input track
      @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
  */
  virtual int setup (int numbuffers, int numtracks, int bufsizes[], const PiPoStreamAttributes *streamattr[]) = 0;
    
  /** the train method performs one iteration of training.

      It is called for each input track with a list of buffer data, receiving the training data sets.
      The first iteration receives the original data, further iterations the training output data of previous iterations, 
      that each iteration can output by calling propagateTrain().
      For multi-modal training (with more than one input track), only the call for the last track may call propagateTrain().

      @param itercount		number of current iteration (starts at zero)
      @param trackindex		index of current input track (up to numtracks - 1)
      @param numbuffers		number of buffers
      @param bufsizes		array[numbuffers] of number of frames in @p{bufdata[i]}
      @param bufdata		array[numbuffers] of input data in format given by @p{streamattr}
      @param timetags		array[numbuffers] of pointers to time tags or NULL when sampled
      @param varsize 		array[numbuffers] of pointers to row sizes or NULL when constant size
      @return			status flag: continue training (> 0), stop training (== 0), error (< 0)
  */
  virtual int train (int itercount, int trackindex, int numbuffers, int bufsizes[], const PiPoValue *bufdata[], const double *timetags[], const int *varsize[] = NULL) = 0;
  
  virtual int train (int itercount, int trackindex, int numbuffers, int bufsizes[], const PiPoValue *bufdata, const double starttime[], const int *varsize[] = NULL)
  {
    // for convenience: default implementation for mimo's that don't need to know about time: fall back to timetagged implementation providing NULL pointer
    return train (itercount, trackindex, numbuffers, bufsizes, bufdata, (const double *) NULL, varsize);
  }

  
  /** return recommended max number of iterations, or 0 for no limit. 
      This can be overridden by the user via an attribute 
      N.B.: The host should ask for maxiter at every iteration, so that a training algorithm can adapt its recommendation to the training progress */
  virtual int maxiter() { return 0; /* unlimited */ }

  /** return error, distance or loss metric from training */
  virtual double getmetric() { return 0; /* whatever */ };

  /** return trained model parameters */
  virtual mimo_model_data *getmodel () = 0;

  int propagateSetup (int numbuffers, int numtracks, int bufsize[], const PiPoStreamAttributes *streamattr[])
  {
    int ret = 0;
    
    for(unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = dynamic_cast<Mimo *>(this->receivers[i])->setup(numbuffers, 1, bufsize, &streamattr[0]);
      
      if(ret < 0)
        break;
    }
    
    return ret;
  }

  int propagateTrain (int itercount, int trackindex, int numbuffers, int bufsizes[], const PiPoValue *bufdata[], const double *timetags[], const int *varsize[])
  {
    int ret = 0;
    
    for (unsigned int i = 0; i < this->receivers.size(); i++)
    {
      ret = dynamic_cast<Mimo *>(this->receivers[i])->train(itercount, trackindex, numbuffers, bufsizes, bufdata, timetags, varsize);
      
      if (ret < 0)
        break;
    }
    
    return ret;
  }
};


/** EMACS **
 * Local variables:
 * mode: c++
 * c-basic-offset:2
 * End:
 */

#endif // defined _MIMO_H_INCLUDED_
