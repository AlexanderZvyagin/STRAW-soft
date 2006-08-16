
/////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF INLINE METHOD DEFINED IN THE CLASS V_plot //
/////////////////////////////////////////////////////////////////

//*****************************************************************************

/////////////////
// METHOD init //
/////////////////


inline void V_plot::init(const int & nb_points) {
        //cout << endl << "Klausis Version of vplot !!!!!!!!!!!" << endl;
///////////////////////////////////////////
// CHECK WHETHER nb_points IS REASONABLE //
///////////////////////////////////////////

	if (nb_points<1) {
		cerr << endl << "Class V_plot, method init: "
			<< "number of points < 1 makes no sense,"
			<< endl << "will be set to 50000."
			<< endl;
		size = 50000;
	}
	else {
		size = nb_points;
	}

/////////////////////
// ALLOCATE MEMORY //
/////////////////////

	t_drift = new double[size];
	r_ref = new double[size];
	sigma = new double[size];
	alpha = new double[1];
	alpha_err = new double[1];

////////////////////
// LOCK VARIABLES //
////////////////////

	for (int k=0; k<size; k++) {
		sigma[k] = -2.0; // sigma <= 0 indicates that the k-th point
				 // sigma == -2 indicates that point has not been used ever
				 // should be neglected in the analysis
	}
	nb_fit_parameters = 0; // no fit has been performed yet

	/*for (int k=0; k<9; k++) {
		fixed_parameters[k] = 0.;
		start_parameters[k] = 0.;
	}
*/
/////////////////////////
// INITIALIZATION DONE //
/////////////////////////
	realsize = 0;
	cout << "VPLOT with " << size << " entries created" << endl;
	return;

}

//*****************************************************************************

/////////////////////
// METHOD destruct //
/////////////////////

inline void V_plot::destruct(void) {

	if (size>0) {
		delete [] t_drift;
		delete [] r_ref;
		delete [] sigma;
	}
//	if (nb_fit_parameters>2) {
		delete [] alpha;
		delete [] alpha_err;
//	}

	return;

}

//*****************************************************************************

/////////////////////////////
// METHOD number_of_points //
/////////////////////////////

inline int V_plot::number_of_points(void) const {

	return realsize;

}

//*****************************************************************************

/////////////////////////////////
// METHOD number_of_parameters //
/////////////////////////////////

inline int V_plot::number_of_parameters(void) const {

	return nb_fit_parameters;

}

//*****************************************************************************

//////////////
// METHOD t //
//////////////

inline double V_plot::t(const int & k) const {

/////////////////////////////////////////////
// CHECK WHETHER THE VALUE OF k IS ALLOWED //
/////////////////////////////////////////////

	if (k<0 || k>size) {
		cerr << endl << "Class V_plot, method t: "
			<< "illegal point number!" << endl;
		return 0.0;
	}

///////////////////////////////////////////
// IF SO, RETURN THE REQUIRED DRIFT TIME //
///////////////////////////////////////////

	return t_drift[k-1];

}

//*****************************************************************************

//////////////
// METHOD r //
//////////////

inline double V_plot::r(const int & k) const {

/////////////////////////////////////////////
// CHECK WHETHER THE VALUE OF k IS ALLOWED //
/////////////////////////////////////////////

	if (k<0 || k>size) {
		cerr << endl << "Class V_plot, method r: "
			<< "illegal point number!" << endl;
		return 0.0;
	}

///////////////////////////////////////////
// IF SO, RETURN THE REQUIRED HIT RADIUS //
///////////////////////////////////////////

	return r_ref[k-1];

}

//*****************************************************************************


//////////////////
// METHOD error //
//////////////////

inline double V_plot::error(const int & k) const {

/////////////////////////////////////////////
// CHECK WHETHER THE VALUE OF k IS ALLOWED //
/////////////////////////////////////////////

	if (k<0 || k>size) {
		cerr << endl << "Class V_plot, method error: "
			<< "illegal point number!" << endl;
		return 0.0;
	}

//////////////////////////////////////
// IF SO, RETURN THE REQUIRED ERROR //
//////////////////////////////////////

	return sigma[k-1];

}

//*****************************************************************************

////////////////
// METHOD t_0 //
////////////////

inline double V_plot::t_0(void) const {

//////////////////////////////////////////////////////////////////////
// CHECK WHETHER A FIT HAS BEEN PERFORMED BEFORE THIS FUNCTION CALL //
//////////////////////////////////////////////////////////////////////

	if (nb_fit_parameters<3) {
		cerr << endl << "Class V_plot, method t_0: "
			<< "illegal function call, perform a fit first!"
			<< endl;
		return 0.0;
	}

///////////////////////
// IF SO, RETURN t_0 //
///////////////////////

	return t0;

}

//*****************************************************************************

////////////////
// METHOD r_0 //
////////////////

inline double V_plot::r_0(void) const {

//////////////////////////////////////////////////////////////////////
// CHECK WHETHER A FIT HAS BEEN PERFORMED BEFORE THIS FUNCTION CALL //
//////////////////////////////////////////////////////////////////////

	if (nb_fit_parameters<3) {
		cerr << endl << "Class V_plot, method r_0: "
			<< "illegal function call, perform a fit first!"
			<< endl;
		return 0.0;
	}

///////////////////////
// IF SO, RETURN r_0 //
///////////////////////

	return r0;

}

//*****************************************************************************

///////////////////////
// METHOD get_rt_pol //
///////////////////////

inline Abs_pol & V_plot::get_rt_pol(void) {

///////////////////////////////
// SET POLYNOMIAL PARAMETERS //
///////////////////////////////

	result_pol.set_degree(nb_fit_parameters);
	result_pol.set_t_0(t0);
	result_pol.set_r_0(r0);
	for (int k=0; k<nb_fit_parameters-2; k++) {
		result_pol.set_coefficient(1+k, alpha[k]);
	}

///////////////////////
// RETURN POLYNOMIAL //
///////////////////////

	return result_pol;

}

//*****************************************************************************

////////////////////////////
// METHOD alpha_parameter //
////////////////////////////

