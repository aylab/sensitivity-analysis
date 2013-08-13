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
io.cpp contains functions for input and output of files and pipes. All I/O related functions should be placed in this file.
*/

#include "io.hpp" // Function declarations

#include "init.hpp"
#include "macros.hpp"

using namespace std;

void read_nominal (input_params& ip) {
	//Opens the file for reading.
	FILE* file_pointer = fopen(ip.nominal_file, "r");
	if (file_pointer == NULL) {
		cout << "Could not open the output file.\n";
		exit(1);
	}
	//If this is the first nominal set that is read, we need to set our count of how many parameters there are and initiatlize the nominal array
	if(ip.dims < 1){
		ip.dims = count_params(file_pointer);
		ip.nominal = new double[ip.dims];
	}
	//Skips over any lines that have already been used as nominal sets. Then increments the number of lines that will need to be skipped over in the future.
	skip_lines( file_pointer, ip.set_skip);
	ip.set_skip ++;
	//A call to the fill_doubles founction with an error check.
	if(! fill_doubles(file_pointer, ip.dims, ip.nominal)){
		cerr << "There are less than " << ip.dims << " values in the file " << ip.nominal_file << "\n";	
		delete[] ip.nominal;
		ip.nominal = NULL;
	}
	//Close the nominal file.
	fclose(file_pointer);
}

int count_params (FILE* file_pointer) {
	int count = 0;
	bool  in_num = false;
	char c = '\0';
	int read = 1;
	//For every character in the first line of the file, increment the count for each sequence of decimal numbers.
	for(; c != '\n' && read == 1; read = fscanf(file_pointer, "%c", &c)){
		if(!in_num && is_num(c)){
			count++;
		}
		in_num = is_num(c);
	}
	//Rewind the file pointer back to the beginning.
	rewind(file_pointer);
	return count;	
}

bool fill_doubles (FILE* file_pointer, int param_num, double* nominal) {
	int result = 0;
	for(int i = 0; i < param_num; i++){
		result = fscanf(file_pointer, "%lf%*[,;\t ]", nominal+i); 
		if ( 1 != result){
			return false;
		}
	}
	return true;
}

void skip_lines (FILE* file_pointer, int set_skip) {
	char pound = '\0';
	for(int i = 0; i < set_skip; i++){
		fscanf(file_pointer,"%c", &pound);
		if(pound == '#'){
			i--;
		} 
		fscanf(file_pointer, "%*s\n");
	}
}

