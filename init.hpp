/*
Local Sensitivity Analysis progam, designed for use with the Deterministic simulator for zebrafish segmentation.
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
#ifndef INIT_HPP
#define INIT_HPP
//include <library> 	//(What the library is for)
#include <stdlib.h>		//(Standard library.)
#include <cstring>		//(Used for strdup, memcpy, etc.)
#include <iostream>		//(Reading/writing files, and for setting cout in quiet mode.)
#include <streambuf>	//(Setting cout in quiet mode.)
#include <fstream>		//(Reading/writing files.)
#include <unistd.h>		//(Needed for removing files/directory, and by io for forking processes and piping data)
#include <cmath>		//(Needed for isinf() checks and log10 call.)
#include <sys/stat.h>	//(Used for file and directory checking and making)
#include <errno.h>		//(Some libraries use error codes that are useful for checking things, like mkdir)
using namespace std;

//Character checking macros used for parsing (by file_io.cpp)
#define is_num(c) (('1' <= c && c <= '9') || (c == '0') || (c == '.') || (c == 'e'))
#define alph_num_slash( car ) ( ('a' <= car && car <= 'z') || ('A' <= car && car <= 'Z') || ('1' <= car && car <= '9') || (car == '0') || (car == '/') || (car == ' '))
//Handy math macros
#define len_num(num) ( log10(num+1)+1 )
#define abs(num) ( ( num < 0 ? -1*num : num) )
//This macro is useful for zeroing-out negative values so that parmeter values will never be negative.
#define at_least_zero(num) (num > 0.0 ? num : 0)

//Declaring this here so it can be used by the input_params destructor.
void unmake_dir(char*);
void unmake_file(char* , bool );

//Struct for holding on to input arguments and values.
struct input_params{	
	bool sim_args;
	bool quiet;
	bool recycle;
	bool delete_data;
	bool generate_only;
	int random_seed;
	int processes;
	int sim_args_num;
	int num_nominal;
	int line_skip;
	int dims;
	double percentage; //Max percentage by which we will perturb parameters +/-
	int points; //Number of points between the nominal and the max percentage +/- to generate data for
	double* nominal; //Array for storing the nominal parameter set.
	streambuf* cout_orig;
	ofstream* null_stream;
	char* nominal_file;
	char* sense_dir;
	char* sense_file;
	char* norm_file;
	char* data_dir;
	char*dim_file;
	char* sim_exec;
	char** simulation_args;
	char* failure;
	int failcode;
	
	input_params(){
		quiet = false;
		sim_args = false;
		recycle = false;
		delete_data = false;
		generate_only = false;
	 	dims= 0;
	 	percentage = 5;
	 	points = 2;
	 	processes = 2;
	 	num_nominal = 1;
	 	line_skip = 0;
	 	dims = -1;
		nominal_file = (char*)"nominal.params";
		sense_dir = (char*)"sensitivities";
		sense_file = (char*)"LSA_";
		norm_file = (char*)"normalized_";
		data_dir = NULL;
		dim_file = (char*)"dim_";		
		sim_exec = (char*) "../sogen-deterministic/deterministic";
		simulation_args = NULL;
		null_stream = NULL;
		failure = NULL;
		failcode = 0;
	}
	
	~input_params(){
		if(delete_data){
			unmake_dir(data_dir);
		}
		if(data_dir != NULL) free(data_dir);
		if(nominal != NULL) delete[] nominal;
		if(simulation_args != NULL) delete[] simulation_args;
	}
};

//Struct for holding all the sets that need to be simulated.
struct sim_set{
	int dims; //Just holds a copy of how many dimensions/parameters are being used.
	int sets_per_dim; //Number of sets to simulate for data per dimension that will be perturbed.
	int points; //Just holds a copy of the number of points to use.
	double step_per_set; //Decimal difference between perturbations.
	double** dim_sets; //An array for holding the perturbed values. See fill() for a description of the structure of this array.
	sim_set(input_params& ip){
		dims = ip.dims;
		points = ip.points;
		sets_per_dim = 2*ip.points;
		step_per_set = (ip.percentage /( (double)100*ip.points ));
		dim_sets = new double*[dims];
		this->fill(ip.nominal);
		
	}
	~sim_set(){
		for(int i = 0; i < dims; i++){
			delete[] dim_sets[i];
		}
		delete[] dim_sets;
	}
	
	/*	This funciton fills the array dim_sets using the nominal parameter values and the calculated perterbation.
		The rows of dim_sets are parameter indicies, the columns are the parameter values with increasing amounts of perturbation (from most-negative to most-positive).
		For example, for a ten percent perturbation with points = 2, dim_sets looks like: 
		{ {dim_0 - 10%, dim_0 - 5%, dim_0 + 5%, dim_0 + 10%}
		  {dim_1 - 10%, dim_1 - 5%, dim_1 + 5%, dim_1 + 10%}
		  ...
		  {dim_n - 10%, dim_n - 5%, dim_n + 5%, dim_n + 10%}  }
		Where dim_i is the nominal value for the i'th parameter (and the percentages are for that particular parameter).
		
		July 25, 2013: Confirmed that this array is being filled with the correct values.
		July 30, 2013: Added in an at_least_zero() check that will ensure no negative values will be used, even if the perturbation percentage is >= 100%. 		
	*/
	void fill(double* nominal){
		for(int i = 0; i < dims; i++){
			dim_sets[i] = new double[sets_per_dim];
			int j = 0;
			for(; j < points; j++){
				dim_sets[i][j] = at_least_zero( nominal[i] * ((double)1 + step_per_set*(double)(j - points)) );
			}
			j++;
			for(; j < sets_per_dim + 1; j++){
				dim_sets[i][j-1] = at_least_zero( nominal[i] * ((double)1 + step_per_set*(double)(j - points)) );
			}
		}
	}
};


//Init functions
void init_seed (input_params& );
void accept_params (int , char** , input_params& );
void ensure_nonempty (const char* , const char* );
void cout_switch(bool , input_params& );
void usage(const char*, int);
void make_dir(char*);

#endif