inline double V_plot::alpha_parameter(const int & k) {

/////////////////////////////////////////////////////////////////
// CHECK IF A FIT HAS BEEN PERFORMED BEFORE THIS FUNCTION CALL //
/////////////////////////////////////////////////////////////////

	if (nb_fit_parameters<3) {
		cerr << endl << "Class V_plot, method alpha: "
			<< "illegal function call, perform a fit first!"
			<< endl;
		return 0.0;
	}

////////////////////////////////////////
// CHECK IF THE VALUE OF k IS ALLOWED //
////////////////////////////////////////

	if (k<1 || k>nb_fit_parameters-2) {
		cerr << endl << "Class V_plot, method alpha: "
			<< "illegal parameter number!"
			<< endl;
		return 0.0;
	}

//////////////////////////////////////////////////////////
// IF EVERYTHING IS FINE, RETURN THE REQUIRED PARAMETER //
//////////////////////////////////////////////////////////

	return alpha[k-1];

}

//*****************************************************************************

//////////////////////
// METHOD t_0_error //
//////////////////////

inline double V_plot::t_0_error(void) const {

/////////////////////////////////////////////////////////////////
// CHECK IF A FIT HAS BEEN PERFORMED BEFORE THIS FUNCTION CALL //
/////////////////////////////////////////////////////////////////

	if (nb_fit_parameters<3) {
		cerr << endl << "Class V_plot, method t_0_error: "
			<< "illegal function call, perform a fit first!"
			<< endl;
		return 0.0;
	}

//////////////////////////////////////
// IF SO, RETURN THE REQUIRED ERROR //
//////////////////////////////////////

	return t0_err;

}

//*****************************************************************************

//////////////////////
// METHOD r_0_error //
//////////////////////

inline double V_plot::r_0_error(void) const {

/////////////////////////////////////////////////////////////////
// CHECK IF A FIT HAS BEEN PERFORMED BEFORE THIS FUNCTION CALL //
/////////////////////////////////////////////////////////////////

	if (nb_fit_parameters<3) {
		cerr << endl << "Class V_plot, method r_0_error: "
			<< "illegal function call, perform a fit first!"
			<< endl;
		return 0.0;
	}

//////////////////////////////////////
// IF SO, RETURN THE REQUIRED ERROR //
//////////////////////////////////////

	return r0_err;

}

//*****************************************************************************

////////////////////////
// METHOD alpha_error //
////////////////////////

inline double V_plot::alpha_error(const int & k) const {

/////////////////////////////////////////////////////////////////
// CHECK IF A FIT HAS BEEN PERFORMED BEFORE THIS FUNCTION CALL //
/////////////////////////////////////////////////////////////////

	if (nb_fit_parameters<3) {
		cerr << endl << "Class V_plot, method alpha_error: "
			<< "illegal function call, perform a fit first!"
			<< endl;
		return 0.0;
	}

////////////////////////////////////////
// CHECK IF THE VALUE OF k IS ALLOWED //
////////////////////////////////////////

	if (k<1 || k>nb_fit_parameters-2) {
		cerr << endl << "Class V_plot, method alpha_error: "
			<< "illegal parameter number!"
			<< endl;
		return 0.0;
	}

//////////////////////////////////////////////////////
// IF EVERYTHING IS FINE, RETURN THE REQUIRED ERROR //
//////////////////////////////////////////////////////

	return alpha_err[k-1];

}

//*****************************************************************************

//////////////////
// METHOD set_t //
//////////////////

inline void V_plot::set_t(const int & k, const double & t_val) {

//////////////////////////////////////////
// CHECK IF THE POINT NUMBER IS ALLOWED //
//////////////////////////////////////////

	if (k<1) {
		cerr << endl << "Class V_plot, method set_t: "
			<< "illegal point number!"
			<< endl;
		return;
	}

	if(k>=realsize)
		realsize = k;

//////////////////////////////////
// IF k>size, REALLOCATE MEMORY //
//////////////////////////////////

	if (k>size) {
		double (*memory);
		memory = new double[size];
		for (int k=0; k<size; k++) {
			memory[k] = t_drift[k];
		}
		delete [] t_drift;
		t_drift = new double[2*size];
		for (int k=0; k<size; k++) {
			t_drift[k] = memory[k];
			memory[k] = r_ref[k];
		}
		delete [] r_ref;
		r_ref = new double[2*size];
		for (int k=0; k<size; k++) {
			r_ref[k] = memory[k];
			memory[k] = sigma[k];
		}
		delete [] sigma;
		sigma = new double[2*size];
		for (int k=0; k<size; k++) {
			sigma[k] = memory[k];
		}
		delete [] memory;
		for (int k=size; k<2*size; k++) {
			sigma[k] = -1;
		}
		size = 2*size;
	}

///////////
// SET t //
///////////

	t_drift[k-1] = t_val;

	return;

}

//*****************************************************************************

//////////////////
// METHOD set_r //
//////////////////

inline void V_plot::set_r(const int & k, const double & r_val) {

//////////////////////////////////////////
// CHECK IF THE POINT NUMBER IS ALLOWED //
//////////////////////////////////////////

	if (k<1) {
		cerr << endl << "Class V_plot, method set_r: "
			<< "illegal point number!"
			<< endl;
		return;
	}

	if(k>=realsize)
		realsize = k;

//////////////////////////////////
// IF k>size, REALLOCATE MEMORY //
//////////////////////////////////

	if (k>size) {
		double (*memory);
		memory = new double[size];
		for (int k=0; k<size; k++) {
			memory[k] = t_drift[k];
		}
		delete [] t_drift;
		t_drift = new double[2*size];
		for (int k=0; k<size; k++) {
			t_drift[k] = memory[k];
			memory[k] = r_ref[k];
		}
		delete [] r_ref;
		r_ref = new double[2*size];
		for (int k=0; k<size; k++) {
			r_ref[k] = memory[k];
			memory[k] = sigma[k];
		}
		delete [] sigma;
		sigma = new double[2*size];
		for (int k=0; k<size; k++) {
			sigma[k] = memory[k];
		}
		delete [] memory;
		for (int k=size; k<2*size; k++) {
			sigma[k] = -1;
		}
		size = 2*size;
	}

///////////
// SET r //
///////////

	r_ref[k-1] = r_val;

	return;

}

//*****************************************************************************

//////////////////////
// METHOD set_error //
//////////////////////