/*	This functions puts output features data into a double[j][i] where j is the index of an output feature and i is the index of the value of that feature at a particular perturbation.
This function also counts how many features there are and stores that value in num_types. Also, if ouptput_names is not NULL, this function puts the names of each output feature in a char** and has output_names point to that array.
	Note that this function can be prone to misreading data if the format of output features files changes. Currently the file is expected to look like:

set,post sync wildtype,post per wildtype,post amp wildtype,post per wildtype/wt,post amp wildtype/wt,ant sync wildtype,ant per wildtype,ant amp wildtype,ant per wildtype/wt,ant amp wildtype/wt,
0,0.999999999999999888977697537484,29.8797435897436045593167364132,56.0505555846975624945116578601,1,1,0,0,0,-nan,-nan,PASSED
1,1,30.2323076923076676791879435768,166.255079755418790909970994107,1,1,0,0,0,-nan,-nan,PASSED
	
	where the number of features can be arbitrary but 0) at most MAX_NUM_FEATS will be read, 1) all values must be comma-seperated with no spaces, 2) the names line must contain a name for every feature, 3) the last value in non-name lines must be followed by a comma, but can have any string after that before the new line, 4) The maximum number of features that can be read is set by the macro MAX_NUM_FEATS and is currently 150. 
	Also important is the fact that there should be no name for the "PASSED" or "FAILED" column which needs to be ignored.
*/
double** load_output (int num_values, int* num_types, char* file_name, char*** output_names) {
	//Open the file for reading.
	FILE* file_pointer = fopen(file_name, "r");
	//Count how many types of output there are, and fill keep track of their (possibly partial) names.
	int output_types = 0;
	//Only store the names if the call to the function has a place for it.
	bool name_store = (output_names != NULL);
	if(name_store){
		(*output_names) = new char*[MAX_NUM_FEATS];
	}
	//An index for putting characters within the output names array
	int o_n_index = 0;
	fscanf(file_pointer, "set,"); // skip the first column containing set number
	for(char c = '\0'; c != '\n'; fscanf(file_pointer, "%c", &c)){
		if( c == ','){
			output_types ++;
			//With a new comma, this assumes that another output type will be filled in.
			if(name_store && output_types < MAX_NUM_FEATS){
				o_n_index = 0; //Still taking feature names, so reset the string index to the beginning.
			} else{
				o_n_index = 50; //output_names must be full, so ensure that it will not be touched further.
			}
			
		} else{
			if (o_n_index < 48 && name_store && alph_num_slash(c)){
				//cout << c << " "; 
				if(o_n_index == 0){
					(*output_names)[output_types] = new char[50];
				}
				(*output_names)[output_types][o_n_index] = c;
				(*output_names)[output_types][o_n_index+1] = '\0';
				o_n_index ++;
				//cout <<"\n"<< (*output_names)[output_types][o_n_index] << "___" << (*output_names)[output_types];
			}
			
		}
	}
	//Ensure that at most MAX_NUM_FEATS features are used for the rest of the processing.
	output_types = min(output_types, MAX_NUM_FEATS);

	//Fill in the function parameter with the output_types count.
	*num_types = output_types;
	//Initialize the arrays of values for each output type, fill them 
	double** out= new double*[output_types];
	for(int i = 0; i < num_values; i++){
		fscanf(file_pointer, "%*d,"); // skip the first column containing set number
		for(int j = 0; j < output_types; j++){
			if( i == 0){
				out[j] = new double[num_values];
			}
			fscanf(file_pointer, "%lf,", out[j] + i);
			//Check for infinity or nan
			out[j][i] = check_num(out[j][i]);
		}
		if(i != num_values-1){
			fscanf(file_pointer, "%*s\n");
		}
	}
	fclose(file_pointer);
	return out;
}

/*	This function writes the sensitivity results to the file specified by file_name.
	The first line of the file contains the same names that were taken from oscillation features file(s) that was made by deterministic.
	The file contains a line for each simulation parameter, with a sensitviti
*/
void write_sensitivity (int dims, int output_types, char** output_names, double** lsa_values, char* file_name) {
	ofstream file_out;
	double x;
	file_out.open(file_name);
	if ( !file_out ) {
		cout << "  Could not open the output file.\n";
		exit ( 1 );
	}
	file_out << "parameter,";
	for(int i = 0; i < output_types; i++){
		file_out << output_names[i] << ',';
	}
	file_out.precision(30);
	for (int i = 0; i < dims; i++ ){
		file_out << "\n" << i << ',';
		for ( int j = 0; j < output_types; j++ ){
			x = (double)lsa_values[i][j];
			file_out << x << ",";
		}
	}
	file_out << '\n';
	file_out.close( );

return;
}

/*
	The rest of the code is used for piping to/from the simulation.
*/

