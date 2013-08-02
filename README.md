Sensitivity Analysis
====================

A program for evaluting the sensitvity of Zebrafish Somitogenesis simulations to input parameters.

Table of contents
-----------------

0: Compatibility and system requirements

* 0.0: Sensitivity progam requirements

* 0.1: Simulation requirements

1: Compilation

* 1.0: Compiling with and without SCons

* 1.1: Compilation options

2: Running Sensitivity Analysis

* 2.0: Overview of Local Sensitvity Analysis

* 2.1: Command-line arguments

* 2.2: Calling the program -- example 

3: Creating figures

* 3.0: Use sogen-scripts/plot-sensitivity.py
	
* 3.1: Overview of plot-sensitivity.py
	
* 3.2: Command-line arguments
	
* 3.3: Running the script -- example

4: Modifying the code

* 4.0: Adding command-line arguments

* 4.1: Modifying the simulation program

5: Authorship and licensing

* 5.0: Authors
	
* 5.1: GNU GPL

0: Compatibility and system requirements
----------------------------------------
************************************
**0.0: Sensitivity progam requirements**

This program, sensitivity, has been designed for Unix based machines. It has been tested on Fedora 18 with GCC version 4.7.2 and OSX 10.7 (Lion) with GCC 4.2.1 using 64-bit processors.

The dependencies of sensitivity are entirely standard C/C++ libraries. The default behavior makes numerous assumpitions about the machine it is running on, however these assumptions can be easily overwritten through appropriate use of commandline arguments.

*****************************
**0.1: Simulation	requirements**

This package can be compiled independntly. However, to run the program there must be a valid executible file to be used for collecting simulation data (not included). The format of how the external program accepts input and writes output files is inherently assumed by sensitivity to match the format of sogen-deterministic/simulation.

To modify the manner in which sensitivity sends data to the external executable, the functions simulate\_samples() and simulate\_nominal() in io.cpp would need to be customized.

To modify the manner in which sensitivity reads simulation results, the function load\_output() in io.cpp would need to be customized.

For more information on this package's I/O behavior and communication with the simulation executable, see the README.md in sogen-deterministic.

1.0. Compilation
----------------

***************************************
**0.0. Compiling with and without SCons**

To compile an application in its default configuration, open a terminal window and navigate to the package's root directory. If SConstruct is installed on the machine, simply enter 'scons' to compile the source.

If SCons cannot be installed on the machine, instead make the appriprate call to the g++ compiler:

	g++ -O2 -Wall -o sensitivity analysis.cpp init.cpp io.cpp finite_differences.cpp

**************************	
**1.1: Compilation options**

All applications come with at least three compilation options, 'profile', 'debug', and 'memtrack'. By entering 'scons profile=1', 'scons debug=1', or 'scons memtrack=1', the application is compiled with compile and link flags designed for profiling, debugging, and memory tracking, respectively. Profiling adds the '-pg' compile and link flags, which adds extra code that enables gprof profiling analysis. Debugging adds the '-g' compile flag, which adds extra code that enables GDB debugging. Memory tracking adds the '-D MEMTRACK' compile flag, which adds a custom macro indicating the program should track its heap memory allocation. 

For more information on these options, see "Debugging, profiling, and memory tracking" in 'sogen-deterministic/README.md'.

2: Running Sensitivity Analysis
-------------------------------
*********************************************
**2.0: Overview of Local Sensitivity Analysis**

The goal of Local Sensitivity Analysis is to quantify the degree to which a simulation is influenced by an input parameter. 

If Y is the output function (amplitude, period, etc), p\_j is the j’th parameter, and p’ is the nominal parameter set, then non-dimensional sensitivity S\_j can be evaluated by:

	S_j = (p’_j/Y(p’)) * (∆Y(p’)/∆p_j)

Here, ∆ refers to taking the delta of a partial derivative, but is approximate because we are evaluating the output at finitely many points.

To normalize the sensitivities across the parameter set, for m parameters, the following gives a quantitative measure of ranking:

	N_j = (S_j) / (sum_1:m{|S_j|})

******************************
**2.1: Overview of the program**

This program accomplishes the above calculations through the following steps:

0. Read in the nominal parameter set from an input file.
1. Based input values for the max percentage by which to perturb values and the number of perturbed points to simulate, calculate the (decimal) amount by which to perturb each parameter.
2. For each parameter:
	0. Create an array of values that can hold all points between [nominal value - (nominal value * max percentage) \] and [nominal value + (nominal value * max percentage) \].
	1. Based on the nominal paramter value, fill in this array.
3. Generate simulation data (e.g. oscillation features) by piping parameter sets to instances of the simulation program:
	0. Send the nominal (unperturbed) set to get Y(p') as above.
	1. For each individual parameter, send the nominal parameter set with the perturbed value inserted appropriately. This would be p_jabove.
4. To get the absolute sensitivity for each feature (i.e. output function) returned in the simulation data, perform the following for each parameter (j):
	0. Use the output values of the perturbed sets to calculate the finite difference, i.e. the derivative around the nominal parameter point (∆Y(p’)/∆p\_j above).
	1. Calculate the non-dimensionalized sensitivity (S\_j above) using the finite difference and the nominal parameter value and output ( p’\_j/Y(p’) above ).
1. To get the normalized sensitivity for each feature, perform the following for each parameter (j):
	0. Sum the non-dimensionalized sensitivity of the feature with respect to all parameters, (sum\_1:m{|S\_j|} above). 
	1. Divide each parameter's sensitivity value by that sum. Optionally, multiply by 100 to give N\_j as a percentage.
5. Write out these calculated values to files.

*****************************
**2.2: Command-line arguments**

The following arguments may be passed when calling the sensitivity program:

	-n, --nominal-file             [filename] : the relative name of the file from which the nominal parameter set should be read, default=nominal.params.

	-d, --sense-dir                [filename] : the relative name of the directory to which the sensitivity results will be stored, default=sensitivities.

	-D, --data-dir                 [filename] : the relative name of the directory to which the raw simulation data will be stored, default=sim-data. WARNING: IF RUNNING MULTIPLE INSTANCES OF THIS PROGRAM (e.g. on cluster) EACH MUST HAVE A UNIQUE DATA DIRECTORY TO AVOID CONFLICT.

	-p, --percentage               [float]    : the maximum percentage by which nominal values will be perturbed (+/-), default=5.

	-P, --points                   [int]      : the number of data points to collect on either side (+/-) of the nominal set, default=10.

	-c, --nominal-count	           [int]      : the number of nominal sets to read from the file, default=1.

	-k, --skip                     [int]      : the number of nominal sets in the file to skip over, a.k.a. the index of the line you would like to start reading from, default=0.

	-s, --random-seed              [int]      : the postivie integer value to be used as a seed in the random number generation for simulations, default is randomly generated based on system time and process id.

	-l, --processes                [int]      : the number of processes to which parameter sets can be sent for parallel data collection, default=2.

	-y, --recycle                  [N/A]      : include this if the simulation output has already been generated FOR EXACTLY THE SAME FILES AND ARGUMENTS YOU ARE USING NOW, disabled by default

	-g, --generate_only            [N/A]      : include this to generate oscillations features files for perturbed parameter values without calculating sensitivity. This is the opposite of recycle. Including this command in conjunction with --recycle will cause the program to do nothing, disabled by default.

	-z, --delete-data              [N/A]      : include this to delete oscillation features data when the program exits. This will preserve sensitivity directory but remove the directory specified by -D, disabled by default.

	-q, --quiet	                   [N/A]      : include this to turn off printing messages to standard output, disabled by default.

	-e, --exec                     [path]     : if included, the simulations are run by executing the program specified by path. The path argument should be the full path, but the default uses the relative path: "../sogen-deterministic/simulation".

	-a, --sim-args                 [args]     : if included, any argument after this will be passed to the simulation program. If -h is one of these arguments, the simulation help will be printed and the program will not run.

	-h, --help                     [N/A]      : print out this help information.

**********************************
**2.3: Input file format**

Sensitivity analysis requires at least one nominal parameter set as input. For strictly local analysis, this set should be one that successfully expresses the features desired for the system. For more global analysis it is recommended the nominal parameter sets be plentiful and varying -- using well-distributed parameter sampling algorithms such as Latin Hypercube Sampling is preferable. However, the nominal parameter sets should not yeild undefined behavior (e.g. infinite or non-numerical feauture values) as this is likely to render sensitivity calculations futile. Assuming the appropriate nominal parameter sets have been collected, the following describes the format in which they may be passed to the sensitivity program:

A parameter sets file consists of a list of parameter sets. Each parameter set is placed on its own line, with each parameter separated by a comma. There is not a comma after the final parameter set. Each parameter must be included and have a floating point or integral value. Blank lines and lines beginning with "#" are ignored. There must be at least one parameter set per file. There is no limit to the number of parameter sets allowed in a single file. Recall that you may specify which parameter set to start with using '-k' or '--skip' and how many sets to use with '-c' or '--nominal-count'.

The following three lines represent an example file:

```
# This is a comment
63.049748,34.955167,0,42.993889,0,44.600336,0.429514,0.453927,0,0.279120,0,0.335253,45.227915,30.430273,0,46.391646,0,22.217098,0.157011,0.338070,0,0.307667,0,0.274870,0.019982,0.000370,0,0.026413,0,0.008877,0,0.027406,0,0,0,0,0.005284,0,0,0.219352,0.032981,0,0.051730,0,0.175033,0,0.116019,0,0,0,0,0.006626,0,0,0.278811,0.269840,0,0.279624,0,0.285013,0,0.238326,0,0,0,0,0.242239,0,0,11.017495,9.139970,0,0.000000,0,7.329024,0.426847,1.210969,0,0.596138,0,12.461520,361.441083,485.384224,0,0,0,0,499.577384
63.000647,33.757737,0,43.849951,0,47.332097,0.434420,0.455262,0,0.274844,0,0.346678,43.338772,30.019011,0,54.822609,0,25.281511,0.141475,0.315663,0,0.345098,0,0.269280,0.018546,0.003612,0,0.028153,0,0.008334,0,0.025200,0,0,0,0,0.011394,0,0,0.170959,0.041615,0,0.044836,0,0.237797,0,0.248760,0,0,0,0,0.017808,0,0,0.280718,0.310334,0,0.343655,0,0.210100,0,0.233876,0,0,0,0,0.214772,0,0,10.916983,9.443056,0,0.000000,0,7.742257,0.445980,1.035695,0,0.578762,0,12.446215,231.836670,477.034572,0,0,0,0,540.815524
```

**********************************
**2.3: Simulation output file format**

This program reads results of simulations from the output files created by the simulation program. If the sensitivity program is run with the standard simulation program, this file format is already created properly and can be read by the simulation without any user modifications. The creation of this format is explicitly defined in sogen-deterministic/README.md, but a summary of what is necessary for the sensitivity program to read such a file properly is as follows:

Some requirements of the simulation output file:

1. The number of features can be arbitrary, but: 
2. The maximum number of features that can be read is set by the macro MAX_NUM_FEATS in "macros.hpp" and is 150 by default. 
3. All values must be comma-seperated with no spaces, 
4. The names line must contain a name for every feature, 
5. The last value in non-name lines must be followed by a comma, but can have any string after that before the new line, 
7. If a trailing column should be ignored it should not be given a name in the first line.

The following three lines represent an example file:

```
set,post sync wildtype,post per wildtype,post amp wildtype,post per wildtype/wt,post amp wildtype/wt,
0,0.999999999999999888977697537484,29.8797435897436045593167364132,56.0505555846975624945116578601,1,1,
1,1,30.2323076923076676791879435768,166.255079755418790909970994107,1,1,
```

**********************************
**2.4: Sensitivity output file format**

The final output of the sensitivity calculation comes in two files with the same format. Both files are stored in a directory that is by default named "SA-data/" though this can be modified with the commands '-d' or '--sense-dir'. In this directory there will be two files for every nominal set that was sent to the sensitivity program: "LSA\_n" and "normalized\_n" where "_n" refers to the n'th nominal sent. The former file contains the absolute, dimensionless sensitivity values while the latter contains the normalized sensitivities as percentages (S\_j and N\_j as described in section 2.0). 

The format of these files is consistent with the format of the simulation output file format, with the sensitivity/normalized-senstivity values in place of the original feature values. The one difference is that the first column will refer to which parameter the sensitiviy value is for. 

The following lines are an example of an absolute sensitivity output file:

```
parameter,post sync wildtype,post per wildtype,post amp wildtype,post per wildtype/wt,post amp wildtype/wt,
0,-0.00743137894005018763421421823523,0.00504250156260815669134744965163,1.00289462409731400249768284993,0,0,
1,-0.00918263623745127141595467890056,0.0275346684530147697844704168801,-0.15316488780164577709896889246,0,0,
```

The following line are an example of a normalized sensitivity output file:

```
parameter,post sync wildtype,post per wildtype,post amp wildtype,post per wildtype/wt,post amp wildtype/wt,
0,1.44836748675782578388293586613,0.422484791108310275831172475591,18.6174946777228882410781807266,0,0,
1,1.78968558545324518682662073843,2.30698563107465703936327372503,2.84331615201126997050096179009,0,0,
```

*************************************
**2.5: Calling the program -- example**
 
For example, the following may be a valid call to program:

	~/sensitivity-analysis/sensitivity -c 2 -k 4 --processes 6 --percentage 100 -P 10 --random-seed 112358 -n ~/sensitivity-analysis/nominal.params -d	~/sensitivity-analysis/sensitivity_data -D	~/sensitivity-analysis/simulation_data -e ~/sogen-deterministic/simulation --sim-args -u ~/sogen-deterministic/input.perturb

where the short and long names may be interchanged with their long/short counterparts.

Some notes about how this could go wrong:

0. This example assumes the file system supports the "~/" path prefix -- for safety, it may be necessary to use full paths for file and directory arguments. This is partiuclarly encouraged when running on a cluster.
1. Becuase '-c 2' is included, the program will look for two nominal parameter sets to run. If the file specified by '-n' does not contain two parameter sets, the program will exit with an error message.
2. The option '-k 4' specifies that the fourth line of the file should contain the first nominal parameter to use -- if there are less than four lines in the file the program will exit with an error message.
3. The option '--processes 6' states that six instances of the simulation program (not including sensitivity itself) may be run simulatneously. Each instance will be given the perturbed parameter sets for a single perturbed parameter. If your system has >= 6 processors, these simulations should run simultaneously. However, even if fewer processors exist the program will not fail, there will just be less effective parallelization.
4. Running more processes requires more system memory, so it is possible that, even if the quantity specified by '--processes' is less than the number of system processors, system memory may create a bottleneck. Again, the program should not fail, but it will have less effective parallelization.
5. If any of the files specified do not exist, an appropriate error message will be returned. In such a case, re-check the path names and consider using full paths.

For information on the arguments that can be passed to the the simulation program (if using sogen-deterministic/simulation) please see 'sogen-deterministic/README.md' or, if you have already compiled that simulation program, navigate to the sogen-deterministic package directory and run:
	
	./simulation -h

3: Creating figures
-------------------
********************************************
**3.0: Use sogen-scripts/plot-sensitivity.py**

This package by itself gathers data and calculates sensitivities but does not have a library for generating figures. 

To create figures from the data this program generates, please use the 'sogen-scripts' package. This package includes numerous python scripts necessary for displaying Zebrafish Somitogenesis simulation data.
The script that is relevent to this package is:

	sogen-scripts/plot-sensitivity.py

**************************************
**3.1: Overview of plot-sensitivity.py**

The plot-sensitivity.py python script includes two plotting methods: 1) sensitivity bar graphs, and 2) output value scatter-line plots.