inline void V_plot::set_error(const int & k, const double & error_val) {

//////////////////////////////////////////
// CHECK IF THE POINT NUMBER IS ALLOWED //
//////////////////////////////////////////

	if (k<1) {
		cerr << endl << "Class V_plot, method set_error: "
			<< "illegal point number!"
			<< endl;
		return;
	}

//////////////////////////////////
// IF k>size, REALLOCATE MEMORY //
//////////////////////////////////

	if (k>size) {
		double (*memory);
		memory = new double[size];
		for (int k=0; k<size; k++) {
			memory[k] = t_drift[k];
		}
		delete [] t_drift;
		t_drift = new double[2*size];
		for (int k=0; k<size; k++) {
			t_drift[k] = memory[k];
			memory[k] = r_ref[k];
		}
		delete [] r_ref;
		r_ref = new double[2*size];
		for (int k=0; k<size; k++) {
			r_ref[k] = memory[k];
			memory[k] = sigma[k];
		}
		delete [] sigma;
		sigma = new double[2*size];
		for (int k=0; k<size; k++) {
			sigma[k] = memory[k];
		}
		delete [] memory;
		for (int k=size; k<2*size; k++) {
			sigma[k] = -1;
		}
		size = 2*size;
	}

///////////////
// SET error //
///////////////

	sigma[k-1] = error_val;

	return;

}

//*****************************************************************************

//////////////////////////////
// METHOD remove_background //
//////////////////////////////

inline void V_plot::remove_background(const double & kappa,
						const double & max_drift) {

///////////////
// VARIABLES //
///////////////

	int *td; // pointer to the drift time spectrum
	double t_min=0.0, t_max=0.0; // minimum and maxmimum drift time
	int counter; // counter
	double bin_content; // mean bin content
	double bin_size; // drift time spectrum bin size
	double bin_size2; // radius spectrum bin size
	const int nbins = 50; // number of bins
	double r_min=0.0, r_max=0.0; // minimum and maximum hit radius
	double delta_mean; // mean spatial resolution
	//double bin_volume; // bin volume
	int *rt; // pointer to binned r-t spectrum
//	int flag;

/////////////////////////////////////////
// 1ST STEP: CLEAN DRIFT TIME SPECTRUM //
/////////////////////////////////////////

// determine t_min, t_max //
	counter = 0;
	for (int k=0; k<size; k++) {
		if (sigma[k]>0.0 && counter == 0) {
			counter = counter+1;
			t_min = t_drift[k];
			t_max = t_drift[k];
		}
		else if (sigma[k]>0.0) {
			counter = counter+1;
			if (t_drift[k]<t_min) {
				t_min = t_drift[k];
			}
			if (t_drift[k]>t_max) {
				t_max = t_drift[k];
			}
		}
	}

// determine bin size //
	bin_size = (t_max-t_min)/(nbins-1);
	cout << "Bin size : " << bin_size << "  after " << counter << " counters" << endl;

//calculate mean bin_content
	bin_content = counter / nbins;
	cout << "mean bin content : " << bin_content << endl;

// clear bins //
	td = new int[nbins];
	for (int k=0; k<nbins; k++) {
		td[k] = 0;
	}

// fill bins //
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		int l = static_cast<int>(((t_drift[k]-t_min)/(t_max-t_min))*(nbins-1));
		//cout << "l   :  " << l << endl;
		td[l] = td[l]+1;
	}

// cut entries in background bins //
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		int l = static_cast<int>(((t_drift[k]-t_min)/(t_max-t_min))*nbins);
		if (td[l]<max_drift*bin_content) {
			sigma[k] = -1.0;
		}
	}

	delete [] td;

//////////////////////////////////////////////////////
// CLEAN r-t RELATION FROM BACKGROUND -crude method //
//////////////////////////////////////////////////////

// determine r_min, r_max, t_min, t_max, delta_mean //
	counter = 0;
	delta_mean = 0.0;
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		delta_mean = delta_mean+sigma[k];
		if (counter == 0) {
			counter = counter+1;
			r_min = r_ref[k]; r_max = r_ref[k];
			t_min = t_drift[k]; t_max = t_drift[k];
			continue;
		}
		else{
			counter = counter+1;
			if (r_ref[k] < r_min) {
				r_min = r_ref[k];
			}
			if (r_ref[k] > r_max) {
				r_max = r_ref[k];
			}
			if (t_drift[k] < t_min) {
				t_min = t_drift[k];
			}
			if (t_drift[k] > t_max) {
				t_max = t_drift[k];
			}
		}
	}
	delta_mean = delta_mean/static_cast<double>(counter);

// determine bin-sizes //

	bin_size = (t_max-t_min)/static_cast<double>(nbins-1);
	bin_size2 = (r_max-r_min)/static_cast<double>(nbins-1);
	rt = new int[nbins*nbins];

	cout << "Bin size : " << bin_size << "  after " << counter << " counters" << endl;
//calculate mean bin_content
	bin_content = counter /(nbins*nbins);
	cout << "mean bin content (2D): " << bin_content << endl;
	cout << "cut everything less than "  << kappa*bin_content << endl;
// clear bins //
	for (int k=0; k<nbins*nbins; k++) {
		rt[k] = 0;
	}

// fill bins //
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		int l1 = static_cast<int>((t_drift[k]-t_min)/bin_size);
		int l2 = static_cast<int>((r_ref[k]-r_min)/bin_size2);
		rt[l2+l1*nbins] = rt[l2+l1*nbins]+1;
	}


	counter = 0;
// reject entries in background bins //
	//ofstream outrt("rt.tmp");
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		int l1 = static_cast<int>((t_drift[k]-t_min)/bin_size);
		int l2 = static_cast<int>((r_ref[k]-r_min)/bin_size2);
		if (rt[l2+l1*nbins]<kappa*bin_content || rt[l2+l1*nbins]<2) {
			sigma[k] = -1.0;
		}
		else {
			counter ++;
		}
	}

	cout << "Points after 2D cut : " << counter << endl;
//////////////////////////////////////
// BACKGROUND SUBTRACTION PERFORMED //
//////////////////////////////////////
delete [] rt;

////////////////////////////////////////////////////
//  do fit with pol(4) for finetuning of background
////////////////////////////////////////////////////

this->fit(3);
double func_val;

// now select only points +-5ns from preliminary 'V'
for (int k=0; k<realsize; k++) {
	func_val = this->t_0() + this->alpha_parameter(1)*fabs(r_ref[k]-this->r_0());
	//func_val = func_val + this->alpha_parameter(2)*(fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0()));
	//func_val = func_val + this->alpha_parameter(3)*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0());
	if(fabs(t_drift[k]-func_val) < 10. && fabs(r_ref[k]-this->r_0()) < 3.3 && sigma[k] != -2.0)
		sigma[k] = 0.1;
	else
		sigma[k] = -1.0;

}