/*	This funciton takes care of running the simulation by forking and executing (execv)
by calling ../deterministic. The parameter sets are passed to child processes and the results of
the simulations are passed back via a read/write pipe pair for each child.
*/
void simulate_samples (int first_dim, input_params& ip, sim_set& ss) {	
    int* pipes[ip.processes];
    if(!make_pipes(ip.processes, pipes)){
    	ip.failure = copy_str("!!! Failure: could not pipe !!!\n");
    	return;
    }
    char*** child_args = make_all_args(first_dim, ip, pipes);
    
    pid_t simpids[ip.processes];
    for(int i = 0; i < ip.processes; i++){
	    	simpids[i] = fork();
    		if (simpids[i] == -1) {
    			ip.failure = copy_str("!!! Failure: could not fork !!!\n");
       		break;
        }
        //Child runs simulation.  
		if (simpids[i] == 0) {
			/*cout << "woopdedoo: "<< ip.sim_args << " ";
			for(int woopdedoo = 0; woopdedoo < ip.sim_args_num; woopdedoo++){
				cout << child_args[i][woopdedoo] << " ";
			}
			cout << endl;*/
			if (-1 == execv(ip.sim_exec, child_args[i])){
				const char* fail_prefix = "!!! Failure: could not exec ";
				ip.failure = (char*)mallocate(sizeof(char)*(strlen(fail_prefix)+strlen(ip.sim_exec) + 5 + 1));
				sprintf(ip.failure, "%s%s !!!\n", fail_prefix, ip.sim_exec);
				break;
			}
		}   	
    }  
    if(ip.failure != NULL){
    	del_args(ip.processes, ip.sim_args_num,child_args);
		del_pipes(ip.processes, pipes, true);
		return;  
    }

    // Parent gives sets and processes results. Writes params to simpipe[1], reads results from simpipe[0].
    for(int i = 0; i < ip.processes; i++){
		if(!(write_info(pipes[i][1], ss.dims, ss.sets_per_dim) && write_dim_sets(pipes[i][1], first_dim + i, ip.nominal, ss))){
			ip.failure = copy_str("!!! Failure: could not write to pipe !!!");
			ip.failcode = pipes[i][1];
			break;
		}
	}
	if(ip.failure != NULL){
    	del_args(ip.processes, ip.sim_args_num,child_args);
		del_pipes(ip.processes, pipes, true);
		return;  
    }

	//Loop for waiting on children and checking their exit status.		
    for(int i = 0; i < ip.processes; i++){ 
		int status = 0; 
		waitpid(simpids[i], &status, WUNTRACED);
		check_status(status, simpids[i], &ip.failcode, &(ip.failure));
	}
	//Children are done, so we know we can delete the argument array.
	del_args(ip.processes, ip.sim_args_num,child_args);
	del_pipes(ip.processes, pipes, true); 
}

//This function is very similar to the above, except that it is designed for only writing out only one parameter set -- the nominal parameter set.
void simulate_nominal (input_params& ip) {
	int* pipes = new int[2];
	if (pipe(pipes) == -1) {
		delete[] pipes;
		ip.failure = copy_str("!!! Failure: could not pipe !!!\n");
		return;
	}
	char** child_args = (char**)mallocate(sizeof(char*)*ip.sim_args_num);
	//make_arg(0, ip.sim_args_num, ip.random_seed, pipes, ip.data_dir, (char*)"nominal", ip.simulation_args, child_args);
	make_arg(0, ip.sim_args_num, ip.random_seed, pipes, ip.data_dir, ip.nom_file, ip.simulation_args, child_args);

    pid_t simpid;
	simpid = fork();
	if (simpid == -1) {
		ip.failure = copy_str("!!! Failure: could not fork !!!\n");
		del_arg( ip.sim_args_num, child_args);
		del_pipes(1, &pipes, true);
		
	}
    //Child runs simulation.  
	if (simpid == 0) {  
		if (-1 == execv(ip.sim_exec, child_args)){
			const char* fail_prefix = "!!! Failure: with nominal set, could not exec ";
			ip.failure = (char*)mallocate(sizeof(char)*(strlen(fail_prefix)+strlen(ip.sim_exec) + 5 + 1));
			sprintf(ip.failure, "%s%s !!!\n", fail_prefix, ip.sim_exec);
			
			del_arg( ip.sim_args_num, child_args);
			del_pipes(1, &pipes, true);
			return;  
		}
	}
    // Parent gives sets and processes results. 
	if(!write_nominal(ip, pipes[1])){
		ip.failure = copy_str("!!! Failure: could not write to pipe !!!");
		ip.failcode = pipes[1];
    	del_arg( ip.sim_args_num, child_args);
		del_pipes(1, &pipes, true);
		return;  
    }

	//Waiting on child and checking their exit status.		
	int status; 
	waitpid(simpid, &status, WUNTRACED);
	check_status(status, simpid, &ip.failcode, &(ip.failure));
	//Children are done, so we know we can delete the argument array.
	del_arg( ip.sim_args_num, child_args);
	del_pipes(1, &pipes, true);
	return;
}

