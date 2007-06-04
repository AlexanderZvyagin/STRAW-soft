#ifndef V_include
#define V_include

#include <vector>
#include <string>
#include <cstdio>

class TSQLServer;
class TF1;
class TH1;
class TH2;
class TGraph;
class TCanvas;
class TVirtualPad;
namespace CS {
class RTRelation;
}

/*! \brief Class designed for V-fit.
    
    Common interface to be used for V-fitting by programs of
    Klaus, Reiner, Sasha.
    
    The idea is to have a simple functions which could be used
    for data analysis by all of us.
    
    Do we need to define the units? Proposal: time is in [ns], length is in [mm].
*/

class V
{
  public:

    /*! \brief Handle point(s) in a V-plot.

        To be more precise it can handle many points with the same (x,t,xe) arguments.
    */
    struct VData
    {
        /*! Simple constructor, it sets all values to zeroes.
        */
        VData (void) : x(0), t(0), xe(0), w(0) {}

        /*! Print point.
        */
        void                Print                   (void) const {std::printf("x=%g t=%g xe=%g w=%g\n",x,t,xe,w);}

        /*! \brief Distance to wire

            It corresponds to the \c X coordinate in a V-plot.
        */
        float x;

        /*! \brief Drift time.

            It corresponds to the \c Y coordinate in a V-plot.
        */
        float t;

        /*! \brief Error in the measurements of distance-to-wire.

            It can be set to zero, in that case it is not used.
        */
        float xe;

        /*! \brief Point weight

            Number of points in V-plot which share the same (x,t,xe) values.
        */
        float w;
    };
    
    /*! \brief class to hold the residual plot information.
    */
    class Residual
    {
      private:

        Residual            (const Residual &);
        Residual & operator=(const Residual &);

      public:

        Residual (void) : h(NULL), fit_func(NULL),points_taken(0),points_rejected(0) {}
        ~Residual(void);
        
        void    Book    (const std::string &name,const std::string &title,unsigned int bins=100,float rmax=0.2);
        
        /*! \brief Fit the residual plot.
        */
        void Fit(const std::string &func="");
        
        const std::string   GetName(void) const;
        
        const TH1* GetHist(void) const {return h;}
              TH1* GetHist(void)       {return h;}
        
        const TF1 * GetFitFunc(void) const {return fit_func;}
              TF1 * GetFitFunc(void)       {return fit_func;}


        /*! \brief Make the report in the given pad.
        */
        void MakeReport(TVirtualPad *pad) const;
        
        /* \brief Residual plot
        */
        TH1  *h;
        
        /*! \brief Residual plot fit function
        */
        TF1 *fit_func;
        
        
        unsigned int    points_taken;
        unsigned int    points_rejected;
    };

    /*! \brief Result of fit from V-plot
    */
    class VFitResult
    {
      private:
        VFitResult(const VFitResult &);
        VFitResult & operator=(const VFitResult &);
      public:
        /*! \brief Simple constructore, set to zeros all values.
        */
        VFitResult(void) :
            channel_first(-1),
            channel_last(-1),
            pos(0),
            delta(0),
            dt(0),
            r(0),
            V_center_coridor(0),
            V_leg_max_dist(9e9),
            V_fit_max_points(1),
            t0_start(0),
            w0_start(0),
            t0_ref(0),
            success(0),
            w0(0),
            w0_err(0),
            t0(0),
            t0_err(0),
            xi2(-1),
            rt(NULL),
            hV(NULL),
            hVf(NULL),
            hT(NULL),
            gResolution(NULL),
            signal_velocity(0)
            {}
        
        virtual ~VFitResult(void);
        
        std::vector<std::string> MakeDescription(void) const;

        /*! Detector which has been used in the fit.
        */
        std::string     detector;

        /*! First channel used in the fit.
        */
        int             channel_first;

        /*! Last channel used in the fit.
        */
        int             channel_last;

        /*! Region center of the V-plot, [cm].
        */
        int             pos;

        /*! Range used for the V-plot
        */
        int             delta;
        
        /*! Cuts used in the V-plot creation.
        */
        std::string     cuts;

        /*! Maximum drift time. Zero if unknown.
        */
        float           dt;

        /*! Maximum drift distafnce. Zero if unknown.
        */
        float           r;
        
        /*! In fit do not consider points which are closer to the wire then #
        */
        float           V_center_coridor;

        /*! In a V-fit code: skip data point if distrance between RT-leg and the point is bigger then V_leg_max_dist
        */
        float           V_leg_max_dist;
        
        /*! Try to maximize number of fitted points in V-fit.
        
            if 0 - it is not used.
            Otherwise multiply the final fit Xi2 by (1+V_leg_max_dist*(M-N)/(M+N))
            where M is the total number of points in the V plot (vdata.size())
            and N is number of points accepted by a fit function.
        */
        float           V_fit_max_points;

