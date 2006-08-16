//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 07.05.1999, AUTHOR: OLIVER KORTNER
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

///////////////////////////////////////////////////////////////////////////
// THIS FUNCTION IS THE V_plot CHI^2 FUNCTION TO BE MINIMIZED BY GEMINI. //
///////////////////////////////////////////////////////////////////////////

//////////////////////////
// INCLUDE HEADER FILES //
//////////////////////////

// C++ librairies //
#include <math.h>
//#include <cstdlib>

// mt-offline //
#include "v_plot.hxx"
#include "abs_pol.hxx"


//////////////////////
// GLOBAL VARIABLES //
//////////////////////

V_plot *v_plot;

///////////////////
// FUNCTION BODY //
///////////////////

void v_chi2(int n, double g[], double *f, const double x[], int code) {

///////////////
// VARIABLES //
///////////////

	double chi2_val; // current value of chi^2
	double fr; // value of fit function
	double t0, r0; // fit parameters
	double r_val, t_val, sigma_val; // auxiliary variables

////////////////////
// FILL VARIABLES //
////////////////////

	t0 = x[0];
	r0 = x[1];
	vector<double> alpha(v_plot->number_of_parameters()-2);
	for (int k=0; k<v_plot->number_of_parameters()-2; k++) {
		alpha[k] = x[2+k];
	}
	Abs_pol func(t0, r0, alpha);

/////////////////////
// CALCULATE chi^2 //
/////////////////////

	chi2_val = 0.0;
	for (int k=0; k<v_plot->number_of_points(); k++) {
		sigma_val = v_plot->error(k+1);
		if (sigma_val<=0) {
			continue;
		}
		r_val = v_plot->r(k+1);
		t_val = v_plot->t(k+1);
		fr = func.value(r_val);
		//chi2_val = chi2_val +
		//		(t_val-fr)*(t_val-fr)/(sigma_val*sigma_val);
		chi2_val = chi2_val +
			fabs(t_val-fr)/sigma_val;
	}

	*f=fabs(chi2_val);
	//cout << "V_CHI2  was  called !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	return;

}
