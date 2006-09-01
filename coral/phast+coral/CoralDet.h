#ifndef CoralDet__include
#define CoralDet__include

#include "CsDet.h"
#include "CsRTRelation.h"

#include "Detectors/DriftDetector.h"

class CsTrack;
class CsHelix;
class CsStrawTubesDetector;
class CsDriftChamberDetector;
class CsDWDetector;
class CsRichWallDetector;

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
        DriftDetectorCORAL      co;
        CoralDet *              coral_det;
    };

  public:
    virtual            ~CoralDet                (void) {}
                        CoralDet                (CsDet &);
  public:
  
    const
    DriftDetector &     GetDriftDet             (void) const {return const_cast<CoralDet*>(this)->GetDriftDet();}

    DriftDetector &     GetDriftDet             (void)       {if(drift_det==NULL)throw "CoralDet::GetDriftDet() empty!";return *drift_det;}
  
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
                        PropogateAndCheck       (const CsTrack &track_MRS, Track3D &trackMRS, Track3D &trackDRS);

    /*! Double hit analysis.
    */
    virtual bool        DoubleHitsAnalyse       (Hit &h1,Hit &h2);

    virtual bool        IsDoubleLayer           (const string &s) const;
    
    CsDetector *        DetCoral                (void) {return reinterpret_cast<CsDetector*>(det);}
    virtual float       DistRTCoral             (float t) const {return -1;}
    
  public:

    DriftDetector *     drift_det;
    CsDet *             det;
    float               misalignment;
};

class CoralDetST: public CoralDet
{
  public:
                        CoralDetST              (CsDet &d) : CoralDet(d) {misalignment=0.1;}
    bool                IsDoubleLayer           (const string &s) const;
    const CsStrawTubesDetector *DetCoral        (void) const {return reinterpret_cast<const CsStrawTubesDetector*>(det);}
    float               DistRTCoral             (float t) const;
};

class CoralDetDW: public CoralDet
{
  public:
                        CoralDetDW              (CsDet &d) : CoralDet(d) {misalignment=1;}
    bool                IsDoubleLayer           (const string &s) const;
    const CsDWDetector *        DetCoral        (void) const {return reinterpret_cast<const CsDWDetector*>(det);}
    float               DistRTCoral             (float t) const;
};

class CoralDetDR: public CoralDet
{
  public:
                        CoralDetDR              (CsDet &d) : CoralDet(d) {misalignment=1;}
    bool                IsDoubleLayer           (const string &s) const;
    const CsRichWallDetector * DetCoral         (void) const {return reinterpret_cast<const CsRichWallDetector*>(det);}
    float               DistRTCoral             (float t) const;
};

class CoralDetDC: public CoralDet
{
  public:
                        CoralDetDC              (CsDet &d) : CoralDet(d) {misalignment=0.1;}
    bool                IsDoubleLayer           (const string &s) const;
    const CsDriftChamberDetector *DetCoral      (void) const {return reinterpret_cast<const CsDriftChamberDetector*>(det);}
    float               DistRTCoral             (float t) const;
};

class CoralDets: public Detectors
{
  public:
                        CoralDets               (void) : tracks_counter(0) {}

    static CoralDet *   Create                  (CsDet &d);

    void                AddDetectors            (void);
    bool                MakeAssociation         (const CsTrack &t,CsDet &d,float misalignment=0);
    void                DoubleHitsAnalyse       (void);
    void                Fill                    (void);
    void                Clear                   (void) {hits.clear();}

    int                   tracks_counter;
    vector<CoralDet::Hit> hits;
    map<CsDet*,CoralDet*> dets;
};

}

#endif // CoralDet__include
