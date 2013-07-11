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
#include "analysis.hpp"
#include "file_io.hpp"

using namespace std;

int main(int argc, char** argv){
	input_params ip;
	accept_params(argc, argv, ip);
	read_nominal(ip);
	if(ip.nominal == NULL) usage("Could not read nominal parameter set.", 0);
	
	return 0;
}



