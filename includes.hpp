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
#ifndef INCLUDES_HPP
#define INCLUDES_HPP

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

using namespace std;

struct input_params{	
	bool sim_args;
	bool quiet;
	bool verbose;
	int random_seed;
	int processes;
	int sim_args_num;
	int dims;
	double* nominal;
	streambuf* cout_orig;
	ofstream* null_stream;
	char* nominal_file;
	char* verbose_file;
	char* sim_exec;
	char** simulation_args;
	
	input_params(){
		sim_args = false;
		quiet = false;
		verbose = false;
		sim_args = false;
	 	dims= 0;
		nominal_file = (char*)"nominal.params";
		verbose_file = (char*)"verbose.txt";			
		sim_exec = (char*) "../deterministic";
		simulation_args = NULL;
		null_stream = NULL;
	}
	
	~input_params(){
		if(nominal != NULL) delete[] nominal;
		if(simulation_args != NULL) delete[] simulation_args;
	}
};

#endif
