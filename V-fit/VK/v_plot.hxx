
//////////////////
// CLASS V_plot //
//////////////////


/////////////////////////////////////////////////////////////////////////
// THIS CLASS STORES THE INFORMATION REQUIRED FOR THE "v-plot" USED TO //
// DETERMINE WIRE POSITIONS.                                           //
/////////////////////////////////////////////////////////////////////////

#ifndef VplotHXX
#define VplotHXX

//////////////////////////
// INCLUDE HEADER FILES //
//////////////////////////


// standard C++ libraries //
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <math.h>

// STL //
#include <sstream>
#include <string>

// GEMINI //
#include "gemini.h"

// mt-offline //
#include "abs_pol.hxx"

using namespace std;

void v_chi2(int n, double g[], double *f, const double x[], int code);


class V_plot {

private:
// units: mm, ns
	int size; // max number of points in the v-plot
	int realsize; // number of real points in the v-plot
	double (*t_drift); // pointer to the array of drift times
	double (*r_ref); // pointer to the array of hit radii determined by
			    // the reference system
	double (*sigma); // pointer to the errors to be used in the fit to
			    // v-plot
	vector<double> fixed_parameters;
	//double *start_parameters;*/

	int nb_fit_parameters; // number of fit parameters to use, must be at least
				  // 3 (r_0, t_0, v_drift)
	double t0, r0, (*alpha); // fit parameters
	double t0_err, r0_err, (*alpha_err); // errors of the fit parameters
	Abs_pol result_pol; // final polynomial
	inline void V_plot::init(const int & nb_points); // initialization
							    // routine
	inline void V_plot::destruct(void); // destruction routine


public:
// Constructors
	V_plot(void) {
		init(50000);
		}				// default constructor
	V_plot(const int & nb_points) {
		init(nb_points);
		}				// initialize the V_plot object
						// for nb_points (r,t,sigma)-
						// points
	~V_plot(void) {
		destruct();
		}				// destructor

// Methods
// get-methods //
	inline int number_of_points(void) const;
						// get number of points in
						// v-plot
	inline int number_of_parameters(void) const;
						// get number of fit parameters
	inline double t(const int & k) const;
						// get the k-th drift time,
						// 1 <= k <= number of points
	inline double r(const int & k) const;
						// get the k-th hit radius,
						// 1 <= k <= number of points
	inline double error(const int & k) const;
						// get the error of the k-th
						// point,
						// 1 <= k <= number of points,
						// error<=0 means: discard the
						// k-th event
	inline double t_0(void) const;
						// get t_0
	inline double r_0(void) const;
						// get r_0
	inline double alpha_parameter(const int & k);
						// get the k-th alpha parameter,
						// 1 <= k <= number of fit
						//           parameters-2
	inline Abs_pol & get_rt_pol(void);
						// get the rt relation
						// polynomial
	inline double t_0_error(void) const;
						// get the error of t_0
	inline double r_0_error(void) const;
						// get the error of r_0
	inline double alpha_error(const int & k) const;
						// get the error of the k-th
						// alpha parameter,
						// 1 <= k <= number of fit
						//           parameters-2

	inline double get_mean_r(void); // get mean value of r's

        inline double get_mean_t(void); // get mean value of t's

	inline double get_min_t(void); // get minimum value of times

	inline double get_max_t(void); // get maximum value of times

        inline double get_min_r(void); // get minimum value of r

	inline double get_max_r(void); // get maximum value of r

	inline int get_realsize(void);

        inline int V_plot::get_activesize(void);

// set-methods //

	inline void set_t(const int & k, const double & t_val);
						// set the k-th drift time =
						// t_val,
						// 1 <= k <= number of points,
						// if k>number of points,
						// number of points is doubled
						// and a warning is printed
	inline void set_r(const int & k, const double & r_val);
						// set the k-th hit radius =
						// r_val,
						// 1 <= k <= number of points,
						// if k>number of points,
						// number of points is doubled
						// and a warning is printed
	inline void set_error(const int & k, const double & error_val);
						// set the error of the k-th
						// point = error_valm
						// 1 <= k <= number of points,
						// if k>number of points,
						// number of points is doubled
						// and a warning is printed,
						// error_val<=0 means: discard
						// the k-th event
	inline void remove_background(const double & kappa,
					const double & max_drift);
						// cut background, assuming
						// a kappa background rate and
						// a maximum drift time
						// max_drift,
						// warning: this routine sets
						// error of rejected entries
						// < 0
	inline void remove_background_fixed(const double & kappa,
					const double & max_drift,
					const int order, const double first_dist);
        
        inline void remove_background_V(const int order, const double first_dist);

	inline void cut(const Abs_pol & func, const double & width);
						// cut entries around func(r),
						// cut width = width
	inline void fit(const int & nb_parm);
						// perform fit with
						// (nb_parm>=3) parameters,
						// if nb_parm<3, the routine
						// throws an error message and
						// performs a fit with 3
						// parameters
	inline void fit_fixed_params(const int & nb_parm);
						// perform fit with
						// (nb_parm>=3) parameters,
						// some parameters are fixed
						// and read from a config file
						// in a later release
        inline void V_plot::fit_fixed_params_V(const int & nb_parm);
};


//////////////////////////////////////////////
// INCLUDE IMPLEMENTATION OF INLINE METHODS //
//////////////////////////////////////////////

#include "v_plot.ixx"

#endif