1) Sensitivity Bar Graphs

These bar graphs display the sensitivity of a simulation output to each simulation parameter. The script generates these graphs by:

1. making system calls to sensitivity (repeatedly if necessary),
2. parsing the sensitivity output files,
3. averaging the sensitivity values for each parameter accross nominal parameter sets,
4. using the average to calculating the standard error for each parameter,
5. creating a bar graph for each simulation output in which the height of each bar is the average sensitivity to a particular input parameter accross nominal parameter sets and the error bars are the standard error values.

This method will create two images for each feature: a bar graph of the normalized sensitivities and a bar graph of the absolute sensitivities. These graphs will be stored in files titled "[feature name].png" and "absolute_[feature name].png", respectively. The feature names will come from the original simulation output data, with the exception that all special characters will be converted to underscores in the file name (though the full feature name will be used as the title on the graph, verbatim).

2) Scatter-Line Plots

These are plots of points connected by lines where each line is a single nominal parameter set. The script generates these plots by:

1. making system calls to sensitivity for every nominal parameter set with the '--generate-only' argument so that the sensitivity values are not calculated,
2. parsing the simulation output files, i.e. the oscillation features of the zebrafish somitogenesis deterministic simulations,
3. For every simualaiton parameter, plotting the simulation output value at successive perturbations of the parameter -- each output value is plotted as the perturbed value divided by the output vale of the nominal (unperturbed) parameter set.