/*	This function establishes a communication pipe from the parent to each simulation child for the passing of parameter sets and results.
	The communication is handled by writing to file descriptors whose values are simply integers.
	The array pipes is two-dimensional because it contains an array that contains a read-end and a write-end file descriptor for each child process, e.g. pipes = { {child_1_read, child_1_write}, {child_2_read, child_2_write} } */
bool make_pipes (int processes, int** pipes) {
	for(int i = 0; i < processes; i++){
		pipes[i] = new int[2];
		if (pipe(pipes[i]) == -1) {
			del_pipes(i+1, pipes, true);
			return false;
		}
	}
	return true;
}
/* 	Closes the reading end of each pipe and deletes the data needed to store them.
	Child processes (../deterministic) are responsible for closing the writing end, but this will try to close them in case of child failure.*/
void del_pipes (int processes, int** pipes, bool close_write) {
	for(int i = 0; i < processes && pipes[i] != NULL; i++){
		close(pipes[i][0]);
		if(close_write && fcntl(pipes[i][1], F_GETFD ) != -1) close(pipes[i][1]);
		delete[] pipes[i];
	}
} 

/*	mallocates an array of char** that are needed for passing arguments to the execv call for each child simulation. 
	It fills in the appropriate argument space with the file descriptor of the pipe the children will read/write with.
	make_arg is just a sub-function that handles the work for one argument. */
char*** make_all_args (int first_dim, input_params& ip,int** pipes) {
	char*** child_args = (char***)mallocate(sizeof(char**)*ip.processes);
	for(int i = 0; i < ip.processes; i++){
		child_args[i] = (char**)mallocate(sizeof(char*)*ip.sim_args_num);
		make_arg(first_dim+i, ip.sim_args_num, ip.random_seed, pipes[i], ip.data_dir, ip.dim_file, ip.simulation_args, child_args[i]);
	}
	return child_args;
}
void make_arg (int dim_num, int sim_args_num, int seed, int* pipes, char* dir_name, char* dim_name, char** simulation_args, char** destination) {
	int pipe_loc = 0;
	int strlen_num;	
	for(int j = 0; j < sim_args_num; j++){
		if(j == 2 || j == 4){
			strlen_num = len_num( pipes[pipe_loc] );
			destination[j]= (char*)mallocate(sizeof(char)*(strlen_num+1));
			sprintf(destination[j], "%d", pipes[pipe_loc]);
			pipe_loc++;
		} else if(j == 6){
			destination[j] = make_name(dir_name, dim_name, dim_num);  
		} else if(j== 8){
			strlen_num = len_num( seed );
			destination[j]= (char*)mallocate(sizeof(char)*(strlen_num+1));
			sprintf(destination[j], "%d", seed);
		}else if (simulation_args[j] == NULL){
			destination[j] = NULL;
		} else{
			destination[j] = copy_str((const char*) simulation_args[j]);
		}
	}
}

/* This mallocates a string that can fit a full directory path, file name, and an integer for uniqe identification of the file. It then sprintf's to fill in the correct characters.*/
char* make_name (char* dir, char* file, int num) {
	int num_len = 0;
	num = abs(num);
	if(num > 0){
		num_len = len_num(num);
	} else {
		num_len = 1;
	} 
	char* name = (char*)mallocate(sizeof(char)*(strlen(dir) + 1 + strlen(file) + num_len + 1));
	sprintf(name, "%s/%s%d", dir, file, num); 
	return name;
}

/*	Deleting the arguments that were used.*/
void del_args (int processes, int sim_args_num, char*** child_args) {
	for(int i = 0; i < processes; i++){
		del_arg(sim_args_num, child_args[i]);
	}
	mfree(child_args);
}
void del_arg(int sim_args_num, char** arg){
	for(int j = 0; j < sim_args_num; j++){
		if(arg[j] != NULL)
			mfree(arg[j]);	
		}
	mfree(arg);
}

