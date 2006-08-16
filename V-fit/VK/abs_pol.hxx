///////////////////
// CLASS Abs_pol //
///////////////////

/////////////////////////////////////////////////////////////
// THIS CLASS PROVIDES THE POLYNOMIALS USED IN V_plot FITS //
/////////////////////////////////////////////////////////////

#ifndef AbspolHXX
#define AbspolHXX

//////////////////////////
// INCLUDE HEADER FILES //
//////////////////////////

// standard C++ libraries //
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>

// STL //
#include <vector>
using namespace std;

class Abs_pol {

private:
	int degr; // degree of the polynomial
	double t0, r0, *alpha; // parameters of the polynomial
	inline void init(const double & t_0, 
			const double & r_0,
			vector<double> coeff); // initialization routine


public:
// Constructors
	Abs_pol(void) {
		vector<double>c(1);
		c[0] = 0.0;
		init (0.0, 0.0, c);
		}				// default constructor
	Abs_pol(const double & t_0, const double & r_0,
							vector<double> coeff) {
		init(t_0, r_0, coeff);
		}				// initialization constructor
	~Abs_pol(void) {
		if (degr>0) {
			delete [] alpha;
		}
		}				// destructor

// Methods
// get-methods //
	inline int degree(void) const { return degr; }
						// get degree of the polynomial
	inline double t_0(void) const { return t0; }
						// get t_0
	inline double r_0(void) const { return r0; }
						// get r_0
	inline double coefficient(const int & k) const;
						// get the k-th coefficient,
						// 1 <= k <= degree-2
	inline double value(const double & r) const;
						// get polynomial(r)
	inline double first_derivative(const double & r) const;
						// get first derivative of
						// the polynomial at r
// set-methods //
	inline void set_degree(const int & d);
						// set degree of the polynomial
						// = d (must be > 0)
	inline void set_t_0(const double & value);
						// set t_0 = value
	inline void set_r_0(const double & value);
						// set r_0 = value
	inline void set_coefficient(const int & k, const double & value);
						// set the k-th coefficient =
						// value,
						// 1 <= k <= degree-2

};


////////////////////////////
// INCLUDE INLINE METHODS //
////////////////////////////

#include "abs_pol.ixx"

#endif
