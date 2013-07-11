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


/*	Establishing a communication pipe from the parent (sampler) to each simulation child for the passing
of parameter sets and results. */
bool make_pipes(int processes, int** pipes){
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
	Child processes (../deterministic) are responsible for closing the writing end, but this will try to close them in case of child failure.
*/
void del_pipes(int processes, int** pipes, bool close_write){
	for(int i = 0; i < processes && pipes[i] != NULL; i++){
		close(pipes[i][0]);
		if(close_write && fcntl(pipes[i][1], F_GETFD ) != -1) close(pipes[i][1]);
		delete[] pipes[i];
	}
} 

/*	Mallocs an array of char** that are needed for passing arguments to the execv call for each child simulation. 
It fills in the appropriate argument space with the file descriptor of the pipe the children will read/write with.*/
char*** make_args(input_params& ip, int** pipes){
	int strlen_num;
	int pipe_loc;
	char*** child_args = (char***)malloc(sizeof(char**)*ip.processes);
	for(int i = 0; i < ip.processes; i++){
		pipe_loc = 0; 			
		child_args[i] = (char**)malloc(sizeof(char*)*ip.sim_args_num);
		for(int j = 0; j < ip.sim_args_num; j++){
			if(j == 2 || j == 4){
				strlen_num = log10(pipes[i][pipe_loc]+1)+1;
				child_args[i][j]= (char*)malloc(sizeof(char)*(strlen_num+1));
				sprintf(child_args[i][j], "%d", pipes[i][pipe_loc]);
				pipe_loc++;
			} else if (ip.simulation_args[j] == NULL){
				child_args[i][j] = NULL;
			}else{
				child_args[i][j] = strdup((const char*) ip.simulation_args[j]);
			}
		}
	}
	return child_args;
}
/*	Deleting the arguments that were used.*/
void del_args(input_params& ip, char*** child_args){
	for(int i = 0; i < ip.processes; i++){
		for(int j = 0; j < ip.sim_args_num; j++){
			if(child_args[i][j] != NULL) 
				free(child_args[i][j]);	
		}
		free(child_args[i]);	
	}
	free(child_args);
}

/*	Determines how many parameter sets shoudl be passed to each child by distributing them evenly. If the
number of children does not divide the number of sets, the remainder r is distributed among the first r children.*/
void segs_per_sim(int segments, int processes, int* distribution){
	int remainder = segments % processes;
	for(int i = 0; i < processes; i++){
		distribution[i] = (remainder > 0 ? segments/processes + 1 : segments/processes);
		remainder--; 
	}
}

/*
	This funciton takes care of running the simulation by forking and executing (execv)
by calling ../deterministic. The parameter sets are passed to child processes and the results of
the simulations are passed back via a read/write pipe pir for each child.*/
void simulate_samples(int first_dim, input_params& ip, sim_set& ss ){	
    int* pipes[ip.processes];
    if(!make_pipes(ip.processes, pipes)){
    	ip.failure = strdup("!!! Failure: could not pipe !!!\n");
    	return;
    }
    char*** child_args = make_args(ip, pipes);
    
    pid_t simpids[ip.processes];
    for(int i = 0; i < ip.processes; i++){
	    	simpids[i] = fork();
    		if (simpids[i] == -1) {
    			ip.failure = strdup("!!! Failure: could not fork !!!\n");
       		break;
        }
        //Child runs simulation.  
		if (simpids[i] == 0) {  
			if (-1 == execv(ip.sim_exec, child_args[i])){
				ip.failure = strdup("!!! Failure: could not exec deterministic !!!\n");
				break;
			}
		}   	
    }
    if(ip.failure != NULL){
    	del_args(ip,child_args);
		del_pipes(ip.processes, pipes, true);
		return;  
    }

    // Parent gives sets and processes results. Writes params to simpipe[1], reads results from simpipe[0].
    for(int i = 0; i < ip.processes; i++){
		write_info(pipes[i][1], ss);
		write_dim_sets(pipes[i][1], first_dim + i, ip.nominal, ss)
	}

	//Loop for waiting on children and checking their exit status.		
    ssize_t result; //For reading purposes.
    for(int i = 0; i < ip.processes; i++){ 
		int status = 0; 
		waitpid(simpids[i], &status, WUNTRACED);

		if(WIFEXITED(status)){
			cout << "Child (" << simpids[i] << ") exited properly with status: " << WEXITSTATUS(status) << "\n";	
		} else if (ip.failure == NULL){
			if(WIFSIGNALED(status)){
				ip.failcode = WTERMSIG(status);
				#ifdef WCOREDUMP
				if(WCOREDUMP(status)){
					ip.failure = strdup("!!! Failure: child experienced core dump !!!");
				} else{
					ip.failure = strdup("!!! Failure: child received signal stop !!!");
				}
				#else
					ip.failure = strdup("!!! Failure: child received some kind of signal. Exact status is uncertain because your OS does not support core dump reporting.\n");
				#endif
			} else if(WIFSTOPPED(status)){
				ip.failure = strdup("!!! Failure: child stopped by delivery of a signal !!!");
				ip.failcode = WSTOPSIG(status);
			} else{
				ip.failure = strdup("!!! Failure: child process did not exit properly !!!");
				ip.failcode = simpids[i];
			}
		}
	}
	//Children are done, so we know we can delete the argument array.
	del_args(ip,child_args);
	if(ip.failure != NULL){
		del_pipes(ip.processes, pipes, true);   
		return ; 
	}

	del_pipes(ip.processes, pipes, true); 
}

void write_info(int fd, sim_set& ss){
	//Send the number of parameters that are used for each simulation .
	int_str = malloc(sizeof(int));
	memcpy(int_str, &ss.dims, sizeof(int));
	write(fd, int_str, sizeof(int));
 
	//Send number of sets that will be sent.
	memcpy(int_str,  &ss.sets_per_dim, sizeof(int)); 
	write(fd, int_str, sizeof(int)); 

}

void write_dim_sets(int fd, int dim, double* nominal, sim_set& ss){
	double nom_hold = nominal[dim];
	double* inserts = ss.dim_sets[dim];
	for(int i = 0; i < num_inserts; i++){
		nominal[dim] = inserts[i];
		write(fd, nominal, sizeof(double)*ss.dims);
	}
	nominal[dim] = nom_hold;
}