this->fit(5);
for (int k=0; k<realsize; k++) {
	func_val = this->t_0() + this->alpha_parameter(1)*fabs(r_ref[k]-this->r_0());
	func_val = func_val + this->alpha_parameter(2)*(fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0()));
	func_val = func_val + this->alpha_parameter(3)*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0());
	if(fabs(t_drift[k]-func_val) < 10. && fabs(r_ref[k]-this->r_0()) < 3.3 && sigma[k] != -2.0)
		sigma[k] = 0.1;
	else
		sigma[k] = -1.0;

}



cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
cout << this->t_0() << "   " << this->r_0()  << "  " << this->alpha_parameter(1) << endl;
cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;


	return;

}

//*****************************************************************************

////////////////
// METHOD cut //
////////////////

inline void V_plot::cut(const Abs_pol & func, const double & width) {

/////////////////////////
// AUXILIARY VARIABLES //
/////////////////////////

	double r1, t1, m;
	ofstream outr("r.tmp");
	ofstream outt("t.tmp");

/////////
// CUT //
/////////

//	r1 = func.r_0()+12.0;
	r1 = func.r_0();
	t1 = func.value(r1);
	m = func.first_derivative(r1);
	for (int k=0; k<size; k++) {
		if (sigma[k]>0) {
// 			if (t_drift[k]<t1) {
// 			if (fabs(t_drift[k]-func.value(r_ref[k]))>width) {
// 				sigma[k] = -1.0;
// 			}
// 			}
// 			else {
// 			if (fabs(t_drift[k]-t1-m*(fabs(r_ref[k])-r1))>width){
// 				sigma[k] = -1.0;
// 			}
// 			}
			if (fabs(t_drift[k]-func.value(r_ref[k]))>width) {
				sigma[k] = -1.0;
			}
		}
		if (sigma[k]>0) {
			outt << t_drift[k] << endl;
			outr << r_ref[k] << endl;
		}
	}

	return;

}

//*****************************************************************************

////////////////
// METHOD fit //
////////////////

inline void V_plot::fit(const int & nb_parm) {

///////////////////////////////////////////////
// CHECK IF THE NUMBER OF PARAMETERS IS OKAY //
///////////////////////////////////////////////

	if (nb_parm<3) {
		cerr << endl << "Class V_plot, method fit: "
			<< "illegal number of parameters, will perform a fit "
			<< "with 3 parameters." << endl;
		nb_fit_parameters = 3;
	}
	else {
		nb_fit_parameters = nb_parm;
	}

///////////////
// VARIABLES //
///////////////

	delete [] alpha;
	delete [] alpha_err;
	alpha = new double[nb_parm-2];
	alpha_err = new double[nb_parm-2];
	extern V_plot *v_plot;
	GEmini *ptr; // general pointer to the minimization object
	OBJfun f(v_chi2); // capture the objective function into OBJfun object f
	double chi2_val; // chi^2 after minimization
	double t_min=0.0, r_min=0.0, r_max=0.0; // auxiliary variables
	double rmean = 0.;
	int counter =0;
///////////////////////////
// SELECT THE FIT ENGINE //
///////////////////////////

	ptr = new CMinuit(nb_fit_parameters ,&f);

// for performance gain switch off printout and warnings //
//	ptr->setPrintLevel(-1);
//	ptr->warningsOFF();
	ptr->setPrecision(1e-9); // set machine precision
	{double strategy=1;
	((CMinuit *)ptr)->command("SET STRATEGY",&strategy,2);
	}

///////////////////////////
// DETERMINE STARTVALUES //
///////////////////////////

	for (int k=0; k<size; k++) {
		if (sigma[k]>0 && counter == 0) {
			t_min = t_drift[k];
			r_min = r_ref[k];
			r_max = r_ref[k];
			rmean = rmean + r_ref[k];
			counter++;
		}
		else if (sigma[k]>0) {
			rmean = rmean + r_ref[k];
			counter++;
			if (t_drift[k]<t_min) {
				t_min = t_drift[k];
			}
			if (r_ref[k]<r_min) {
				r_min = r_ref[k];
			}
			if (r_ref[k]>r_max) {
				r_max = r_ref[k];
			}
		}
	}
	rmean = rmean / counter;
///////////////////////////
// INITIALIZE FIT ENGINE //
///////////////////////////

	ptr->parmDef(1, "t_0", 9.0, 1.0, MINF, INF);
	ptr->parmDef(2, "r_0", rmean, 1.0, MINF, INF);
	ptr->parmDef(3, NULL, 9., 1.0, MINF, INF);
	for (int k=4; k<=nb_fit_parameters; k++) {
		if(k==4)
			ptr->parmDef(k, NULL, 3.0, 1.0, MINF, INF);
		else
			ptr->parmDef(k, NULL, 0.0, 1.0, MINF, INF);
	}

	cout << "FFFIITTT ENNGINE INITIALIUZED  !!! " << endl;
//////////////////
// MINIMIZATION //
//////////////////

	v_plot = this;
	ptr->minimize();

//////////////////////
// SAVE FIT RESULTS //
//////////////////////

	ptr->getResult(1, t0, t0_err, chi2_val);
	ptr->getResult(2, r0, r0_err, chi2_val);
	for (int k=3; k<=nb_fit_parameters; k++) {
		ptr->getResult(k, alpha[k-3], alpha_err[k-3], chi2_val);
	}

/////////////////////////
// END OF MINIMIZATION //
/////////////////////////

	return;

}