/*	Determines how many parameter sets shoudl be passed to each child by distributing them evenly. If the
	number of children does not divide the number of sets, the remainder r is distributed among the first r children.
*/
void segs_per_sim (int segments, int processes, int* distribution) {
	int remainder = segments % processes;
	for(int i = 0; i < processes; i++){
		distribution[i] = (remainder > 0 ? segments/processes + 1 : segments/processes);
		remainder--; 
	}
}

/*	This function is used to check the status of a child process that has been waited on (i.e. it has finished). 
	It returns true if the child exited normally/successfully, in which case the string failure and integer failcode are not changed.
	It returns false if there was an error with the child process, in which case it allocates a message for failure and assigns failcode to be an appropriate status, or the pid of the failed child. 
	The "#ifdef WCOREDUMP" is necessary to check whether the OS running the program has an implementation for checking for core dumps.
*/
bool check_status (int status, int simpid, int* failcode, char** failure) {
	if(WIFEXITED(status) && WEXITSTATUS(status) != 6){
		cout << "Child (" << simpid << ") exited properly with status: " << WEXITSTATUS(status) << "\n";
		return true;	
	} else{
		if(WIFSIGNALED(status)){
			*failcode = WTERMSIG(status);
			#ifdef WCOREDUMP
			if(WCOREDUMP(status)){
				*failure = copy_str("!!! Failure: child experienced core dump !!!");
			} else{
				*failure = copy_str("!!! Failure: child received signal stop !!!");
			}
			#else
				*failure = copy_str("!!! Failure: child received some kind of signal. Exact status is uncertain because your OS does not support core dump reporting.\n");
			#endif
		} else if(WIFSTOPPED(status)){
			*failure = copy_str("!!! Failure: child stopped by delivery of a signal !!!");
			*failcode = WSTOPSIG(status);
		} else{
			*failure = copy_str("!!! Failure: child process did not exit properly !!!");
			*failcode = simpid;
		}
	}
	return false;
}

/*	The following functions are used for making write() calls to the parent-child communication pipes. 
	Each returns a boolean that is true iff the write was successful.
*/

/*	This function is necessary for writing to the pipe specified by fd how many parameters the simulation should use and how many sets are going to be sent.
	This information is assumed to come in exactly this format (two successive integers in a pipe) by sogen-deterministic/deterministic.
*/
bool write_info (int fd, int dims, int sets_per_dim) {
	//Send the number of parameters that are used for each simulation .
	char* int_str = (char*)mallocate(sizeof(int));
	memcpy(int_str, &dims, sizeof(int));
	if(sizeof(int) != write(fd, int_str, sizeof(int)) ){
		mfree(int_str);
		return false;
 	}
	//Send number of sets that will be sent.
	memcpy(int_str,  &sets_per_dim, sizeof(int)); 
	if (sizeof(int) != write(fd, int_str, sizeof(int)) ){
		mfree(int_str);
		return false;
	} 
	mfree(int_str);
	return true;
}

/*	After the necessary information has been sent to the simulation program, this function writes a byte stream to the pipe (fd) that contains the double values to use as simulation parameters.
	The format in which the simulations reads and stores these values is determined by the integers it received from write_info() and the structure of the simulation program itself.
*/
bool write_dim_sets (int fd, int dim, double* nominal, sim_set& ss) {
	double nom_hold = nominal[dim];
	double* inserts = ss.dim_sets[dim];
	bool good_write = true;
	for(int i = 0; i < ss.sets_per_dim; i++){
		nominal[dim] = inserts[i];
		/*cout << "\n#Dimension " << dim << " Gets sets: \n";
		for(int check = 0; check < ss.dims; check++){
			cout << nominal[check] << ", ";
		}
		cout << "\n";*/
		good_write = (int)sizeof(double)*ss.dims == write(fd, nominal, sizeof(double)*ss.dims);
		if(!good_write) break;
	}
	nominal[dim] = nom_hold;
	return good_write;
}

/*	This function wraps write_info() and write_dim_sets() together for the simple case of writing only the nominal parameter set. 
*/
bool write_nominal (input_params& ip, int fd) {
	if(!write_info(fd, ip.dims, 1)) return false;
	return ((int)sizeof(double)*ip.dims == write(fd, ip.nominal, sizeof(double)*ip.dims) );
}

