#ifndef include__VK
#define include__VK

#include "V.h"

class VK: public V
{
  public:
    
    void                VCreate                 (const char *src,const char *det, int ch_start, int ch_end, int pos, int delta,const char *cuts,std::vector<V::VData> &v_data);

    void                VFit                    (VFitResult &result);

    void                VStore                  (void *location, const std::string &name, const VFitResult &result);
};

#endif // include__VK