        /*! Starting value for t0. If it is 0 - then it was not specified.
        */
        float           t0_start;

        /*! Starting value for w0 (wire position).
        */
        float           w0_start;

        /*! Reference t0 value. It is used to create uncorrected residuals plot.
            If it is 0 - then it was not specified (do not create uncorrected residuals plots then!).
        */
        float           t0_ref;

        /*! \brief Success code from MINUIT.
        */
        int             success;

        /*! \brief Wire position.
        */
        float           w0;

        /*! \brief Error on wire position.
        */
        float           w0_err;

        /*! \brief \f$T_0\f$
        */
        float           t0;

        /*! \brief Error on \f$T_0\f$
        */
        float           t0_err;

        /*!  \brief Fit \f$\chi^2\f$
        */
        float           xi2;

        /*! \brief Data points used in fit.
        */
        std::vector<VData>  vdata;

        /*! \brief RT-relation used in fit.

            If \b rt is NULL, then the fitting subroutine should calculate RT.
            Otherwise a fixed shape RT should be applied to data points.

        */
        CS::RTRelation *rt;
        
        /*! If there is a need to calculate RT, you can provide here a list of distances
            at which drift time will be calculated. Leave it empty for the default ones.
        */
        std::vector<float> rt_r;
        
        /*! \brief Name of the fit function to be used in fitting
        */
        std::string     fit_function;
        
        /*! \brief V-plot
        */
        TH2*            hV;

        /*! \brief V-plot after applying filter
        */
        TH2*            hVf;

        /*! \brief Time projection of the V-plot hV
        */
        TH1*            hT;

        /*! \brief Resolution versus distance.
        */
        TGraph*         gResolution;
        
        /*! \brief residual plots (all/left/right) uncorrected (if any)
        */
        Residual        residuals_ref[3];
        
        /*! \brief residual plots (all/left/right) corrected (if any)
        */
        Residual        residuals_corr[3];
    
        /// Comment associated with this result
        std::string     comment;

        /// Full program name used fot the fit.
        std::string     program;
        
        float           signal_velocity;
    };

    /// Constructor
                        V                       (void) {}

    /// Destructor.
    virtual             ~V                      (void) {}

    /*! \brief Create a V-plot.

        \param src        Data source. It can be path to a ROOT file.
        \param det        Straw detector name. Use official names! Example: \c ST03X1ub.
        \param ch_start   Starrting channel to be used in the V-plot creation.
        \param ch_end     Ending channel to be used in the V-plot creation.
        \param pos        Position of a region center along straw where we want to create V (0-center).
        \param delta      Region of straw used for V-plot creation: [pos-delta,pos+delta].
        \param v_hist     Histogram to be filled.
        \param v_data     Data to be used in fitting.

        If ch_start!=ch_end then several straws should be combined into a single V-plot.
    */
    virtual void        VCreate                 (const char *src,VFitResult &result)
    { throw "V::VCreate(): no code"; }

    /*! \brief Fit V data points.

        \param v_data Data for fit
        \param result Fit result
    */
    virtual void        VFit                    (VFitResult &result)
    { throw "V::VFit(): no code"; }

    /*! Save fit results.
    */
    virtual void        VStore                  (void *location, const std::string &name, const VFitResult &result);

    /// Create histogram from data points    
    virtual TH2*        MakeHistogram           (const char *name, const char *title,const std::vector<VData> &vdata,float dx, float dt);

    /// Make a report about the fit.
    virtual TCanvas*    MakeReport              (const V::VFitResult &result);

    virtual void        FillResidualPlots       (VFitResult &result,float r_limit=100,unsigned int bins=100,float r_max=0.2);

    /*! \brief Fill residual plot
        \param res_all    Residual to be filled with both left and right V legs
        \param res_left   Residual to be filled with the left V leg
        \param res_right  Residual to be filled with the right V leg
        \param vdata    V-plot data
        \param rt       RT-relation to be used (T0 will be ignored)
        \param t0       T0 to be used.
        \param w0       V-plot misalignment.
    */
    virtual void        FillResidualPlot        (const std::string &name, Residual *res_all,Residual *res_left,Residual *res_right,
                                                 const std::vector<VData> &vdata,const CS::RTRelation &rt_orig,float t0,float w0,float r_limit=100,
                                                 unsigned int bins=100,float rmax=0.2);
    
    /*! \brief Fill time difference plot
        \param ht       Empty histogram to be filled.
        \param vdata    V-plot data
        \param rt       RT-relation to be used
        \param w0       V-plot misalignment.
    */
    virtual void        FillTimeDiffPlot        (TH1 &ht,const std::vector<VData> &vdata,const CS::RTRelation &rt,float w0=0);
    
    /// Fit time difference plot
    virtual void        FitTimeDiffPlot         (TH1 &h);
    
    static bool         read_data               (std::istream &is,std::vector<V::VData> &data);
};

#endif // V_include
