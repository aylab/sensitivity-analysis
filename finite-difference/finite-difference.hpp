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

/*
finite-difference.hpp contains function declarations, macros, and structs for finite-difference.cpp.
*/

#ifndef FINITE_DIFFERENCE_HPP
#define FINITE_DIFFERENCE_HPP

//This if checks to see if the sensitivity analysis init.hpp file has already been included -- if so, these libraries don't need to be included again.
#ifndef INIT_HPP
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include <errno.h>
#endif

//Min and max accuracy macros and a nested boolean function that returns min if num<min, max if max<num, and num otherwise.
#define ACC_MAX 8
#define ACC_MIN 2
#define minmax(min, num, max) ( min >= num ? min : ( max < num ? max : num))

/*	Macros for defining the coefficient array.
	Later version may include a general form for defining these coefficients, but an accuracy of (∆x)^8 should be adequate, if not superfuous due to computational rounding error.
*/
#define C_EIGHT {((double)1)/280, -1*((double)4)/105, ((double)1)/5, -1*((double)4)/5, ((double)4)/5, -1*((double)1)/5, ((double)4)/105, -1*((double)1)/280}
#define C_SIX {-1*((double)1)/60, ((double)3)/20, -1*((double)3)/4,  ((double)3)/4,  -1*((double)3)/20, ((double)1)/60}
#define C_FOUR {((double)1)/12, -1*((double)2)/3,  ((double)2)/3,  -1*((double)1)/12}
#define C_TWO {-1*((double)1)/2,  ((double)1)/2}
/*
Chart describign coefficient assignment for different levels of accuracy:

x-step:	 	−4		−3		−2		−1		0		1		2		3		4
--------
Accuracy
2	 	 	 						−1/2	0		1/2	 	 	 
4	 	 					1/12	−2/3	0		2/3		−1/12	 	 
6	 				−1/60	3/20	−3/4	0		3/4		−3/20	1/60	 
8			1/280	−4/105	1/5		−4/5	0		4/5		−1/5	4/105	−1/280
*/

using namespace std;

//Struct declaration -- this struct takes care of holding the correct coefficients based on the accuracy value given.
struct fin_dif_coef{
	int accuracy;
	double* coef;
	
	//Constructor takes an accuracy value, a, ensures it is valid value, then allocates coef to be the appopriate size and fills it in based of the macro values.
	fin_dif_coef(int a){
		accuracy = a + (a % 2); //This ensures that accuracy=a if a is even, but accuracy = a+1 if a is odd.
		accuracy = minmax(ACC_MIN, accuracy, ACC_MAX); //This ensures a is one of the hard-coded acceptible values.
		//This switch statement determines how coef should be filled depending on the accuracy value.
		switch (accuracy)
		{
			case 8:{
				double temp8[8] = C_EIGHT;
				this->fill(temp8); 
				break;
			}
			case 6:{
				double temp6[6] = C_SIX;
				this->fill(temp6);
				break;
			}
			case 4:{
				double temp4[4] = C_FOUR;
				this->fill(temp4);
				break;
			}
			default:{
				double temp2[2] = C_TWO;
				this->fill(temp2);
				break;
			}
		}
			
	}
	
	~fin_dif_coef(){
		delete[] this->coef;
	}
	
	//This just loops through the macro coef values and puts them in the coef array.
	void fill(double* source){
		this->coef = new double[this->accuracy];
		for(int i = 0; i < this->accuracy; i++){
			this->coef[i] = source[i];
		}
	}
};

//Function declarations for finite-difference.cpp:
double finite_difference(int num_points, double step_size, double* function_values);
void fdy_fdx(int accuracy, double delta_independent, double* dependent, double* fin_dif_output, double* round_error);
double sum_num(double* dependent, fin_dif_coef& fdc);

#endif

