using namespace std;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 12.05.1999, AUTHOR: OLIVER KORTNER
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF INLINE METHOD DEFINED IN THE CLASS Abs_pol //
//////////////////////////////////////////////////////////////////

//*****************************************************************************

/////////////////
// METHOD init //
/////////////////

inline void Abs_pol::init(const double & t_0, const double & r_0,
							vector<double> coeff) {

///////////////
// VARIABLES //
///////////////

	int size = coeff.size();

////////////////////
// FILL VARIABLES //
////////////////////

	degr = size;
	t0 = t_0;
	r0 = r_0;
	alpha = new double[size];
	for (int k=0; k<size; k++) {
		alpha[k] = coeff[k];
	}

	return;

}

//*****************************************************************************

////////////////////////
// METHOD coefficient //
////////////////////////

inline double Abs_pol::coefficient(const int & k) const {

////////////////////////////////////////////////
// CHECK IF THE COEFFICIENT NUMBER IS ALLOWED //
////////////////////////////////////////////////

	if (k<1 || k>degr-2) {
		cerr << endl << "Class Abs_pol, method coefficient: "
			<< "illegal coefficient number!" << endl;
		return 0.0;
	}

////////////////////////////////////////////
// IF SO, RETURN THE REQUIRED COEFFICIENT //
////////////////////////////////////////////

	return alpha[k-1];

}

//*****************************************************************************

//////////////////
// METHOD value //
//////////////////

inline double Abs_pol::value(const double & r) const {

/////////////////////////
// AUXILIARY VARIABLES //
/////////////////////////

	double value;

//////////////////////////////
// CALCULATE FUNCTION VALUE //
//////////////////////////////

	value = t0;
	for (int k=0; k<degr; k++) {
		value = value + alpha[k]*pow(fabs(r-r0), k+1);
	}

	return value;

}

//*****************************************************************************

/////////////////////////////
// METHOD first_derivative //
/////////////////////////////

inline double Abs_pol::first_derivative(const double & r) const {

/////////////////////////
// AUXILIARY VARIABLES //
/////////////////////////

	double value;

//////////////////////////////
// CALCULATE FUNCTION VALUE //
//////////////////////////////

	if (degr<1) {
		return 0.0;
	}

	value = alpha[0];
	for (int k=1; k<degr; k++) {
		value = value + alpha[k]*pow(fabs(r-r0), k);
	}

	return value;

}

//*****************************************************************************

///////////////////////
// METHOD set_degree //
///////////////////////

inline void Abs_pol::set_degree(const int & d) {

/////////////////////////
// AUXILIARY VARIABLES //
/////////////////////////

	double *memory;
	memory = new double[degr];

////////////////////
// RETURN, IF d<1 //
////////////////////

	if (d<1) {
		cerr << endl << "Class Abs_pol, method set_degree: "
			<< "illegal degree, must be > 0 !" << endl;
		return;
	}

/////////////////
// FILL MEMORY //
/////////////////

	for (int k=0; k<degr; k++) {
		memory[k] = alpha[k];
	}

///////////////////////
// REALLOCATE MEMORY //
///////////////////////

	delete [] alpha;
	alpha = new double[d];
	for (int k=0; k<degr; k++) {
		alpha[k] = memory[k];
	}

///////////////////////////////////
// CHANGE DEGREE AND FREE MEMORY //
///////////////////////////////////

	degr = d;
	delete [] memory;

	return;

}

//*****************************************************************************

////////////////////
// METHOD set_t_0 //
////////////////////

inline void Abs_pol::set_t_0(const double & value) {

	t0 = value;
	return;

}

//*****************************************************************************

////////////////////
// METHOD set_r_0 //
////////////////////

inline void Abs_pol::set_r_0(const double & value) {

	r0 = value;
	return;

}

//*****************************************************************************

////////////////////////
// METHOD coefficient //
////////////////////////

inline void Abs_pol::set_coefficient(const int & k, const double & value) {

///////////////////////////////////////////
// CHECK IF THE PARAMETER NUMBER IS OKAY //
///////////////////////////////////////////

	if (k<1 || k>degr) {
		cerr << endl << "Class Abs_pol, method coefficient: "
			<< "illegal parameter number!" << endl;
		return;
	}

/////////////////////////////
// IF SO, CHANGE ITS VALUE //
/////////////////////////////

	alpha[k-1] = value;

	return;

}
