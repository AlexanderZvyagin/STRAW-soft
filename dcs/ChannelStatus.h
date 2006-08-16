#ifndef include___ChannelStatus
#define include___ChannelStatus

/*! \file ChannelStatus.h
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include <string>
#include <ctime>

class TH1F;
class TGraph;

/*! \brief Collect, analyse, display channel status information.
*/
class ChannelStatus
{
  public:
    /*! \brief Base constructor of a channel status in a time interval.

        \param det_name Detector name
        \param chan_name Channel name
        \param t1 Beginning of the time interval
        \param t2 Ending of the time interval
        \param db Database where channel measurements can be found.

        We mainly book histograms here.
    */
                        ChannelStatus           (const std::string &det_name,
                                                 const std::string &chan_name,
                                                 time_t t1,time_t t2,TSQLServer &db);

    /*! \brief Main methods which calculates channel status.
    
        Get number of trips, percentage of time on-line, off-line.
        Fills shared (among other channels) histograms all_trips
        and all_not_available as well.
    */
    void                CalculateStatus         (void);

    /*! \brief Read channel current
    */
    void                ReadCurrent             (void);

    /*! \brief Read temperature measurements of the hall.
    
        \attention The COMPASS hall temperature is used, not the channel one!
    */
    void                ReadTemperature         (void);

    /*! \brief Draw histograms

        Create a new TCanvas and plot histograms.
        
        \bug The function should not call for ReadTemperature(void) and
             ReadCurrent(void). But it does, and that is why it does
             not have the 'const' keyword.
    */
    void                DrawInOneWindow         (void);

    /*! \brief Draw histograms

        Create several TCanvas objects for every plot and plot histograms.
        
        \arg what to plot.
             - V - voltage
             - A - availability
             - T - temperature
             - I - current
        
        \bug The function should not call for ReadTemperature(void) and
             ReadCurrent(void). But it does, and that is why it does
             not have the 'const' keyword.
    */
    void                Draw                    (const char *what="VATI");
    
    /*! \brief Set option to draw temperature on Draw(void) call.
    */ 
    void                SetDrawCurrent          (bool f=true);

    /*! \brief Set option to draw current on Draw(void) call.
    */ 
    void                SetDrawTemperature      (bool f=true);
    
    /*! \return Check that channel holds the nominal HV.
    
        \sa IsOn(float hv)const;
    */
    bool                HoldNominalHV           (float hv) const {return hv>nominal_HV-8;}
    
    /*! \brief Determinate whether the channel is in the \b ON state.
    
        \attention Channel may be in the \b ON state even if it does not hold a nominal HV!
        \sa HoldNominalHV(float hv)const;
    */
    bool                IsOn                    (float hv) const {return hv>=nominal_HV-150;}
    
    /*! \brief Trip detection.
    
        To get a channel \c trip concept you have to consider that a HV channel
        can be in only two states: \b ON and \b OFF. A \c trip happens when
        channel goes from \b ON state to \b OFF one.
    
        \return \b true if we think there was a trip.
        \sa IsOn(float hv) const;
    */
    bool                IsTrip                  (float hv_old,float hv_new) const
                                                {return IsOn(hv_old) && !IsOn(hv_new);}

    /*! \brief Number of time bins used for \b h_avail histogram.
        to calculate the \b availability status for a present time bin.
    */
    static int          bins;
    
    /*! \brief Number of time bins in the past which have been used
        to calculate the \b availability status for a present time bin.
    */
    static int          bins_avail;

    std::string         detector;               ///< Detector name
    std::string         channel;                ///< Channel name
    time_t              t1;                     ///< Beginning of the time interval
    time_t              t2;                     ///< Ending of the time interval
    
    TSQLServer &        db;                     ///< The data base to communicate

    float               nominal_HV;             ///< nominal high voltage for the channel
    float               voltage_ok;             ///< percentage of time when the voltage was nominal
    float               voltage_low;            ///< percentage of time when the voltage was lower then nominal
    
    int                 trips;                  ///< Number of trips in the given period.

    TGraph *            g_Temperature;          ///< Graph with the temperature measurements
    TGraph *            g_current;              ///< Graph with the current measurements
    TGraph *            g_HV;                   ///< Graph with the high voltage measurements
    TH1F *              h_avail;                ///< Availability of the channel

    /*! Histogram with a number of tripped channels versus time
    */
    TH1F *              all_trips;

    /*! Histogram with a number of non-perfect channels versus time
    */
    TH1F *              all_not_available;
};

#endif // include___ChannelStatus