///////////////////////////////////////////////
///////////////////////////////////////////////
inline void V_plot::fit_fixed_params(const int & nb_parm){
///////////////////////////////////////////////
// CHECK IF THE NUMBER OF PARAMETERS IS OKAY //
///////////////////////////////////////////////
	double fixed_params[9];
	double start_params[9];
	// set start params to 0

	for (int k=0; k<9; k++){
	fixed_params[k]=0.;
	start_params[k]=0.;
	}

	fixed_params[2] = 9.18292;  /// ST3X1u
	fixed_params[3] = 2.5395;
	fixed_params[4] = -0.807645;
	fixed_params[5] = -0.282376;
	fixed_params[6] = 0.0341231;
	fixed_params[7] = 0.0528839;
	fixed_params[8] = -0.0088662;

//  	fixed_params[2] = 7.9831;  // ST3X1d
//  	fixed_params[3] = 2.49154;
//  	fixed_params[4] = -0.271995;
//  	fixed_params[5] = -0.18066;
//  	fixed_params[6] = -0.0432458;
//  	fixed_params[7] = 0.0187878;
//  	fixed_params[8] = 0.00167301;

//  	fixed_params[2] = 10.4754;  // ST3U1u
//  	fixed_params[3] = 2.01537;
//  	fixed_params[4] = -0.870699;
//  	fixed_params[5] = -0.229019;
//  	fixed_params[6] = -0.0102257;
//  	fixed_params[7] = 0.0673777;
//  	fixed_params[8] = -0.00852331;


// 	fixed_params[2] = 8.43464;  /// ST3X2d
// 	fixed_params[3] = 2.38662;
// 	fixed_params[4] = -0.398968;
// 	fixed_params[5] = -0.178028;
// 	fixed_params[6] = -0.0590659;
// 	fixed_params[7] = 0.00539522;
// 	fixed_params[8] = 0.00968198;

// 	fixed_params[2] = 9.33727;  /// ST3X2u
// 	fixed_params[3] = 2.08406;
// 	fixed_params[4] = -0.735512;
// 	fixed_params[5] = -0.161533;
// 	fixed_params[6] = 0.00750863;
// 	fixed_params[7] = 0.0310696;
// 	fixed_params[8] = -0.00497869;

	if (nb_parm<3) {
		cerr << endl << "Class V_plot, method fit: "
			<< "illegal number of parameters, will perform a fit "
			<< "with 3 parameters." << endl;
		nb_fit_parameters = 3;
	}
	else {
		nb_fit_parameters = nb_parm;
	}

///////////////
// VARIABLES //
///////////////

	delete [] alpha;
	delete [] alpha_err;
	alpha = new double[nb_parm-2];
	alpha_err = new double[nb_parm-2];
	extern V_plot *v_plot;
	GEmini *ptr; // general pointer to the minimization object
	OBJfun f(v_chi2); // capture the objective function into OBJfun object f
	double chi2_val; // chi^2 after minimization
	double t_min=0.0, r_min=0.0, r_max=0.0; // auxiliary variables
	double rmean = 0.;
	int counter =0;
///////////////////////////
// SELECT THE FIT ENGINE //
///////////////////////////

	ptr = new CMinuit(nb_fit_parameters ,&f);

// for performance gain switch off printout and warnings //
//	ptr->setPrintLevel(-1);
//	ptr->warningsOFF();
	ptr->setPrecision(1e-9); // set machine precision
	{double strategy=1;
	((CMinuit *)ptr)->command("SET STRATEGY",&strategy,2);
	}

///////////////////////////
// DETERMINE STARTVALUES //
///////////////////////////

	for (int k=0; k<size; k++) {
		if (sigma[k]>0 && counter == 0) {
			t_min = t_drift[k];
			r_min = r_ref[k];
			r_max = r_ref[k];
			rmean = rmean + r_ref[k];
			counter++;
		}
		else if (sigma[k]>0) {
			rmean = rmean + r_ref[k];
			counter++;
			if (t_drift[k]<t_min) {
				t_min = t_drift[k];
			}
			if (r_ref[k]<r_min) {
				r_min = r_ref[k];
			}
			if (r_ref[k]>r_max) {
				r_max = r_ref[k];
			}
		}
	}
	rmean = rmean / counter;

///////////////////////////
// INITIALIZE FIT ENGINE //
///////////////////////////

	ptr->parmDef(1, "t_0", 9., 1.0, MINF, INF);
	ptr->parmDef(2, "r_0", rmean, 1.0, MINF, INF);

	for (int k=3; k<=nb_fit_parameters; k++) {
		if(k<10 && fixed_params[k-1] != 0.)
			ptr->parmDef(k, NULL, fixed_params[k-1], 1.0, fixed_params[k-1], fixed_params[k-1]);
		else if(k<10 && start_params[k-1] != 0.)
			ptr->parmDef(k, NULL, start_params[k-1], 1.0, MINF, INF);
		else
			ptr->parmDef(k, NULL, 0.0, 1.0, MINF, INF);
	}

	/*for (int k=3; k<=nb_fit_parameters; k++) {
		ptr->parmDef(k, NULL, 0.0, 1.0, MINF, INF);
	}
*/
	cout << "FFFIITTT ENNGINE INITIALIUZED  !!! " << endl;
//////////////////
// MINIMIZATION //
//////////////////

	v_plot = this;
	ptr->minimize();

//////////////////////
// SAVE FIT RESULTS //
//////////////////////

	ptr->getResult(1, t0, t0_err, chi2_val);
	ptr->getResult(2, r0, r0_err, chi2_val);
	for (int k=3; k<=nb_fit_parameters; k++) {
		ptr->getResult(k, alpha[k-3], alpha_err[k-3], chi2_val);
	}

/////////////////////////
// END OF MINIMIZATION //
/////////////////////////

	return;

}
























////////////////////////////////////////////////////////////////////////////////////
///// REMOVE BACKGROUND FIXED //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