These plots display the ratio between an simulation output value at a perturbed parameter value. This method will create an image for every parameter's effect on every simulation output feature. The files are titled "[index of parameter]on[index of feature]_[feature name].png". For example, the file "10on2\_posterior\_amplitude\_wildtype.png" would refer the the effect of parameter #10 on feature #2 which is named "posterior amplitude wildtype". As before, feature names come from the original simulation output with special characters (e.g. white space) replaced with underscores.

*****************************
**3.2: Command-line arguments**

The following arguments may be passed in a command-line call to the python script. Some of them are the same as for sensitivity, but most are distinct.

	-n, --nominal-file     [filename]:The file to use for getting nominal parameter sets, default=~/sensitivity-analysis/sens/sensitivity-analysis/sensitivity-analysis/nominal.params.
	
	-d, --dir              [filename]:The directory to put all plots in, default ="all-sensitivities"
	
	-D, --data-dir         [filename]:The directory in which to store raw sensitivity data, default ="sense-for-plot"
	
	-o, --output           [filename]:The name of file(s) to use for plot images, default="sensitivity"
	
	-j, --job-name         [string]  :The name that should be used for the pbs job, default="PLOT_SA"
	
	-C, --cluster-name     [string]  :The name of the cluster on which to run simulations, if any, default=None.
	
	-N, --nodes            [int]     :The number of nodes to be utilized. Runs are done locally unless -N > 1, default=1.
	
	-f, --feature          [int]     :Index of feature, if only one feature should be plotted. Omit if all features should be plotted. 
	
	-p, --percent          [int]     :Max percentage by which to perturb nominal value, default=20.
	
	-P, --points           [int]     :Number of data points to collect between nominal and nominal + (max percent * nominal), default=4. 
	
	-l, --ppn              [int]     :Processors per node, a.k.a. processes to use in parallel for the analysis.
	
	-c, --nominal-count    [int]     :The number of nominal sets to use for sensitivity calculations. If this is greater than the number availible in the -n file, there will be errors. default=1.
	
	-e, --exec             [path]    :The path of the executable for performing sensitivity analysis, default=~/sensitivity-analysis/sens/sensitivity-analysis/sensitivity-analysis/s_a
	
	-s, --sim              [path]    :The path of the executable for running simulations, default=../sogen-deterministic/simulation
	
	-g, --graph            [N/A]     :Include this if you would just like to generate graphs without running simulation. This assumes the appropriate files have already been generated based on the other command line arguments passed.
	
	-E, --elasticity       [N/A]     :Include this to plot (oscillation features)/(nominal feature value) as a scatter plot connected by lines (instead of sensitivity bar graphs). 
	
	-a, --args             [args]    :Any arguments passed beyond this argument will be passed verbatim to the sensitivity program.

	-h, --help             [N/A]     :Display this help information.
	
