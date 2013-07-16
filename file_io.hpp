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

#ifndef FILE_IO_HPP
#define FILE_IO_HPP

using namespace std;

void read_nominal(input_params& );
int count_params(FILE* );
bool fill_doubles(FILE* , int , double* );
double** load_output(int, int*, char* );

bool make_pipes(int , int** );
void del_pipes(int , int** , bool );

char*** make_all_args(int , input_params& ,int** );
void make_arg(int first_dim, int sim_args_num, int* pipes, char* dir_name, char* dim_name, char** simulation_args, char** destination);
char* make_name(char* dir, char* file, int num);
void del_args(int, int, char*** );
void del_arg(int sim_args_num, char** arg);
void segs_per_sim(int , int , int* );
void simulate_samples(int , input_params& , sim_set&  );
bool write_info(int , int, int);
bool write_dim_sets(int , int , double* , sim_set& );

void simulate_nominal(input_params& ip);

bool write_nominal(input_params& ip, int fd);
bool check_status(int status, int simpid, int* failcode, char* failure);


#endif 
