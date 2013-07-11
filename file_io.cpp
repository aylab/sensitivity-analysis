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


#include "init.hpp"
#include "file_io.hpp"

using namespace std;

void read_nominal(input_params& ip){
	FILE* file_pointer = fopen(ip.nominal_file, "r");
	ip.dims = count_params(file_pointer);
	ip.nominal = new double[ip.dims];
	if(! fill_doubles(file_pointer, ip.dims, ip.nominal)){
		//cerr << "There are less than " << ip.dims << " values in the file " << ip.nominal_file << "\n";	
		delete[] ip.nominal;
		ip.nominal = NULL;
	}
	fclose(file_pointer);
}

int count_params(FILE* file_pointer){
	int count = 0;
	bool  in_num = false;
	char c = '\0';
	int read = 1;
	for(; c != '\n' && read == 1; read = fscanf(file_pointer, " %c", &c)){
		if(!in_num && is_num(c)){
			count++;
		}
		in_num = is_num(c);
	}
	cout << "I am a plane:\n"; 
	rewind(file_pointer);
	return count;	
}

bool fill_doubles(FILE* file_pointer, int param_num, double* nominal){
	int result = 0;
	for(int i = 0; i < param_num; i++){
		result = fscanf(file_pointer, "%lf%*[,;\t ]", nominal+i); 
		if ( 1 != result){
			//cout << "res: " << result << "\n";
			return false;
		}
		//cout << nominal[i] << "\n";
	}
	return true;
}

