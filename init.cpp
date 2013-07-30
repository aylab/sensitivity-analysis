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

#include "init.hpp"
#include "finite_difference.hpp"

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
				ip.data_dir = value;
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
				ip.line_skip = atoi(value);
				if (ip.line_skip < 0) {
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
			} else if (strcmp(option, "-h") == 0 || strcmp(option, "--help") == 0) {
				const char* mess = "Welcome to the help options.\n Possible command line arguments are:\n"; 
				usage(mess,0);	
				i--;
			}
		}
	}
	//If a custom name is not included for data_dir, this gives data dir a name of the format "sim-data-[pid]" where the pid is useful to ensure unique working directory.
	if(ip.data_dir == NULL){
		ip.data_dir = (char*)malloc(sizeof(char)*(strlen("sim-data-") + len_num(getpid()) + 1));
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
		char* message = (char*)malloc(strlen("Missing argument for '' flag.") + strlen(flag) + 1);
		sprintf(message, "Missing the argument for the '%s' flag.", flag);
		usage(message, 0);
	}
}

/*	Function for turning quit mode on/off by redirecting cout.
*/
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
	cout << "\
-n, --nominal-file   [filename]   : the relative name of the file from which the nominal parameter set should\n\
                                     be read, default=nominal.params\n\
-d, --sense-dir      [filename]   : the relative name of the directory to which the sensitivity results\n\
                                     will be stored, default=sensitivities\n\
-D, --data-dir       [filename]   : the relative name of the directory to which the raw simulation data\n\
                                     will be stored, default=sim-data\n\
                                     WARNING: IF RUNNING MULTIPLE INSTANCES OF THIS PROGRAM (e.g. on cluster)\n\
                                     EACH MUST HAVE A UNIQUE DATA DIRECTORY TO AVOID CONFLICT.\n\
-p, --percentage     [float]      : the maximum percentage by which nominal values will be perturbed (+/-),\n\
                                     default=5\n\
-P, --points         [int]        : the number of data points to collect on either side (+/-) of the nominal set,\n\
                                     default=10\n\
-c, --nominal-count  [int]        : the number of nominal sets to read from the file, default=1\n\
-k, --skip           [int]        : the number of nominal sets in the file to skip over, a.k.a. the\n\
                                     index of the line you would like to start reading from, default=0\n\
-s, --random-seed    [int]        : the postivie integer value to be used as a seed in the random\n\
                                     number generation for simulations, default is randomly generated\n\
                                     based on system time and process id\n\
-l, --processes      [int]        : the number of processes to which parameter sets can be sent for\n\
                                     parallel data collection, default=2\n\
-y, --recycle        [N/A]        : include this if the simulation output has already been\n\
                                     generated FOR EXACTLY THE SAME FILES AND ARGUMENTS YOU\n\
                                     ARE USING NOW, disabled by default\n\
-z, --delete-data    [N/A]        : include this to delete oscillation features data when the program\n\
                                     exits. This will preserve sensitivity directory but remove the\n\
                                     directory specified by -D, disabled by default.\n\
-q, --quiet          [N/A]        : include this to turn off printing messages to standard output,\n\
                                     disabled by default\n\
-e, --exec           [path]       : if included, the simulations are run by executing the program\n\
                                     specified by path. The path argument should be the full path,\n\
                                     but the default uses the relative path: \n\"../sogen-deterministic/deterministic\n\".\n\
-a, --sim-args       [args]       : if included, any argument after this will be passed to the simulation\n\
                                     program. If -h is one of these arguments, the simulation help will be\n\
                                     printed and the program will not run.\n\
-h, --help           [N/A]        : print out this help menu.\n" ; 		
	exit(error);
}

void init_seed (input_params& ip) {
	if (ip.random_seed == 0) {
		ip.random_seed = abs((((int)time(0) * 181) * ((getpid() - 83) * 359)) % 805306457); // the final seed value, calculated using the given seed and the process ID
	}
	srand(ip.random_seed);
}

//Safely creates a directory and checks for success.
void make_dir(char* dir){
	if(-1 == mkdir((const char*)dir, S_IRWXU) && errno != EEXIST){
		usage("Could not make directory.", errno);
	}
}

//Deletes a directory that is empty. Could have the functionality to recursively remove contents if not empty, but doesn't currently.
void unmake_dir(char* dir){
	int made = mkdir((const char*)dir, S_IRWXU);
	if( (-1 == made && errno == EEXIST) || made == 0){
		if( -1 == rmdir((const char*)dir)){
			cerr << "Could not remove directory." << endl;		
		}		
	} else{
		usage("This can't be happening", errno);
	}
	return;
}

//Removes a file if remove is true, otherwise does nothing. Should put some error checking here.
void unmake_file(char* file_name, bool remove){
	if(!remove){
		return;
	}
	unlink((const char*)file_name);	
}