************************************
**3.3: Running the script -- example**

To generate bar graphs of the sensitivity of all feautures to all parameters, the following may be a valid call to the script:

	python plot-sensitivity.py -n ~/sensitivity-analysis/nominal.params -d ~/sogen-scripts/sensitivity-plots -C biomath --percent 10 -P 2 -N 5 -c 10 --ppn 6 --exec ~/sensitivity-analysis/s_a --sim ~/sogen-deterministic/simulation --args -s 112358 -a -M 1 -u ~/sogen-deterministic/input.perturb

To generate line-connected scatter plots of oscillation features for various perturbations of input parameters:

	python plot-sensitivity.py --elasticity -n ~/sensitivity-analysis/nominal.params -d ~/sogen-scripts/sensitivity-plots -C biomath -p 100 --points 10 -N 5 -c 10 --ppn 6 --exec ~/sensitivity-analysis/s_a --sim ~/sogen-deterministic/simulation --args -s 112358 -a -M 1 -u ~/sogen-deterministic/input.perturb

See the notes about calling the sensitivity analysis program for more information.


4: Modifying the code
-----------------------

If you are user who has an advanced understanding of C/C++ and the code contained in this package, we encourage you to thuroughly read this section before attempting to customize the program.

If you are a user who has an advanced understanding of C/C++ but not the code contained in this package, please reread sections 0 through 9.

