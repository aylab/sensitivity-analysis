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

using namespace std;

/*	Getting command line arguments from the user, which are stored in input_params& ip.*/
void accept_params (int num_args, char** args, input_params& ip) {
	int sim_args_index = 0;
	bool just_help = false;
	if (num_args > 1) { // if arguments were given and each argument option is followed by a value
		for (int i = 1; i < num_args; i += 2) { // iterate through each argument pair
			char* option = args[i];
			char* value;
			if (i < num_args - 1) {
				value = args[i + 1];
			} else {
				value = NULL;
			}
				
			/*
			Check for each possible argument option and overrides the default value for each specified option. If the option isn't recognized or the value given for an option doesn't appear valid then the usage information for the program is printed with an error message and no simulations are run. The code should be fairly self-explanatory with a few exceptions:
			1) atoi converts a string to an integer, atof converts a string to a floating point number (i.e. rational)
			2) strings should always be compared using strcmp, not ==, and strcmp returns 0 if the two strings match
			3) usage(true) prints the usage information with an error message while usage(false) prints it without one
			*/
			if(ip.sim_args){
				ip.simulation_args[sim_args_index] = option;
				sim_args_index++;
				i--;
				if (strcmp(option, "-h") == 0 || strcmp(option, "--help") == 0){
					just_help = true;
				}
			} else if (strcmp(option, "-e") == 0 || strcmp(option, "--exec") == 0) {
				ensure_nonempty(option, value);
				ip.sim_exec = value;
			} else if (strcmp(option, "-n") == 0 || strcmp(option, "--nominal-file") == 0) {
				ensure_nonempty(option, value);
				ip.nominal_file = value;
			} else if (strcmp(option, "-d") == 0 || strcmp(option, "--data-dir") == 0) {
				ensure_nonempty(option, value);
				ip.data_dir = value;
			} else if (strcmp(option, "-v") == 0 || strcmp(option, "--verbose-file") == 0) {
				if(value == NULL || value[0] == '-'){
					i--;
				} else{
					ip.verbose_file = value;
				}
				ip.verbose = true;
			} else if (strcmp(option, "-s") == 0 || strcmp(option, "--random-seed") == 0) {
				ensure_nonempty(option, value);
				ip.random_seed = atoi(value);
				if (ip.random_seed < 1) {
					usage("You must use a postivie, non-zero integer for the ranodm seed you would like to perform.", 0);
				}
			} else if (strcmp(option, "-l") == 0 || strcmp(option, "--processes") == 0) {
				ensure_nonempty(option, value);
				ip.processes = atoi(value);
				if (ip.processes < 1) {
					usage("I doubt you want a zero or negative amount of processes to run.", 0);
				}
			} else if (strcmp(option, "-P") == 0 || strcmp(option, "--points") == 0) {
				ensure_nonempty(option, value);
				ip.points = atoi(value);
				if (ip.points < 1) {
					usage("I doubt you want a zero or negative amount of points to analyze.", 0);
				}
			} else if (strcmp(option, "-p") == 0 || strcmp(option, "--percentage") == 0) {
				ensure_nonempty(option, value);
				ip.percentage = atof(value);
				if (ip.percentage == 0) {
					usage("I doubt you want a zero percent perturbation.", 0);
				} else if(ip.percentage < 0){
					ip.percentage = -1*ip.percentage;
				}
			} else if (strcmp(option, "-q") == 0 || strcmp(option, "--quiet") == 0) {
				ip.quiet = true;
				i--;
			} else if (strcmp(option, "-a") == 0 || strcmp(option, "--sim-args") == 0) {
				ip.sim_args = true;
				ip.simulation_args = new char*[num_args - i  + 7];
				ip.sim_args_num = num_args - i  + 7;
				sim_args_index = 7;
				i--;
			} else if (strcmp(option, "-h") == 0 || strcmp(option, "--help") == 0) {
				const char* mess = "Welcome to the help options.\n Possible command line arguments are:\n"; 
				usage(mess,0);	
				i--;
			}
		}
	}
	//Use minimum system resources if the user just wants deterministic help menu.
	if(just_help){
		ip.processes = 1;
		ip.quiet = true;
	}
	//Setting up quiet mode.
	if(ip.quiet) cout_switch(true, ip);
	
	//Making the directory in which all of the simulation data will be stored.
	make_dir(ip.data_dir);
	
	//Initializing some arguments that are always passed into the simulation program.
	if(!ip.sim_args){
		ip.simulation_args = new char*[8];
		ip.sim_args_num = 8;
		sim_args_index = 7;
	}	
	ip.simulation_args[0] = ip.sim_exec;
	ip.simulation_args[1] = (char*)"--pipe-in";
	ip.simulation_args[3] = (char*)"--pipe-out";
	ip.simulation_args[5] = (char*)"--print-cons";
	ip.simulation_args[sim_args_index] = NULL;
	//Initializing the random seed.
	init_seed(ip);
}

