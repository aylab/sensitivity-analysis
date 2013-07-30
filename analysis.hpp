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
#ifndef ANALYSIS_HPP
#define ANALYSIS_HPP

//	This macro is used when checking infinite values. 
#define INF_SUBSTITUTE 500

//Function declarations.
void generate_data(input_params&, sim_set&);
void LSA_all_dims(input_params& , sim_set& );
double* fin_dif_one_dim(int, int, double, double**);
void normalize(int , int t, double** );
double check_num(double );
void del_double_2d(int , double** );
void del_char_2d(int , char** );
#endif
