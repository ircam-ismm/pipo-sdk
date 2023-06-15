
#include "maxmimohost.h"

Mimo *MaxMimoHost::set_module (const char *str, Mimo *receiver)
{
#if USE_PIPO_GRAPH
  if (chain.create(str, false)) // does instantiate and wire to connect graph pipos
  {
    //return dynamic_cast<Mimo *>(&chain);
    return dynamic_cast<Mimo *>(chain.getPiPo((unsigned int)0));
  }
#else
  this->chain.clear();
  
  if (this->chain.parse(str) > 0 && this->chain.instantiate() && this->chain.connect(receiver))
    return dynamic_cast<Mimo *>(this->chain.getHead());
#endif
  else
    return NULL;
}
