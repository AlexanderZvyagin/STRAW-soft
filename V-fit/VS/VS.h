#ifndef include__VS
#define include__VS

#include "V.h"

class TDirectory;
class TH1;
class TH2;
class TGraphErrors;

class VS: public V
{
  public:
    
    virtual            ~VS                      (void) {}
    void                VCreate                 (const char *src,VFitResult &result);

    void                VFit                    (VFitResult &result);

    void                CalculateRT             (VFitResult &result,float dx,const std::vector<float> &rrr);
    void                CalculateRT2            (VFitResult &result);
    static float        GetT0                   (TH1 &h_p);
    static TGraphErrors*FitV                    (TH2 &h, VFitResult &result);
};

#endif // include__VS
