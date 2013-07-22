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
		double** lsa_values = LSA_all_dims(ip, ss);
		del_double_2d(ip.dims, lsa_values); //This probably doesn't need to be returned, but it seemed potentially useful at the time... fix when this is clear
	
		if(ip.failure != NULL){
			usage(ip.failure, ip.failcode);
			break;
		}
	}
	cout << "\n ~ Exiting ~ \n";
	return 0;
}

/*	This function does exactly what its name implies. 
	The sim_set struct handles the work of deciding how to parameter sets should be passed, and file_io takes care of the execution and parallelization. See init.hpp & file_io.cpp
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
	It then normalizes the sensitivities based on their fraction of the total LSA_all_dims of the system.
	This also makes the calls to write out the information to appropriate files. See file_io.cpp
*/
double** LSA_all_dims(input_params& ip, sim_set& ss){
	//First, load the output for the nominal set against which other values will be compared. This call also handles counting the number of output features and holding on to the output features names.
	int num_dependent = -1;
	char* file_name = make_name(ip.data_dir, (char*)"nominal", 0);
	char** output_names[1];
	double** nominal_output = load_output(1, &num_dependent, file_name, output_names);
	free(file_name);
	
	//Based on the above count (num_dependent), calculate the sensitivities of each output for each dimension.
	double** dim_output;
	double** lsa = new double*[ip.dims];
	int i = 0;
	for(; i < ip.dims; i++){
		// Get output for this particular dimension
		char* file_name = make_name(ip.data_dir, ip.dim_file, i);
		dim_output = load_output(ss.sets_per_dim, &num_dependent,file_name, NULL);
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
	cout << "SKIP: " << ip.line_skip - 1" << endl;
	//Write out the sensitivity and normalized sensitivity to the correct directory/files
	file_name = make_name(ip.sense_dir, ip.sense_file, ip.line_skip - 1);
	write_sensitivity(ip.dims, num_dependent, output_names[0], lsa, file_name);
	free(file_name);
	
	//This call modifies lsa in place, so the array that is returned is the normalized sensitivities.
	normalize(ip.dims, num_dependent, lsa);
	file_name = make_name(ip.sense_dir, ip.norm_file, ip.line_skip - 1);
	write_sensitivity(ip.dims, num_dependent, output_names[0], lsa, file_name);
	free(file_name);
	
	//Delete the nominal data and ouput names.
	del_double_2d(num_dependent, nominal_output);
	del_char_2d(num_dependent, output_names[0]);
	return lsa;
}

/*	Handles the call to the finite difference library which is simple to use.
	In the case that the finite difference fills round_error with a value greater than the parameter perturbation size, this prints out a message but does not halt the program.
	See finite_difference.cpp & .hpp 
*/
double* fin_dif_one_dim(int accuracy, int num_dependent, double independent_step, double** dependent_values){
	double round_error = 0;
	double* fin_dif = new double[num_dependent];
	for(int i = 0; i < num_dependent; i++){
		fdy_fdx( accuracy, independent_step, dependent_values[i], fin_dif + i, &round_error);
		if(round_error >= independent_step){
			cout << "\tBad round error ("<< round_error << ") for output: " << i << "\n";
		}
	}
	
	return fin_dif;
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

/*	Calling this function performs a normalization by taking the sum of lsa values accross each parameter then then divides individual parameter sensitivity values by the sum and multiplies by 100 to give a percentage of total sensitivity.
	The input double array is modified in place.
*/
void normalize(int dims, int num_dependent, double** lsa_values){
	//cout << "norm\n";
	double sum = 1;
	volatile double val;
	for( int i = 0; i < num_dependent; i++){
		sum = 0;
		for( int j = 0; j < dims; j++){
			val = abs(lsa_values[j][i]);
			if(val == val){
				sum += val;
			} 
			
		}
		for( int j = 0; j < dims; j++){
			if(sum != 0 && sum != INFINITY){
				lsa_values[j][i] = abs(lsa_values[j][i] *(double)100 ) / sum;
			}
		}
	}
}
/*
If Y is the output function (amplitude, period, etc), p_ j is the j’th parameter, and p’ is the nominal parameter set, then non-dimensional LSA_all_dims S_ j can be evaluated by:
	S_ j = (p’_ j/Y(p’)) * (∆Y(p’)/∆p_j)

Here, ∆ refers to taking the delta of a partial derivative, but is approximate because we are evaluating the output at finitely many points.

To normalize the sensitivities across the parameter set, for m parameters, the following gives a quantitative measure of ranking and allows comparisons over time (Taylor et al):

	N_ j = (S_ j) / (m j =1|S_ j|)

Both of the above can be applied to time-dependent output. The LSA_all_dims at a particular time point will be dependent on the evaluation of the output function at that time point. 

The sloppy-stiff method allows us to see the change when multiple parameters are varied. If the function Y has multiple outputs, let Y_i be the i’th output. Let V be a vector of parameter values (like a bunch of p’s from above) and V’ the nominal parameter set. Then the cost function X2 when there are n output variables is evaluated by:
X2(V) = n i =1[ (w_i / 2T) ∫0T [ (Y_i(V, t) – Y_i(V’,t))/q_i]2 dt ]

where w and q are weighting terms; w might be used to de/emphasize the influence of particular output; q might be the maximum over the time period to normalize values to a range of zero to one.  Note that for an output function with only one output that is not time-dependent, the evaluation reduces to:

	X2(V) = w * [ ( Y(V) – Y(V’) ) / q ]2 
 
We may calculate the influence of different parameters on period with this equation, because period is a single output we do not expect to change. For the traveling wave extension we will probably evaluate it at discrete times/levels of Hes6, but we could look at the time average for a simulation as well.

	Once the method of evaluation is clear, performing LSA on large parameter sets can create global LSA_all_dims values. The evaluation is done by using each parameter set as the nominal value (V’) and calculating the LSA_all_dims when varying a particular parameter/subset of parameters. Compiling the results can be done in numerous ways, and we have to decide which is most representative of our system. Averaging the LSA_all_dims with respect to a particular parameter is simplest, but not very representative. Figure 5b from Taylor et al. provides a good demonstration for single-parameter LSA_all_dims: each parameter has its own curve plotted on a Number of points (parameter values) vs. Normalized LSA_all_dims graph. A figure like this would be a good inclusion in our analysis. 


*/



