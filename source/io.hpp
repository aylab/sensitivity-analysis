/*
Sensitivity analysis for simulations
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
io.hpp contains function declarations for io.cpp.
*/

#ifndef IO_HPP
#define IO_HPP

#include <fcntl.h>		//(Needed to check on open pipes when closing them.)
#include <sys/wait.h>	//(Waiting on processes to finish and reading their return status.)

#include "init.hpp"

using namespace std;
/* Function declarations */
//File input:
void read_nominal(input_params& );
int count_params(FILE* );
bool fill_doubles(FILE* , int , double* );
void skip_lines( FILE* , int);
double** load_output(int, int*, char*, char*** );

//File output:
void write_sensitivity(int , int , char** , double** , char*  );

//Simulation execution functions:
void simulate_samples(int , input_params& , sim_set&  );
void simulate_nominal(input_params& );

//Simulation execution helper functions:
char*** make_all_args(int , input_params& ,int** );
void make_arg(int , int , int, int* , char* , char* , char** , char** );
char* make_name(char* , char* , int );
void del_args(int, int, char*** );
void del_arg(int , char** );
bool make_pipes(int , int** );
void del_pipes(int , int** , bool );
void segs_per_sim(int , int , int* );
bool write_info(int , int, int);
bool write_dim_sets(int , int , double* , sim_set& );
bool write_nominal(input_params& , int );
bool check_status(int , int , int* , char** );

#endif

