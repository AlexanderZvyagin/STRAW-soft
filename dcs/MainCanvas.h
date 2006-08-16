#ifndef include__MainCanvas
#define include__MainCanvas

#include "TCanvas.h"

class MainCanvas : public TCanvas
{
  public:
    MainCanvas(const char *name, const char *title="", Int_t form=1) :
        TCanvas(name,title,form) {Init();}
    MainCanvas(const char *name, const char *title, Int_t ww, Int_t wh) :
        TCanvas(name,title,ww,wh) {Init();}

    void                Init                    (void) {fDrawVoltage=fDrawCurrent=fDrawTemperature=fDrawAvailability=true;}

    bool                GetDrawVoltage          (void) const {return fDrawVoltage;}
    bool                GetDrawCurrent          (void) const {return fDrawCurrent;}
    bool                GetDrawTemperature      (void) const {return fDrawTemperature;}
    bool                GetDrawAvailability     (void) const {return fDrawAvailability;}

    void                SetDrawVoltage          (Bool_t f=kTRUE) {fDrawVoltage=f;} // *TOGGLE*
    void                SetDrawCurrent          (Bool_t f=kTRUE) {fDrawCurrent=f;} // *TOGGLE*
    void                SetDrawTemperature      (Bool_t f=kTRUE) {fDrawTemperature=f;} // *TOGGLE*
    void                SetDrawAvailability     (Bool_t f=kTRUE) {fDrawAvailability=f;} // *TOGGLE*

  private:

    bool                fDrawVoltage;
    bool                fDrawCurrent;
    bool                fDrawTemperature;
    bool                fDrawAvailability;

    ClassDef(MainCanvas,1)  // Many boxes
};

#endif