If you ar a user who does not have an advanced understanding of C/C++, please open your web browser to <http://www.cplusplus.com/files/tutorial.pdf> and read pages 0 through 138.

**************************************
**4.0: Adding command-line arguments**

Parsing of command-line argument is handled entirely in 'sensitivity-analysis/source/init.cpp' in the function:

	void accept_params (int num_args, char** args, input_params& ip)
	
If you would like to add a comand-line argument to the senstivity program simply add an "else if" statement to the main conditional like so:

```
...
}else if (strcmp(option, "-?") == 0 || strcmp(option, "--your-option") == 0) {
	ensure_nonempty(option, value);
	your_variable = value;
} else ...
```

You should replace 'your-option' with the command you would like to add and '?'  with a single-character short version. Please ensure that these strings are unique, i.e. they do not conflict with any previously assigned commands. Please do not ovewrite the default commands.

You should also replace 'your\_variable' with the variable/object you would like to use to store the value passed as an argument. The most convenient method of passing this variable throughout the program is by making it an instance variable of the input\_params struct. To do this, simply open 'sensitivity-analysis/source/init.hpp' and add your instance variable to the input\_params struct. You may also want to set a default value for you variable, in which case you should add this assignment to the input_params constructor and destrctor. For example:

```
struct input_params{
	TYPE your_variable_name;
	bool sim_args;
	bool quiet;	
	...
	input_params(){
		your_variable_name = default_value;
		quiet = false;
		sim_args = false;
		...
	}
	~input_params(){
		your_destruction_method( your_variable_name );
		if(nominal != NULL) delete[] nominal;
	}
};
```

You may then reference your variable in all functions that take 'input_params& ip' as a parameter by entering:

	ip.your_variable_name


**************************************
**4.1: Modifying the simulation program**

The code has been made with as much modularity as possible. If you would like to use an alternative simulation program you have two options:

**Low-Modification Option:**

Make a simulation program executible that takes the the same command-line arguments as 'sogen-deterministic/simulation'. This program should also create the same output file format as is specified in section 4.3.2. 

While this requires you to conform to very specific bounds, if you create a simulation program in this way you will only need to call 'sensitivity-analysis/sensitivity' with the argument '-e' or '--executable' followed by your simulation program and the sensitivity program will run with the appropriate behavior.

**High-Modification Option:**

If you are willing to modify the source code of 'sensitivity-analysis/sensitivity' it is possible create custom simulation program calls and file formats by modifying 'io.cpp' and 'io.hpp'. The functions that would need to be rewritten for you own needs are as follows.

For changing how simulations are called:

* 
	void simulate_samples(int , input_params& , sim_set&  )
* 
	void simulate_nominal(input_params& );

For changing how simulation results are read from file:

* 
	double** load_output(int, int*, char*, char*** );

For changing how sensitivity calculations are written to file:

* 
	void write_sensitivity(int , int , char** , double** , char*  );
	
Please store a backup of the original package before any modifications and proceed with caution.


5: Authorship and licensing
---------------------------

**************
**5.0: Authors**

Copyright (C) 2013 Ahmet Ay, Jack Holland, Adriana Sperlea, Sebastian Sangervasi

**************
**5.1: GNU GPL**

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in the file "GNU_GPL.txt". If not, see <http://www.gnu.org/licenses/>.

