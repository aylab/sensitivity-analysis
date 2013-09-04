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
init.cpp contains initialization functions used before actual sensitivity analysis occurs.
*/

#include "init.hpp" // Function declarations

#include "analysis.hpp"

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
			} else if (strcmp(option, "-D") == 0 || strcmp(option, "--data-dir") == 0) {
				ensure_nonempty(option, value);
				ip.data_dir = copy_str((const char*)value);
			} else if (strcmp(option, "-d") == 0 || strcmp(option, "--sense-dir") == 0) {
				ensure_nonempty(option, value);
				ip.sense_dir = value;
			} else if (strcmp(option, "-c") == 0 || strcmp(option, "--nominal-count") == 0) {
				ensure_nonempty(option, value);
				ip.num_nominal = atoi(value);
				if (ip.num_nominal < 1) {
					usage("You must use a postivie, non-zero integer for the number of nominal sets from the file you would like to analyze.", 0);
				}
			}else if (strcmp(option, "-k") == 0 || strcmp(option, "--skip") == 0) {
				ensure_nonempty(option, value);
				ip.set_skip = atoi(value);
				if (ip.set_skip < 0) {
					usage("You must use a postivie integer for the number of nominal sets you would like to skip.", 0);
				}
			}else if (strcmp(option, "-s") == 0 || strcmp(option, "--random-seed") == 0) {
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
			} else if (strcmp(option, "-g") == 0 || strcmp(option, "--generate-only") == 0) {
				ip.generate_only = true;
				i--;
			} else if (strcmp(option, "-z") == 0 || strcmp(option, "--delete-data") == 0) {
				ip.delete_data = true;
				i--;
			} else if (strcmp(option, "-y") == 0 || strcmp(option, "--recycle") == 0) {
				ip.recycle = true;
				i--;
			} else if (strcmp(option, "-q") == 0 || strcmp(option, "--quiet") == 0) {
				ip.quiet = true;
				i--;
			} else if (strcmp(option, "-a") == 0 || strcmp(option, "--sim-args") == 0) {
				ip.sim_args = true;
				ip.simulation_args = new char*[num_args - i  + 9];
				ip.sim_args_num = num_args - i  + 9;
				sim_args_index = 9;
				i--;
			} else if (strcmp(option, "-l") == 0 || strcmp(option, "--licensing") == 0) {
				licensing();
				i--;
			} else if (strcmp(option, "-h") == 0 || strcmp(option, "--help") == 0) {
				const char* message = "Welcome to the help options.\n Possible command line arguments are:\n"; 
				usage(message, 0);
				i--;
			} else{
				const char* message_0 = "'";
				const char* message_1 = "' is not a valid option! Please check that every argument matches one available in the following usage information.";
				char* message = (char*)mallocate(sizeof(char) * (strlen(message_0) + strlen(option) + strlen(message_1) + 1));
				sprintf(message, "%s%s%s", message_0, option, message_1);
				usage(message, 0);
			}
		}
	} else{
		usage("No arguments given...", 0);
	}
	
	//If a custom name is not included for data_dir, this gives data dir a name of the format "sim-data-[pid]" where the pid is useful to ensure unique working directory.
	if(ip.data_dir == NULL){
		ip.data_dir = (char*)mallocate(sizeof(char)*(strlen("sim-data-") + len_num(getpid()) + 1));
		sprintf(ip.data_dir, "%s%d", (char*)"sim-data-", getpid()); 
	}

	//Making the directories in which all of the simulation data and results will be stored.
	make_dir(ip.data_dir);
	make_dir(ip.sense_dir);
	
	//Use minimum system resources if the user just wants deterministic help menu.
	if(just_help){
		ip.processes = 1;
		ip.quiet = true;
	}
	//Setting up quiet mode.
	if(ip.quiet) cout_switch(true, ip);
	
	//Initializing the random seed.
	init_seed(ip);
	
	//Initializing some arguments that are always passed into the simulation program. The arguments that need to be filled in are taken care of by file_io.cpp in make_arg()
	if(!ip.sim_args){
		ip.simulation_args = new char*[10];
		ip.sim_args_num = 10;
		sim_args_index = 9;
	}	
	ip.simulation_args[0] = ip.sim_exec;
	ip.simulation_args[1] = (char*)"--pipe-in";
	ip.simulation_args[3] = (char*)"--pipe-out";
	ip.simulation_args[5] = (char*)"--print-osc-features";
	ip.simulation_args[7] = (char*)"--seed";
	ip.simulation_args[sim_args_index] = NULL;
	}

void ensure_nonempty (const char* flag, const char* arg) {
	if (arg == NULL) {
		char* message = (char*)mallocate(strlen("Missing argument for '' flag.") + strlen(flag) + 1);
		sprintf(message, "Missing the argument for the '%s' flag.", flag);
		usage(message, 0);
	}
}

/*	Function for turning quit mode on/off by redirecting cout.
*/
void cout_switch (bool turn_off, input_params& ip){
	if(turn_off){
		ip.cout_orig = cout.rdbuf();
		ip.null_stream = new ofstream("/dev/null");
		cout.rdbuf(ip.null_stream->rdbuf());
	} else{
		cout.rdbuf(ip.cout_orig);			
	}
}

char* copy_str (const char* str) {
	char* newstr = (char*)mallocate(sizeof(char) * strlen(str) + 1);
	return strcpy(newstr, str);
}

void init_seed (input_params& ip) {
	if (ip.random_seed == 0) {
		ip.random_seed = abs((((int)time(0) * 181) * ((getpid() - 83) * 359)) % 805306457); // the final seed value, calculated using the given seed and the process ID
	}
	srand(ip.random_seed);
}

//Safely creates a directory and checks for success.
void make_dir (char* dir) {
	if(-1 == mkdir((const char*)dir, S_IRWXU) && errno != EEXIST){
		usage("Could not make directory.", errno);
	}
}

//Deletes a directory that is empty. This is used by the destructor of input_params (if -z was passed as an argument) to delete the oscillation features files after they have been processed. The files within the directory should have been deleted in the main loop of LSA_all_dims().
void unmake_dir (char* dir) {
	int made = mkdir((const char*)dir, S_IRWXU);
	if( (-1 == made && errno == EEXIST) || made == 0){
		if( -1 == rmdir((const char*)dir)){
			cerr << "Could not remove simulation data directory." << endl;
			return;		
		}		
	} else{
		unmake_dir(dir);
	}
	return;
}

//Removes a file if remove is true, otherwise does nothing. This is used in the main loop of LSA_all_dims() to delete simulation data files after the have been processed.
void unmake_file (char* file_name, bool remove) {
	if(!remove){
		return;
	}
	unlink((const char*)file_name);	
}


/*	These checks are primarily for the sensivitity of period -- when there are damped oscillations the period may be measured as INFINITY, in which case the sensitivity may become infinity.
	In the case of a an "infinite" value (e.g. period), we choose to treat the infinite value as finite but higher than any functioning value, i.e. the INF_SUBSTITUTE macro.
*/
double check_num (double num) {
	if(isinf(num)){
		return INF_SUBSTITUTE;
	}else if (isnan(num)){
		return 0;
	}
	return num;
}

