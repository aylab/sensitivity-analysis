/*
Deterministic simulator for zebrafish segmentation
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

#include <iostream>
#include <streambuf>
#include <fstream>

#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <cmath>
#include <sys/resource.h>
#include <fcntl.h>
#include <cstdio>
#include <sys/stat.h>
#include <errno.h>
using namespace std;

//Handy math macros
#define is_num(c) (('1' <= c && c <= '9') || (c == '0') || (c == '.') || (c == 'e'))
#define alph_num_slash( car ) ( ('a' <= car && car <= 'z') || ('A' <= car && car <= 'Z') || ('1' <= car && car <= '9') || (car == '0') || (car == '/') || (car == ' '))

#define len_num(num) ( log10(num+1)+1 )
#define abs(num) ( ( num < 0 ? -1*num : num) )
#define non_dim_sense(nom_param, nom_out, dout_dparam) ( ((double)nom_param * (double)dout_dparam) / (double)nom_out)

struct input_params{	
	bool sim_args;
	bool quiet;
	bool recycle;
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
		sim_args = false;
		quiet = false;
		sim_args = false;
		recycle = false;
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
		if(nominal != NULL) delete[] nominal;
		if(data_dir != NULL) free(data_dir);
		if(simulation_args != NULL) delete[] simulation_args;
	}
};

//Struct for holding all the sets that need to be simulated.
struct sim_set{
	int dims;
	int sets_per_dim; //Number of sets to simulate for data per dimension that will be perturbed.
	int points;
	double step_per_set; //Decimal difference between perturbations.
	double** dim_sets;
	
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
	void fill(double* nominal){
		for(int i = 0; i < dims; i++){
			dim_sets[i] = new double[sets_per_dim];
			int j = 0;
			for(; j < points; j++){
				dim_sets[i][j] = nominal[i] * ((double)1 + step_per_set*(double)(j - points));
			}
			j++;
			for(; j < sets_per_dim + 1; j++){
				dim_sets[i][j-1] = nominal[i] * ((double)1 + step_per_set*(double)(j - points));
			}
		}
	}
};

void init_seed (input_params& );
void accept_params (int , char** , input_params& );
void ensure_nonempty (const char* , const char* );
void cout_switch(bool , input_params& );
void usage(const char*, int);
void make_dir(char*);

#endif
