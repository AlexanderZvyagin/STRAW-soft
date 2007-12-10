#ifndef CoralDet__include
#define CoralDet__include

#include "CsDet.h"
#include "CsRTRelation.h"

#include "Detectors/DriftDetector.h"
#include "Detectors/DetDat.h"

class CsTrack;
class CsHelix;
class CsStrawTubesDetector;
class CsDriftChamberDetector;
class CsDWDetector;

namespace CS {

/*!  Interface to CORAL.
*/
class CoralDet
{
  public:

    /*! Single hit.
    */
    class Hit
    {
      public:
        Hit(const DriftDetectorCORAL &c,CoralDet *cord) : co(c),coral_det(cord) {}
        //DriftDetectorChannel*   ch;
        DriftDetectorCORAL      co;
        CoralDet *              coral_det;
    };

  public:
    virtual            ~CoralDet                (void) {delete drift_det;}
                        CoralDet                (CsDet &,const CS::DetDatLine *ddl=NULL);
  private:
                        CoralDet                (const CoralDet &);
  public:
  
    const DriftDetector *GetDriftDet            (void) const {return drift_det;}
          DriftDetector *GetDriftDet            (void)       {return drift_det;}
    bool                Extrapolate             (const CsTrack &t) const;
    bool                MakeAssociation         (const CsTrack &t,float misalignment=0);

    /*! Check that track is good
    */
    virtual bool        IsGoodTrack             (const CsTrack &t) const;

    /*! Find the best helix for use.
    */
    virtual const CsHelix *
                        FindBestHelix        (const CsTrack &track_MRS);

    /*! Propagate track to detector and check is it usefull for analysis.
    
        \return NULL if association has failed.
    */
    virtual const CsHelix *
                        PropagateAndCheck       (const CsTrack &track_MRS, Track3D &trackMRS, Track3D &trackDRS);

    /*! Double hit analysis.
    */
    virtual bool        DoubleHitsAnalyse       (Hit &h1,Hit &h2);

    virtual bool        IsDoubleLayer           (const string &s) const;
    
    CsDetector *        DetCoral                (void) {return reinterpret_cast<CsDetector*>(det);}
    virtual float       DistRTCoral             (float t) const {return -1;}
    
  public:

    CsDet *             det;
    DriftDetector *     drift_det;
    float               misalignment;
};

class CoralDetST: public CoralDet
{
  public:
                        CoralDetST              (CsDet &d,const CS::DetDatLine *ddl=NULL) : CoralDet(d,ddl) {misalignment=0.1;}
    bool                IsDoubleLayer           (const string &s) const;
    const CsStrawTubesDetector *DetCoral        (void) const {return reinterpret_cast<const CsStrawTubesDetector*>(det);}
    float               DistRTCoral             (float t) const;
};

class CoralDetDW: public CoralDet
{
  public:
                        CoralDetDW              (CsDet &d,const CS::DetDatLine *ddl=NULL) : CoralDet(d,ddl) {misalignment=1;}
    bool                IsDoubleLayer           (const string &s) const;
    const CsDWDetector *        DetCoral        (void) const {return reinterpret_cast<const CsDWDetector*>(det);}
    float               DistRTCoral             (float t) const;
};

class CoralDetDC: public CoralDet
{
  public:
                        CoralDetDC              (CsDet &d,const CS::DetDatLine *ddl=NULL) : CoralDet(d,ddl) {misalignment=0.1;}
    bool                IsDoubleLayer           (const string &s) const;
    const CsDriftChamberDetector *DetCoral      (void) const {return reinterpret_cast<const CsDriftChamberDetector*>(det);}
    float               DistRTCoral             (float t) const;
};

class CoralDets
{
  public:
                        CoralDets               (const string &det_dat);

    bool                MakeAssociation         (const CsTrack &t,CsDet &d,float misalignment=0);
    void                DoubleHitsAnalyse       (void);
    void                Fill                    (void);
    void                Clear                   (void) {hits.clear();}

    int                   tracks_counter;
    vector<CoralDet::Hit> hits;
    map<string,CoralDet*> dets;
    CS::DetDat          detdat;
};

}

#endif // CoralDet__include
