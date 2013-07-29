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

/*
Summary of sensitivity analysis:

If Y is the output function (amplitude, period, etc), p_ j is the j’th parameter, and p’ is the nominal parameter set, then non-dimensional LSA_all_dims S_ j can be evaluated by:
	S_ j = (p’_ j/Y(p’)) * (dY(p’)/dp_j)

Here, d refers to taking the delta of a partial derivative, but is approximate because we are evaluating the output at finitely many points.

To normalize the sensitivities across the parameter set, for m parameters, the following gives a quantitative measure of ranking and allows comparisons over time (Taylor et al):

	N_ j = (S_ j) / (m j =1|S_ j|)

*/

#include "init.hpp"
#include "analysis.hpp"
#include "io.hpp"
#include "finite_difference.hpp"

using namespace std;

/*	The main() function does standard c++ main things -- it calls functions to initialze parameters based on commandline arguments, then distrbiutes the work to functions that perform the gathering of data and analysis.
*/
int main(int argc, char** argv){
	//Setup the parameter struct based on arguments. See init.cpp & init.hpp
	input_params ip;
	accept_params(argc, argv, ip);
	
	//Loop for processign multiple nominal parameter sets. Each step of the loop will do all of the work based on one nominal parameter set, then increment ip.line_skip which will cause proceeding steps of the loop to read other nominal sets from the input file.
	for(int which_nominal = 0; which_nominal < ip.num_nominal;  which_nominal ++){
		//Read in the nominal parameter set from file.
		read_nominal(ip);
		//If ip.nominal is set to NULL after the read_nominal() call, this function exits.
		if(ip.nominal == NULL) usage("Could not read nominal parameter set.", which_nominal); 

		//Initializes the struct that holds sets that will be simulated and fills it in with the appropriate values.
		sim_set ss(ip);
	
		//Send out the sets that need to be simulated to get data stored in files. Recycle checks to see whether the user indicated that the data has already been generated and, if so, assumes it can read the necessary files. 
		//The recycle option is prone to failure if commandline arguments are inconsistent with previous runs or if num_nominal is greater than 1. (There is a warning about this in the usage help.) 
		if(!ip.recycle){
			cout << "\n ~ Set: " << which_nominal << " -- Generating data ~ \n";
			generate_data(ip, ss);
		}
		//Ready to calculate the sensitivity. The LSA_all_dims() function does all of the work.
		cout << "\n ~ Set: " << which_nominal << " -- Calculating sensitivity ~ \n"; 
		LSA_all_dims(ip, ss);
		
		if(ip.failure != NULL){
			usage(ip.failure, ip.failcode);
			break;
		}
	}
	cout << "\n ~ Exiting ~ \n";
	return 0;
}

/*	This function does exactly what its name implies. 
	The sim_set struct handles the work of how perterbations of parameter sets should be stored,
	the simulate_samples() function in file_io takes care of the execution and parallelization. See init.hpp & io.cpp
*/
void generate_data(input_params& ip, sim_set& ss){
	//Run the simulation on the nominal set 
	simulate_nominal(ip);
	
	//Dispatch the sets for perturbations of each dimension to the simulation program.
	int first_dim = 0;
	int proc = ip.processes; //Temporarily holds on to the number of processes set for the input params so that the struct can have its value modified.
	for(; first_dim < ip.dims; first_dim += proc){
		if( (ip.dims - first_dim) < proc ){
			ip.processes = ip.dims - first_dim;
		}
		simulate_samples(first_dim, ip, ss);
		if(ip.failure != NULL) break;
	}
	ip.processes = proc;
}

