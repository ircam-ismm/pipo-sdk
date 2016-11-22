
#ifndef _MIMO_H_INCLUDED_
#define _MIMO_H_INCLUDED_

#include "PiPo.h"


/* interface for a class that holds the mimo module-specific model parameters resulting from training
 */
class mimo_model_data
{
    /** output as json string */
    virtual std::string to_json () = 0;

    /** get model from json string */
    virtual int from_json (std::string json) = 0;
};

/*

  every mimo module inherits the basic stream description and data
  passing methods (streamAttributes() and frames()) from PiPo, but
  ignores real-time oriented methods (segment()), and adds iteration
  for training.
 */

class mimo : public PiPo
{
public:
  // constructor
  mimo (PiPo::Parent *parent, mimo *receiver = NULL)
  : PiPo(parent, receiver)
  {
  };
  
  mimo (const mimo &other)
  : PiPo(other)
  {
  }
  
  /** the PiPo::frames() method performs decoding */
  //virtual int frames (...);

  /** prepare for training

      @param numbuffers	number of buffers with training data
      @param numtracks	number of tracks per input buffer with training data
      @param streamattr	array[numtracks] attributes of input data for each input track
      @return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
  */
  virtual int setup (int numbuffers, int numtracks, PiPoStreamAttributes *streamattr[]) = 0;
    
  /** the train method is called for each buffer and each input track, receiving the training data set, and performs one iteration of training.
      The first iteration receives the original data, further iterations the training output data of previous iterations, 
      that each iteration can output by calling propagateTrain().
      For multi-modal training (with more than one input track), only the call for the last track calls propagateTrain().

      @param itercount		number of current iteration (starts at zero)
      @param bufferindex	index of current input buffer (up to numbuffers - 1)
      @param trackindex		index of current input track (up to numtracks - 1)
      @param numframes		number of frames in @p{data}
      @param data		input data in format given by @p{streamattr}
      @param timetags		pointer to @p{numframes} time tags or NULL when sampled
      @param varsize 		pointer to @p{numframes} of row sizes or NULL when constant size
      @return			status flag: continue training (> 0), stop training (== 0), error (< 0)

      N.B.: we could replace the constraints on calling sequence by a more complicated input format where pointers to data and attributes of each input track are passed as arrays, that would penalise the vast majority of single-track use cases:

      virtual int train (int itercount, int bufferindex, int numframes[], PiPoValue *data[], double *timetags[], int *varsize[]) = 0;
  */
  virtual int train (int itercount, int bufferindex, int trackindex, int numframes, PiPoValue *data, double *timetags, int *varsize) = 0;
  
  /** return recommended max number of iterations, or 0 for no limit. 
      This can be overridden by the user via an attribute 
      N.B.: The host should ask for maxiter at every iteration, so that a training algorithm can adapt its recommendation to the training progress */
  virtual int maxiter() { return 0; /* unlimited */ }

  /** return error, distance or loss metric from training */
  virtual double getmetric() { return 0; /* whatever */ };

  /** return trained model parameters */
  virtual mimo_model_data *getmodel () = 0;
};


/** EMACS **
 * Local variables:
 * mode: c
 * c-basic-offset:2
 * End:
 */

#endif // defined _MIMO_H_INCLUDED_