inline void V_plot::remove_background_fixed(const double & kappa,
					const double & max_drift, const int order, const double first_dist){

///////////////
// VARIABLES //
///////////////

	//int *td; // pointer to the drift time spectrum
	double t_min=0.0, t_max=0.0; // minimum and maxmimum drift time
	int counter; // counter
	double counter_dbl;
	double bin_content; // mean bin content
	double bin_size; // drift time spectrum bin size
	double bin_size2; // radius spectrum bin size
	const int nbins = 20; // number of bins
	double r_min=0.0, r_max=0.0; // minimum and maximum hit radius
	double delta_mean; // mean spatial resolution
	double r_mean; // bin volume
	int *rt; // pointer to binned r-t spectrum
//	int flag;


	cout << " VPLOT  SIZE   : " << size << endl;
	cout << " mean : " << this->get_mean_r() << endl;

	// kill entries with to high drift times
	for (int k=0; k<size; k++) {
		if (fabs(t_drift[k]) <= max_drift) {
			continue;
		}
		else{
		sigma[k] = -2.;
		}
	}

	// kill entries too far away from mean
	r_mean = this->get_mean_r();
	for (int k=0; k<size; k++) {
		if (fabs(r_ref[k] - r_mean) <= 20.) {
			continue;
		}
		else{
		sigma[k] = -2.;
		}
	}

// determine r_min, r_max, t_min, t_max, delta_mean //
	counter = 0;
	delta_mean = 0.0;
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		delta_mean = delta_mean+sigma[k];
		if (counter == 0) {
			counter = counter+1;
			r_min = r_ref[k]; r_max = r_ref[k];
			t_min = t_drift[k]; t_max = t_drift[k];
			continue;
		}
		else{
			counter = counter+1;
			if (r_ref[k] < r_min) {
				r_min = r_ref[k];
			}
			if (r_ref[k] > r_max) {
				r_max = r_ref[k];
			}
			if (t_drift[k] < t_min) {
				t_min = t_drift[k];
			}
			if (t_drift[k] > t_max) {
				t_max = t_drift[k];
			}
		}
	}
	delta_mean = delta_mean/static_cast<double>(counter);
	cout << "!!!! Entries AFTER first step : " << counter << endl;
	counter_dbl = counter;
	counter_dbl = counter_dbl/(nbins*nbins);
  	cout << "!!!!!!!!!" << counter_dbl << endl;
// determine bin-sizes //

	bin_size = (t_max-t_min)/static_cast<double>(nbins-1);
	bin_size2 = (r_max-r_min)/static_cast<double>(nbins-1);
	rt = new int[nbins*nbins];

	cout << "Bin size : " << bin_size << "  after " << counter << " counters" << endl;
	//calculate mean bin_content
	bin_content = counter /(nbins*nbins);
	cout << "R_max, R_min : " << r_max << "  ," << r_min << endl;
	cout << "T_max, T_min : " << t_max << "  ," << t_min << endl;
	cout << "mean bin content (2D): " << counter_dbl << endl;
	cout << "cut everything less than "  << kappa*counter_dbl << endl;
	cout << "mean r  " << this->get_mean_r() << endl;
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

	// clear bins //
	for (int k=0; k<nbins*nbins; k++) {
		rt[k] = 0;
	}

	// fill bins //
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		int l1 = static_cast<int>((t_drift[k]-t_min)/bin_size);
		int l2 = static_cast<int>((r_ref[k]-r_min)/bin_size2);
		rt[l2+l1*nbins] = rt[l2+l1*nbins]+1;
	}


	counter = 0;
	// reject entries in background bins //
	//ofstream outrt("rt.tmp");
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		int l1 = static_cast<int>((t_drift[k]-t_min)/bin_size);
		int l2 = static_cast<int>((r_ref[k]-r_min)/bin_size2);

		//if (rt[l2+l1*nbins]<(kappa/3)*bin_content || rt[l2+l1*nbins]<2)
		//	sigma[k] = -2.0;
		if (rt[l2+l1*nbins]<kappa*counter_dbl)
			sigma[k] = -1.0;
		else{
			sigma[k] = .1;
			counter ++;
		}
	}

	cout << "Points after 2D cut : " << counter << endl;
	cout << "mean r  " << this->get_mean_r() << endl;
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//////////////////////////////////////
// BACKGROUND SUBTRACTION PERFORMED //
//////////////////////////////////////
delete [] rt;
cout << "RT should be deleted !!    !!!!!!!!!!!new!!!!!!!!!!!" << endl;
////////////////////////////////////////////////////
//  do fit with pol(3) for finetuning of background
////////////////////////////////////////////////////

if(order>=3){
	counter = 0;
	cout << " ENHANCED BACKGROUND SUBTRACTION entered " << endl;
	//mask everything far away from mean to get t0 down !!!


	//this->fit_fixed_params(3);
	this->fit_fixed_params(3);
	double func_val;
	if(fabs(this->r_0()-this->get_mean_r())<1.5 && fabs(this->t_0()) < 20.){ /// make sure that params do make sense
		// now select only points +-10ns from preliminary 'V'
		for (int k=0; k<size; k++) {
			func_val = this->t_0() + this->alpha_parameter(1)*fabs(r_ref[k]-this->r_0());
			//func_val = func_val + this->alpha_parameter(2)*(fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0()));
			//func_val = func_val + this->alpha_parameter(3)*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0());
			if((fabs(t_drift[k]-func_val) < 10.) && (fabs(r_ref[k]-this->r_0()) < first_dist) && (sigma[k] != -2.0)){
				sigma[k] = 0.1;
				counter++;
			}
			else
				sigma[k] = -1.0;
		}

		if(order >= 5){
			counter = 0;
			this->fit_fixed_params(5);
			for (int k=0; k<size; k++) {
				func_val = this->t_0() + this->alpha_parameter(1)*fabs(r_ref[k]-this->r_0());
				func_val = func_val + this->alpha_parameter(2)*(fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0()));
				func_val = func_val + this->alpha_parameter(3)*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0());
				if(fabs(t_drift[k]-func_val) < 8. && fabs(r_ref[k]-this->r_0()) < first_dist && sigma[k] != -2.0){
					sigma[k] = 0.1;
					counter++;
				}
				else
					sigma[k] = -1.0;
			}
		}
	}
	else
		cout << " ENHANCED BACKGROUND SUBTRACTION FAILED !!!!!! " << endl;

}


cout << "!!!!!!!!!!!!!! ENHANCED BACKGROUND SUBTRACTION : " << counter << "!!!!!!!!!!!!!!!" << endl;
cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
cout << this->t_0() << "   " << this->r_0()  << "  " << this->alpha_parameter(1) << endl;
cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;


	return;

}




























////////////////////////////////////////////////////////////////////////////////////
///// REMOVE BACKGROUND for Sasha (new) ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

