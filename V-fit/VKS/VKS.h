#ifndef include__VKS
#define include__VKS

#include "../VS/VS.h"
#include "../VK/VK.h"

class VKS: public VS, public VK
{
  public:
    
    void                VCreate                 (const char *src,VFitResult &result);

    void                VFit                    (VFitResult &result);
};

#endif // include__VKS