void ensure_nonempty (const char* flag, const char* arg) {
	if (arg == NULL) {
		char* message = (char*)malloc(strlen("Missing argument for '' flag.") + strlen(flag) + 1);
		sprintf(message, "Missing the argument for the '%s' flag.", flag);
		usage(message, 0);
	}
}

void cout_switch(bool turn_off, input_params& ip){
	if(turn_off){
		ip.cout_orig = cout.rdbuf();
		ip.null_stream = new ofstream("/dev/null");
		cout.rdbuf(ip.null_stream->rdbuf());
	} else{
		cout.rdbuf(ip.cout_orig);			
	}
}

void usage(const char* message, int error){
	cout << message << endl;
	if(error){
		cerr << "\tError: " << error << endl;
	}
	cout << "Usage: [-option [value]]. . . [--option [value]]. . ." << endl;	
	cout << "-n, --nominal-file   [filename]   : the relative name of the file from which the nominal parameter set should\
                                                 be read, default=nominal.params" << endl;
	cout << "-d, --data-dir       [filename]   : the relative name of the directory to which the raw simulation data\
                                                 will be stored, default=sim-data" << endl;
	cout << "-v, --verbose-file   [filename]   : the relative name of the file to which verbose information will be written,\
                                                 default=none" << endl;
	cout << "-p, --percentage     [float]      : the maximum percentage by which nominal values will be perturbed (+/-),\
	                                             default=5" << endl;
	cout << "-P, --points         [int]        : the number of data points to collect on either side (+/-) of the nominal set,\
                                                 default=10" << endl; 
	cout << "-s, --random-seed    [int]        : the postivie integer value to be used as a seed in the random\
                                                 number generation for simulations, default is randomly generated\
                                                 based on system time and process id" << endl;
	cout << "-l, --processes      [int]        : the number of processes to which parameter sets can be sent for\
                                                 parallel data collection, default=2" << endl;
	cout << "-q, --quiet          [N/A]        : include this to turn off printing messages to standard output,\
                                                 disabled by default" << endl;
	cout << "-e, --exec           [path]       : if included, the simulations are run by executing the program\
                                                 specified by path. The path argument should be the full path,\
                                                 but the default uses the relative path: \"../sogen-deterministic/deterministic\"." << endl;
	cout << "-a, --sim-args       [args]       : if included, any argument after this will be passed to the simulation\
                                                 program. If -h is one of these arguments, the simulation help will be\
                                                 printed and the program will not run." << endl;
	cout << "-h, --help           [N/A]        : print out this help menu." << endl; 			
	exit(error);
}

void init_seed (input_params& ip) {
	if (ip.random_seed == 0) {
		ip.random_seed = abs((((int)time(0) * 181) * ((getpid() - 83) * 359)) % 805306457); // the final seed value, calculated using the given seed and the process ID
	}
	srand(ip.random_seed);
}

void make_dir(char* dir){
	if(-1 == mkdir((const char*)dir, S_IRWXU) && errno != EEXIST){
		usage("Could not make directory.", errno);
	}
}