inline void V_plot::remove_background_V(const int order, const double fit_range = 3.0){

///////////////
// VARIABLES //
///////////////

	const double first_step_keep = 5.; // calculated Nr. of points kept in the first step
	double t_min=0.0, t_max=0.0; // minimum and maxmimum drift time
	int counter; // counter
	double counter_dbl;
	double bin_content; // mean bin content
	double bin_size; // drift time spectrum bin size
	double bin_size2; // radius spectrum bin size
	const int nbins = 20; // number of bins
	double r_min=0.0, r_max=0.0; // minimum and maximum hit radius
	
	double r_mean; // bin volume
	double t_mean = 0.;
	int *rt; // pointer to binned r-t spectrum
//	int flag;

	
	cout << " VPLOT  SIZE   : " << size << endl;
	r_mean = this->get_mean_r();
	cout << " mean : " << this->get_mean_r() << endl;
	t_mean = this->get_mean_t();
	cout << " mean time : " << t_mean << endl;
	cout << "Active points : " << this->get_activesize() << endl;

	// kill entries with high drift times too far from mean
	for (int k=0; k<size; k++) {
		if (fabs(t_drift[k]-t_mean) <= 50.) {
			continue;
		}
		else{
		sigma[k] = -2.;
		}
	}
	cout << "Active points : " << this->get_activesize() << endl;
       //  
// 	// kill entries too far away from mean
// 	r_mean = this->get_mean_r();
// 	cout << "RMEAN :  " << r_mean << endl;
// 	for (int k=0; k<size; k++) {
// 		if (fabs(r_ref[k] - r_mean) <= 20.) {
// 			continue;
// 		}
// 		else{
// 		sigma[k] = -2.;
// 		}
// 	}
// 	 cout << "AFTER X cut : " << this->get_activesize() << endl;

// determine r_min, r_max, t_min, t_max, delta_mean //
	t_min = this->get_min_t();
	t_max = this->get_max_t();
	r_min = this->get_min_r();
	r_max = this->get_max_r();
	
	counter_dbl = this->get_activesize();
	counter_dbl = counter_dbl/(nbins*nbins);
  
// determine bin-sizes //
	bin_size = (t_max-t_min)/static_cast<double>(nbins-1);
	bin_size2 = (r_max-r_min)/static_cast<double>(nbins-1);
	rt = new int[nbins*nbins];


	//calculate mean bin_content
	bin_content = counter /(nbins*nbins);
	// cout << "R_max, R_min : " << r_max << "  ," << r_min << endl;
// 	cout << "T_max, T_min : " << t_max << "  ," << t_min << endl;
// 	cout << "mean bin content (2D): " << counter_dbl << endl;
// 	cout << "cut everything less than "  << first_step_keep*counter_dbl << endl;
// 	cout << "mean r  " << this->get_mean_r() << endl;
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

	// clear bins //
	for (int k=0; k<nbins*nbins; k++) {
		rt[k] = 0;
	}

	// fill bins //
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		int l1 = static_cast<int>((t_drift[k]-t_min)/bin_size);
		int l2 = static_cast<int>((r_ref[k]-r_min)/bin_size2);
		rt[l2+l1*nbins] = rt[l2+l1*nbins]+1;
	}
	

	// reject entries in background bins //
	//ofstream outrt("rt.tmp");
	for (int k=0; k<size; k++) {
		if (sigma[k]<=0.0) {
			continue;
		}
		int l1 = static_cast<int>((t_drift[k]-t_min)/bin_size);
		int l2 = static_cast<int>((r_ref[k]-r_min)/bin_size2);

		//if (rt[l2+l1*nbins]<(kappa/3)*bin_content || rt[l2+l1*nbins]<2)
		//	sigma[k] = -2.0;
		if (rt[l2+l1*nbins]<first_step_keep*counter_dbl)
			sigma[k] = -1.0;
		else{
			sigma[k] = .1;
		}
	}

	cout << "Points after 2D cut : " << this->get_activesize() << endl;
	cout << "mean r  " << this->get_mean_r() << endl;
	cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

////////////////////////////////////////////////////
// first step of BACKGROUND SUBTRACTION PERFORMED //
////////////////////////////////////////////////////



delete [] rt;
//cout << "RT should be deleted !!    !!!!!!!!!!!new!!!!!!!!!!!" << endl;
////////////////////////////////////////////////////
//  do fit with pol(3) for finetuning of background
////////////////////////////////////////////////////

if(order>=3){
	//cout << " ENHANCED BACKGROUND SUBTRACTION entered " << endl;
	//mask everything far away from mean to get t0 down !!!


	this->fit_fixed_params_V(3);
	double func_val;
	if(fabs(this->r_0()-this->get_mean_r())< 5. && this->t_0()<this->get_max_t() &&
	this->t_0()>this->get_min_t()){ /// make sure that params do make sense
		// now select only points +-15ns from preliminary 'V'
		for (int k=0; k<size; k++) {
			func_val = this->t_0() + this->alpha_parameter(1)*fabs(r_ref[k]-this->r_0());
			//func_val = func_val + this->alpha_parameter(2)*(fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0()));
			//func_val = func_val + this->alpha_parameter(3)*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0());
			if((fabs(t_drift[k]-func_val) < 10.) && sigma[k] != -2.0){
				sigma[k] = 0.1;
			}
			else
				sigma[k] = -1.0;
		}
		//cout << "After 3 Pol backgrounf substr : " << this->get_activesize()  << endl;
		if(order >= 5){
			this->fit_fixed_params_V(5);
			for (int k=0; k<size; k++) {
				func_val = this->t_0() + this->alpha_parameter(1)*fabs(r_ref[k]-this->r_0());
				func_val = func_val + this->alpha_parameter(2)*(fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0()));
				func_val = func_val + this->alpha_parameter(3)*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0())*fabs(r_ref[k]-this->r_0());
				if(fabs(t_drift[k]-func_val) < 8. && sigma[k] != -2.0){
					sigma[k] = 0.1;
				}
				else
					sigma[k] = -1.0;
			}
			//cout << "After 5 Pol backgrounf substr : " << this->get_activesize() << endl;
		}
	}
	else
		cout << " ENHANCED BACKGROUND SUBTRACTION FAILED !!!!!! " << endl;
}



//cout << "!!!!!!!!!!!!!! ENHANCED BACKGROUND SUBTRACTION ended: " << "!!!!!!!!!!!!!!!" << endl;
cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
cout << "Fitted t0 : " << this->t_0() << endl; 
cout << "Fitted r0 : " << this->r_0() << endl;
cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

	return;
	
	//now apply the cut on the fit range
	for (int k=0; k<size; k++) {
		if(fabs(this->r(0)-r_ref[k])>fit_range)
			sigma[k] = -1.0;	
	}
	
}