/*	This function calculates the local LSA_all_dims around the the nominal parameter set with respect to each parameter. 
	It then normalizes the sensitivities of each feature to each parameter based on the parameter's fraction of the total sensitivity from all parameters. (See the normalize function)
	This also makes the calls to write out the information to appropriate files. See io.cpp
*/
void LSA_all_dims(input_params& ip, sim_set& ss){
	//First, load the output for the nominal set against which other values will be compared. This call also handles counting the number of output features and holding on to the output features names.
	int num_dependent;
	char* file_name = make_name(ip.data_dir, (char*)"nominal", 0); //Make name just mallocs a string based on a directory+filename+integer combination.
	char*** output_names = new char**[1]; //Holds a pointer to an array of strings, each of which is the name of an output feature.
	double** nominal_output = load_output(1, &num_dependent, file_name, output_names); //Load output puts the data created by generate_data into a double[j][i] where j is the index of an output feature and i is the index of the value of that feature at a particular perturbation.
	unmake_file(file_name, ip.delete_data); //Deletes a the features file if ip.delete_data is true.
	free(file_name);
	//Based on the above count (num_dependent), calculate the sensitivities of each output for each dimension.
	double** dim_output;
	double** lsa = new double*[ip.dims];
	int i = 0;
	for(; i < ip.dims; i++){
		// Get output for this particular dimension
		file_name = make_name(ip.data_dir, ip.dim_file, i);
		dim_output = load_output(ss.sets_per_dim, &num_dependent,file_name, NULL);
		unmake_file(file_name, ip.delete_data);
		free(file_name);
		// Fills LSA array with derivative values
		cout << "Parameter: " << i << "\n"; 
		lsa[i] = fin_dif_one_dim(ss.sets_per_dim, num_dependent, (ip.nominal[i] * ss.step_per_set), dim_output);
		// Scale each sensitivity value to remove dimensionalization
		for (int j = 0; j < num_dependent; j++){
			lsa[i][j] = non_dim_sense(ip.nominal[i], nominal_output[j][0], lsa[i][j]); 
		}
		del_double_2d(num_dependent, dim_output);
	}
	//Write out the sensitivity and normalized sensitivity to the correct directory/files
	file_name = make_name(ip.sense_dir, ip.sense_file, ip.line_skip - 1);
	write_sensitivity(ip.dims, num_dependent, output_names[0], lsa, file_name);
	free(file_name);
	
	//This call modifies lsa in place, so after the call to normalize(), lsa contains the normalized sensitivities.
	normalize(ip.dims, num_dependent, lsa);
	file_name = make_name(ip.sense_dir, ip.norm_file, ip.line_skip - 1);
	write_sensitivity(ip.dims, num_dependent, output_names[0], lsa, file_name);
	free(file_name);
	
	//Delete the nominal data and ouput names.
	del_double_2d(num_dependent, nominal_output);
	del_char_2d(num_dependent, output_names[0]);
	delete[] output_names;
	del_double_2d(ip.dims, lsa);
	return;
}

/*	Handles the call to the finite difference library which is simple to use.
	In the case that the finite difference fills round_error with a value greater than the parameter perturbation size, this prints out a message but does not halt the program.
	See finite_difference.cpp & .hpp
	Note that the int accuracy is/should be equal to the length of each array within dependent_values -- i.e. it is equal to how many pertubation points were made by generate_data. 
*/
double* fin_dif_one_dim(int accuracy, int num_dependent, double independent_step, double** dependent_values){
	double round_error = 0;
	double* fin_dif = new double[num_dependent];
	for(int i = 0; i < num_dependent; i++){
		/*cout << "Output: " << i << "  with delta x = " << independent_step << "\n output values: ";
		for(int check = 0; check < accuracy; check++){
			cout << dependent_values[i][check] << " , ";
		}
		cout << endl;*/
		fdy_fdx( accuracy, independent_step, dependent_values[i], fin_dif + i, &round_error);
		if(round_error >= independent_step){
			cout << "\tBad round error ("<< round_error << ") for output: " << i << "\n";
		}
	}
	
	return fin_dif;
}


/*	Calling this function performs a normalization by taking the sum of lsa values accross each parameter then then divides individual parameter sensitivity values by the sum and multiplies by 100 to give a percentage of total sensitivity.
	The input double array is modified in place.
*/
void normalize(int dims, int num_dependent, double** lsa_values){
	double sum = 1;
	volatile double val;
	for( int i = 0; i < num_dependent; i++){
		sum = 0;
		for( int j = 0; j < dims; j++){
			val = abs(lsa_values[j][i]);
			if(isinf(val)){
				lsa_values[j][i] = 500;
				val = 500;
			}
			if(isnan(val) == 0){
				sum += val;
			} 
		}
		for( int j = 0; j < dims; j++){
			if(sum != 0 && isinf(sum) == 0){
				lsa_values[j][i] = abs(lsa_values[j][i] *(double)100 ) / sum;
			}
		}
	}
}

/*	Methods for deleting arrays.
*/
void del_double_2d(int rows, double** victim){
	for(int i = 0; i < rows; i++){
		delete[] victim[i];
	}
	delete[] victim;
}
void del_char_2d(int rows, char** victim){
	for(int i = 0; i < rows; i++){
		delete[] victim[i];
	}
	delete[] victim;
}


