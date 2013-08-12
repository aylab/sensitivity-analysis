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
macros.hpp is used for consolodating the macros used by analysis.cpp, init.hpp, io.cpp.
This file does not contain any of the macros used by finite-difference.cpp/hpp because those files are intended to make up an independent library.
*/

#ifndef MACROS_HPP
#define MACROS_HPP

//This macro specifies the maximum number of features that may be read from a simulation output file. If there are more features than this quantitiy in the output file they will be ignored and their sensitivity will not be calculated. 
#define MAX_NUM_FEATS 150

//	This macro is used when checking infinite values. See the check_num function in analysis.cpp.
#define INF_SUBSTITUTE 500

/*	Character checking macros used for parsing by file_io.cpp.
*/
#define is_num(c) (('1' <= c && c <= '9') || (c == '0') || (c == '.') || (c == 'e'))
#define alph_num_slash( car ) ( ('a' <= car && car <= 'z') || ('A' <= car && car <= 'Z') || ('1' <= car && car <= '9') || (car == '0') || (car == '/') || (car == ' '))

/*	Simple math macro functions
*/
#define len_num(num) ( log10(num+1)+1 )
#define abs(num) ( ( num < 0 ? -1*num : num) )
#define min(alpha, beta) (alpha < beta ? alpha : beta)

//This macro function is used to take away the dimensionality of sensitivity values. See the summary of sensitivity analysis at the beginning of analysis.cpp.
#define non_dim_sense(nom_param, nom_out, dout_dparam) ( ((double)nom_param / (double)nom_out) * (double)dout_dparam )

//This macro is useful for zeroing-out negative values so that parmeter values will never be negative. This is used by sim_set->fill() in init.hpp.
#define at_least_zero(num) (num > 0.0 ? num : 0)

#endif