///////////////////////////////////////////////
///////////////////////////////////////////////
inline void V_plot::fit_fixed_params_V(const int & nb_parm){
///////////////////////////////////////////////
// CHECK IF THE NUMBER OF PARAMETERS IS OKAY //
///////////////////////////////////////////////
	double fixed_params[9];
	double start_params[9];
	// set start params to 0

	for (int k=0; k<9; k++){
	fixed_params[k]=0.;
	start_params[k]=0.;
	}

	fixed_params[2] = 9.18292;  /// ST3X1u
	fixed_params[3] = 2.5395;
	fixed_params[4] = -0.807645;
	fixed_params[5] = -0.282376;
	fixed_params[6] = 0.0341231;
	fixed_params[7] = 0.0528839;
	fixed_params[8] = -0.0088662;

	if (nb_parm<3) {
		cerr << endl << "Class V_plot, method fit: "
			<< "illegal number of parameters, will perform a fit "
			<< "with 3 parameters." << endl;
		nb_fit_parameters = 3;
	}
	else {
		nb_fit_parameters = nb_parm;
	}

///////////////
// VARIABLES //
///////////////

	delete [] alpha;
	delete [] alpha_err;
	alpha = new double[nb_parm-2];
	alpha_err = new double[nb_parm-2];
	extern V_plot *v_plot;
	GEmini *ptr; // general pointer to the minimization object
	OBJfun f(v_chi2); // capture the objective function into OBJfun object f
	double chi2_val; // chi^2 after minimization
	double t_min=0.0, r_min=0.0, r_max=0.0; // auxiliary variables
	double rmean = 0.;
///////////////////////////
// SELECT THE FIT ENGINE //
///////////////////////////

	ptr = new CMinuit(nb_fit_parameters ,&f);

// for performance gain switch off printout and warnings //
	ptr->setPrintLevel(-1);
	ptr->warningsOFF();
	ptr->setPrecision(1e-11); // set machine precision
	{double strategy=1;
	((CMinuit *)ptr)->command("SET STRATEGY",&strategy,2);
	}

///////////////////////////
// DETERMINE STARTVALUES //
///////////////////////////

	t_min = this->get_min_t();
	r_min = this->get_min_r();
	r_max = this->get_max_r();
	rmean = this->get_mean_r();

///////////////////////////
// INITIALIZE FIT ENGINE //
///////////////////////////

	ptr->parmDef(1, "t_0", t_min, 1.0, MINF, INF);
	ptr->parmDef(2, "r_0", rmean, 1.0, MINF, INF);

	for (int k=3; k<=nb_fit_parameters; k++) {
		if(k<10 && fixed_params[k-1] != 0.)
			ptr->parmDef(k, NULL, fixed_params[k-1], 1.0, fixed_params[k-1], fixed_params[k-1]);
		else if(k<10 && start_params[k-1] != 0.)
			ptr->parmDef(k, NULL, start_params[k-1], 1.0, MINF, INF);
		else
			ptr->parmDef(k, NULL, 0.0, 1.0, MINF, INF);
	}

	/*for (int k=3; k<=nb_fit_parameters; k++) {
		ptr->parmDef(k, NULL, 0.0, 1.0, MINF, INF);
	}
*/
	//cout << "FFFIITTT ENNGINE INITIALIZED  !!! " << endl;
//////////////////
// MINIMIZATION //
//////////////////

	v_plot = this;
	ptr->minimize();

//////////////////////
// SAVE FIT RESULTS //
//////////////////////

	ptr->getResult(1, t0, t0_err, chi2_val);
	ptr->getResult(2, r0, r0_err, chi2_val);
	for (int k=3; k<=nb_fit_parameters; k++) {
		ptr->getResult(k, alpha[k-3], alpha_err[k-3], chi2_val);
	}

/////////////////////////
// END OF MINIMIZATION //
/////////////////////////

	return;

}











/////////////////////////////////////////////////////////////////////////////////////////////////////
// helper routines
/////////////////////////////////////////////////////////////////////////////////////////////////////


inline double V_plot::get_min_t(void){ // get minimum value of times

int counter = 0;
double t_min = 0.;

for (int k=0; k<size; k++) {
	if (sigma[k]>0 && counter == 0) {
		t_min = t_drift[k];
		counter++;
	}
	else if (sigma[k]>0) {
		counter++;
		if (t_drift[k]<t_min) {
			t_min = t_drift[k];
		}
	}
}
return t_min;
}


inline double V_plot::get_max_t(void){ // get maximum value of times

int counter = 0;
double t_max = 0.;

for (int k=0; k<size; k++) {
	if (sigma[k]>0 && counter == 0) {
		t_max = t_drift[k];
		counter++;
	}
	else if (sigma[k]>0) {
		counter++;
		if (t_drift[k]>t_max) {
			t_max = t_drift[k];
		}
	}
}
return t_max;
}

inline double V_plot::get_min_r(void){ // get minimum value of r

int counter = 0;
double r_min = 0.;

for (int k=0; k<size; k++) {
	if (sigma[k]>0 && counter == 0) {
		r_min = r_ref[k];
		counter++;
	}
	else if (sigma[k]>0) {
		counter++;
		if (r_ref[k]<r_min) {
			r_min = r_ref[k];
		}
	}
}
return r_min;
}


inline double V_plot::get_max_r(void){ // get maximum value of times

int counter = 0;
double r_max = 0.;

for (int k=0; k<size; k++) {
	if (sigma[k]>0 && counter == 0) {
		r_max = r_ref[k];
		counter++;
	}
	else if (sigma[k]>0) {
		counter++;
		if (r_ref[k]>r_max) {
			r_max = r_ref[k];
		}
	}
}
return r_max;
}

inline double V_plot::get_mean_r(void){

double rmean = 0;
int counter = 0;

for (int k=0; k<size; k++) {
		if (sigma[k]>0) {
			rmean = rmean + r_ref[k];
			counter++;
		}
}

rmean = rmean / counter;
return rmean;

}

inline double V_plot::get_mean_t(void){

double tmean = 0;
int counter = 0;

for (int k=0; k<size; k++) {
		if (sigma[k]>0) {
			tmean += t_drift[k];
			counter++;
		}
}

tmean = tmean / counter;
return tmean;

}

inline int V_plot::get_realsize(){
	return realsize;
}

inline int V_plot::get_activesize(){
	int counter = 0;
	for (int k=0; k<size; k++) {
		if (sigma[k]>0)
			counter++;
	}
return counter;
}

////////////////////////////////////////////////////////////////////////////////////
