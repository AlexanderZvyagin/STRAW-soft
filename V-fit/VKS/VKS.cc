#include "VKS.h"

void VKS::VCreate(const char *src,VFitResult &result)
{
    VS::VCreate(src,result);
}

void VKS::VFit(VFitResult &result)
{
    VK::VFit(result);
}
