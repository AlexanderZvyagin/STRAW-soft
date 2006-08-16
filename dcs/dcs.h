#ifndef dcs___include
#define dcs___include

/*! \file dcs.h
    \author Alexander Zvyagin <Alexander.Zviagine@cern.ch>
*/

#include <ctime>
#include <vector>
#include <string>
#include <map>
#include <set>

class TVirtualPad;
class TPad;
class TH1F;
class TGraph;
class TSQLServer;
class TSQLResult;
class TSQLRow;

using namespace std;

/*! \addtogroup Draw
    Everything related to a graphical output.
    @{
*/

/*! \brief Draw main TCanvas window and call dcs(TPad *many_boxes,const char *st1,const char *st2)

    - draw main window
    - create the title
    - call dcs(TPad *many_boxes,const char *st1,const char *st2)
*/
void dcs(const char *st1,const char *st2);

/*! \brief It is called when a pad is clicked with the mouse

    This function is called when a mouse is clicked upon one of the channel TPads.
*/
void dcs(int n,time_t t1,time_t t2);

/*! \brief Fill the canvas \c many_boxes with detectr+channel information.
    - Book histogram with not-perfect channels counter.
    - Get list of detectors, list of chennels.
    - Subdivide the \c many_boxes canvas according to the
      detectors list and channels number
    - Loop over all detector+channels
        - Create #ChannelStatus instance of a channel
        - Call ChannelStatus::CalculateStatus
        - fill channel's pad with a result of calculation
*/
void dcs(TPad *many_boxes,const char *st1,const char *st2);

/*! \brief Fill pad according to p1/p2/trips
    - Write channel name to the pad
    - Write p1,p2,trips
    - Fill pad with a color depening on p1/p2/trips
*/
void fill_pad(TVirtualPad *pad, const char *channel, float p1, float p2,int trips);

/*! \return List of detectors which satisfy the pattern.
    
    \param pattern It is ignored for the moment!
    \warning For the moment returns all detectors found in the DB.
*/

/*! \brief Draw graph+axis in the current TPad.
*/
void draw_graph(TGraph *g,float axis_pos,float ymax,
                int color,const char *title,const char *draw_opt);

TH1F *draw_graph(TGraph *g,time_t t1,time_t t2,float y_min,float y_max);

/*! @} */

////////////////////////////////////////////////////////////////////////////////

/*! \addtogroup DecodingEntries
    How to decode some DB entries.
    @{
*/

/*! \brief Decode a db entry into time and voltage

    It can be called after #get_measurements(TSQLServer &db,const char *detector,const char *channel,time_t t1,time_t t2,const char *table_name,std::vector<TSQLRow*> &measurements)
    \param rr db entry
    \param time of the decoded entry
    \param voltage of the decoded entry
    \return time
    \return voltage
    \exception char* if \c rr is NULL
    \warning No checks of correct time and current formats!
*/
void decode_HV_measurement(TSQLRow *rr,time_t &time,float &voltage);

/*! \brief Decode a db entry into time and temperature

    It can be called after #get_measurements(TSQLServer &db,const char *detector,const char *channel,time_t t1,time_t t2,const char *table_name,std::vector<TSQLRow*> &measurements)
    \param rr db entry
    \param time of the decoded entry
    \param temperature of the decoded entry
    \return time
    \return temperature
    \exception char* if \c rr is NULL
    \warning No checks of correct time and current formats!
*/
void decode_temperature_measurement(TSQLRow *rr,time_t &time,float &temperature);

/*! \brief Decode a db entry into time and current

    It can be called after #get_measurements(TSQLServer &db,const char *detector,const char *channel,time_t t1,time_t t2,const char *table_name,std::vector<TSQLRow*> &measurements)
    \param rr db entry
    \param time of the decoded entry
    \param current of the decoded entry
    \return time
    \return current
    \exception char* if \c rr is NULL
    \warning No checks of correct time and current formats!
*/
void decode_current_measurement(TSQLRow *rr,time_t &time,float &current);

/*! @} */

////////////////////////////////////////////////////////////////////////////////

/*! \addtogroup DatabaseCommunication
    Collection of functions which talk with a DB.
    @{
*/

/*! Get list of detectors and there channel names for a given time period.
    \param \c db Database for communication
    \param \c t1,t2 time interval
    \param \c det_chan output container with detector name and their channels.
*/
void get_detectors(TSQLServer &db,time_t t1,time_t t2,map<string,set<string> > &det_chan);

/*! \brief Get list of DB entries for given detector channel in time period [t1,t2)

    \param db Data base to communicate
    \param detector name of the detector ("ST03X1ub")
    \param channel name of a specific channel
    \param t1 beginning of the time interval
    \param t2 ending of the time interval
    \param table_name name ofthe table in the DB
    \param measurements container with all measurements
    
    \return NULL in the case of an error
*/
TSQLResult *get_measurements(TSQLServer &db,const char *detector,const char *channel,time_t t1,time_t t2,const char *table_name,std::vector<TSQLRow*> &measurements);

/*! \brief Get list of DB entries for given detector channel in time period [t1,t2)

    \param db Data base to communicate
    \param table_name name ofthe table in the DB
    \param t1 beginning of the time interval
    \param t2 ending of the time interval
    \param cuts extra parameters passed to the SQL Querry (example: "AND var=5")
    \param measurements container with all measurements
    
    \return NULL in the case of an error
*/
TSQLResult *get_measurements(TSQLServer &db,const char *table_name,
                             time_t t1,time_t t2, const char *cuts,
                             std::vector<TSQLRow*> &measurements);

/*! \brief Find and return the closest measurement for time moment t<=t1
*/
TSQLRow *get_previous_measurement(TSQLServer &db,const char *detector,const char *channel,time_t t1,const char *table_name,int time_past=500000);

/*! \return nominal HV for a given detector channel at the time 't'
    \return 0 if nominal HV is not known.
    \exception no exceptions for the moment
*/
float get_nominal_HV(TSQLServer &db,const char *detector,const char *channel,time_t t=0);

/*! \brief Write file ontent to the DB.
*/
void db_write_HV(TSQLServer *db,const char *file_name);

/*! \brief Write file ontent to the DB.
*/
void db_write_temperature(TSQLServer *db,const char *file_name);

/*! \brief Write file ontent to the DB.
*/
void db_write_current(TSQLServer *db,const char *file_name);

/*! \brief Write file ontent to the DB.
*/
void db_write_hall_pressure_temperature(TSQLServer *db,const char *file_name);

/*! @} */

////////////////////////////////////////////////////////////////////////////////

/*! \addtogroup TimeFunctions
    Collection of functions which deal with time
    @{
*/

/*! \brief Convert timestamp in format "YYYYMMDDhhmmss" to time_t type.
    \warning No checks at all! The convertion is blind on errors!
*/
time_t timestamp_to_time(const char *timestamp,bool fix=false);

/*! \brief String represenation of the interval (t1,t2)
*/
std::string interval_s(time_t t1,time_t t2,int format=0);

/*! \brief Convert time_t structure to timestamp
*/
std::string time_to_timestamp(time_t t);

/*! @} */

////////////////////////////////////////////////////////////////////////////////

#endif
