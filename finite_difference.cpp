/*
Finite difference calculator with varying orders of accuracy,
Designed for use with the sensitivity-analysis package,
for the Deterministic simulator of zebrafish segmentation
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

#include "finite_difference.hpp"

using namespace std;

/*	A wrapper function for taking the minimum arguments and returning the finite difference, 
disregarding error unless it is greater than the step size.
*/
double finite_difference(int num_points, double step_size, double* function_values){
	int output_shift = 0;
	num_points -= num_points % 2;
	if(num_points > ACC_MAX){
		output_shift += ACC_MAX / 2;
	}
	
	double round_error;
	double fin_dif;
	fdy_fdx(num_points - output_shift, step_size, function_values, &fin_dif, &round_error);
	/*
	if( round_error > step_size){
		cerr << "Error was greater than step size (" << round_error << ">" << step_size << ").\n Value cannot be trusted.\n";
	}
	*/
	return fin_dif;
}

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
void fdy_fdx(int accuracy, double delta_independent, double* dependent, double* fin_dif_output, double* round_error){
	fin_dif_coef fdc(accuracy);
	double numerator = sum_num(dependent, fdc);
	double fin_dif  = numerator / delta_independent;
	if(round_error != NULL){
	 *round_error = numerator - (fin_dif * delta_independent);
	}
	*fin_dif_output = fin_dif;
}

double sum_num(double* dependent, fin_dif_coef& fdc){
	double numerator = 0;
	for(int i = 0; i < fdc.accuracy; i++){
		numerator += dependent[i]*fdc.coef[i];
	}
	return numerator;
}
