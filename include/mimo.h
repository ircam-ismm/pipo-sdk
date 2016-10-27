
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
  
  
    /** the frames method performs decoding */
    //virtual int frames (...);

    /** prepare for training

    	@param streamattr	attributes of input data
	@return 0 for ok or a negative error code (to be specified), -1 for an unspecified error
    */
    virtual int setup (PiPoStreamAttributes *streamattr) = 0;
    
    /** the train method receives the training data set and performs one iteration of training 
	Initialisation is done on the first iteration that receives the original data.
	Each iteration can output transformed input data by calling propagateTrain().

	@param itercount	number of current iteration (starts at zero)
	@param streamattr	attributes of input @p{data}
	@param numframes	number of frames in @p{data}
	@param data		input data in format given by @p{streamattr}
	@param timetags		pointer to @p{numframes} time tags or NULL when sampled
	@param varsize 		pointer to @p{numframes} of row sizes or NULL when constant size
	@return			status flag: continue training (> 0), stop training (== 0), error (< 0)

     */
    virtual int train (int itercount, int numframes, PiPoValue *data, double *timetags, int *varsize) = 0;

    /** return recommended max number of iterations, or 0 for no limit. 
	This can be overridden by the user via an attribute */
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
