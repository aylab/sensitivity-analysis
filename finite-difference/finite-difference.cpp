/*
Finite difference calculator with varying orders of accuracy,
Designed for use with the sensitivity-analysis package,
for the deterministic simulator of zebrafish segmentation
Copyright (C) 2013 Ahmet Ay, Jack Holland, Adriana Sperlea, Sebastian Sangervasi

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
finite-difference.cpp contains functionality for a finite difference calculator.
*/

#include "finite-difference.hpp" // Function declarations

using namespace std;

/*	Function: 
		"finite difference of y" / "finite difference of x" 
	Where the input parameters are:
		accuracy = the number of points used to calculate the finite difference
		delta_independent = dx, the step size
		dependent = y, the dependent variable, an array of size = accuracy
	and the array indecies should align properly such that:
		dependent[i] = y_i = f( x_i ) = f( independent[i] )
	where f(x) is the output function for which we are calculating the finite difference.
*/
void fdy_fdx (int accuracy, double delta_independent, double* dependent, double* fin_dif_output, double* round_error) {
	fin_dif_coef fdc(accuracy);
	double numerator = sum_num(dependent, fdc);
	double fin_dif;
	double rerr = 0;
	if( isinf(numerator) != 0 ){
		fin_dif = INFINITY;
		rerr = INFINITY;
	}else if (delta_independent == 0){
		fin_dif = 0;
		rerr = 0;
	} else{
		fin_dif  = numerator / delta_independent;
		rerr = numerator - (fin_dif * delta_independent);
	}
	if(round_error != NULL){
		*round_error =  rerr;
	}
	*fin_dif_output = fin_dif;
}

/*	Simple function for adding up the numerator for the finite difference calculation.
Each term is scaled by the appropriate coefficient as determined by the fin_dif_coef struct.
If each term of the sum is infinite, this returns a sum of zero -- i.e. the function values are not changing.
If the numerator contains at least one finite and one infinite value, the sum returns INFINITY -- i.e. infinite slope.
*/
double sum_num (double* dependent, fin_dif_coef& fdc) {
	double numerator = 0;
	double next = 0;
	int inf = 0;
	for(int i = 0; i < fdc.accuracy; i++){
		next = dependent[i];
		if( isinf(next) != 0){
			inf++;
		} else if(isnan(next) != 0){
			continue;
		}
		numerator += next*fdc.coef[i];
	}
	if(inf == 0){
		return numerator;
	} else if(inf == fdc.accuracy){
		return 0;
	} else{
		return INFINITY;
	}
}


/*	A wrapper function for taking the minimum arguments and returning the finite difference, 
disregarding error -- this is dangerous.
*/
double finite_difference (int num_points, double step_size, double* function_values) {
	int output_shift = 0;
	num_points -= num_points % 2;
	if(num_points > ACC_MAX){
		output_shift += ACC_MAX / 2;
	}
	
	double fin_dif;
	fdy_fdx(num_points - output_shift, step_size, function_values, &fin_dif, NULL);

	return fin_dif;
}

