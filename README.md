Sensitivity Analysis
====================

A program for evaluting the sensitvity of Zebrafish Somitogenesis simulations to input parameters.

Table of contents
-----------------

0: Compatibility and system requirements
****************************************
	0.0: Sensitivity progam requirements

	0.1: Simulation requirements

1: Compilation
**************
	1.0: Compiling with and without SCons

	1.1: Compilation options

2: Running Sensitivity Analysis
*******************************
	2.0: Overview of Local Sensitvity Analysis

	2.1: Command-line arguments

	2.2: Calling the program -- example 

3: Creating figures
*******************
	3.0: Scripts

4: Authorship and licensing
***************************
	4.0: GNU GPL

0: Compatibility and system requirements
****************************************

0.0. Sensitivity progam requirements
***********************************
This program, s\_a, has been designed for Unix based machines. It has been tested on Fedora 18 with GCC version 4.7.2 and OSX 10.7 (Lion) with GCC 4.2.1 using 64-bit processors.

The dependencies of s\_a are entirely standard C/C++ libraries. The default behavior makes numerous assumpitions about the machine it is running on, however these assumptions can be easily overwritten through appropriate use of commandline arguments.

0.1. Simulation	requirements
*****************************
This package can be compiled independntly. However, to run the program there must be a valid executible file to be used for collecting simulation data (not included). 
The format of how the external program accepts input and writes output files is inherently assumed by s\_a to match the format of sogen-deterministic/deterministic.

To modify the manner in which s\_a sends data to the external executable, the functions simulate\_samples() and simulate\_nominal() in io.cpp would need to be customized.

To modify the manner in which s\_a reads simulation results, the function load\_output() in io.cpp would need to be customized.

For more information on this package's I/O behavior and communication with the simulation executable, see the README.md in sogen-deterministic.

1.0. Compilation
----------------

0.0. Compiling with and without SCons
*************************************
To compile an application in its default configuration, open a terminal window and navigate to the package's root directory. If SConstruct is installed on the machine, simply enter 'scons' to compile the source.

If SCons cannot be installed on the machine, instead make the appriprate call to the g++ compiler:

	'g++ -O2 -Wall -o s\_a analysis.cpp init.cpp io.cpp finite_differences.cpp'
	
1.1. Compilation options
************************
All applications come with at least three compilation options, 'profile', 'debug', and 'memtrack'. By entering 'scons profile=1', 'scons debug=1', or 'scons memtrack=1', the application is compiled with compile and link flags designed for profiling, debugging, and memory tracking, respectively. Profiling adds the '-pg' compile and link flags, which adds extra code that enables gprof profiling analysis. Debugging adds the '-g' compile flag, which adds extra code that enables GDB debugging. Memory tracking adds the '-D MEMTRACK' compile flag, which adds a custom macro indicating the program should track its heap memory allocation. 

For more information on these options, see "Debugging, profiling, and memory tracking" in 'sogen-deterministic/README.md'.

2. Running Sensitivity Analysis
-------------------------------

2.0. Overview of Local Sensitivity Analysis
*******************************************
The goal of Local Sensitivity Analysis is to quantify the degree to which a simulation is influenced by an input parameter. 

If Y is the output function (amplitude, period, etc), p\_j is the j’th parameter, and p’ is the nominal parameter set, then non-dimensional sensitivity S\_j can be evaluated by:

	S_ j = (p’\_j/Y(p’)) * (∆Y(p’)/∆p\_j)

Here, ∆ refers to taking the delta of a partial derivative, but is approximate because we are evaluating the output at finitely many points.

To normalize the sensitivities across the parameter set, for m parameters, the following gives a quantitative measure of ranking:

	N\_j = (S\_j) / (sum\_1:m{|S\_j|})

2.1. Overview of the program
****************************
This program accomplishes the above calculations through the following steps:

0. Read in the nominal parameter set from an input file.
1. Based input values for the max percentage by which to perturb values and the number of perturbed points to simulate, calculate the (decimal) amount by which to perturb each parameter.
2. For each parameter:
	0. Create an array of values that can hold all points between \[nominal value - (nominal value * max percentage) \] and \[nominal value + (nominal value * max percentage) \].
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

2.2. Command-line arguments
***************************
The following arguments may be passed when calling the sensitivity program:

	-n, --nominal-file	 [filename]	 : the relative name of the file from which the nominal parameter set should be read, default=nominal.params.

	-d, --sense-dir			[filename]	 : the relative name of the directory to which the sensitivity results will be stored, default=sensitivities.

	-D, --data-dir			 [filename]	 : the relative name of the directory to which the raw simulation data will be stored, default=sim-data. WARNING: IF RUNNING MULTIPLE INSTANCES OF THIS PROGRAM (e.g. on cluster) EACH MUST HAVE A UNIQUE DATA DIRECTORY TO AVOID CONFLICT.

	-p, --percentage		 [float]			: the maximum percentage by which nominal values will be perturbed (+/-), default=5.

	-P, --points				 [int]				: the number of data points to collect on either side (+/-) of the nominal set, default=10.

	-c, --nominal-count	[int]				: the number of nominal sets to read from the file, default=1.

	-k, --skip					 [int]				: the number of nominal sets in the file to skip over, a.k.a. the index of the line you would like to start reading from, default=0.

	-s, --random-seed		[int]				: the postivie integer value to be used as a seed in the random number generation for simulations, default is randomly generated based on system time and process id.

	-l, --processes			[int]				: the number of processes to which parameter sets can be sent for parallel data collection, default=2.

	-y, --recycle				[N/A]				: include this if the simulation output has already been generated FOR EXACTLY THE SAME FILES AND ARGUMENTS YOU ARE USING NOW, disabled by default

	-g, --generate_only	[N/A]				: include this to generate oscillations features files for perturbed parameter values without calculating sensitivity. This is the opposite of recycle. Including this command in conjunction with --recycle will cause the program to do nothing, disabled by default.

	-z, --delete-data		[N/A]				: include this to delete oscillation features data when the program exits. This will preserve sensitivity directory but remove the directory specified by -D, disabled by default.

	-q, --quiet					[N/A]				: include this to turn off printing messages to standard output, disabled by default.

	-e, --exec					 [path]			 : if included, the simulations are run by executing the program specified by path. The path argument should be the full path, but the default uses the relative path: "../sogen-deterministic/deterministic".

	-a, --sim-args			 [args]			 : if included, any argument after this will be passed to the simulation program. If -h is one of these arguments, the simulation help will be printed and the program will not run.

	-h, --help					 [N/A]				: print out this help information.

2.3. Calling the program -- example
*********************************** 
For example, the following may be a valid call to program:

	"./s\_a -c 2 -k 4 --processes 6 --percentage 100 -P 10 --random-seed 112358 -n ~/sensitivity-analysis/nominal.params -d	~/sensitivity-analysis/sensitivity\_data -D	~/sensitivity-analysis/simulation\_data -e ~/sogen-deterministic/deterministic --sim-args -u ~/sogen-deterministic/input.perturb"

where the short and long names may be interchanged with their long/short counterparts.

Some notes about how this could go wrong:
	0. This example assumes the file system supports the "~/" path prefix -- for safety, it may be necessary to use full paths for file and directory arguments. This is partiuclarly encouraged when running on a cluster.
	1. Becuase '-c 2' is included, the program will look for two nominal parameter sets to run. If the file specified by '-n' does not contain two parameter sets, the program will exit with an error message.
	2. The option '-k 4' specifies that the fourth line of the file should contain the first nominal parameter to use -- if there are less than four lines in the file the program will exit with an error message.
	3. The option '--processes 6' states that six instances of the simulation program (not including s\_a itself) may be run simulatneously. Each instance will be given the perturbed parameter sets for a single perturbed parameter. 
		 If your system has >= 6 processors, these simulations should run simultaneously. However, even if fewer processors exist the program will not fail, there will just be less effective parallelization.
	4. Running more processes requires more system memory, so it is possible that, even if the quantity specified by '--processes' is less than the number of system processors, system memory may create a bottleneck. Again, the program should not fail, but it will have less effective parallelization.
	5. If any of the files specified do not exist, an appropriate error message will be returned. In such a case, re-check the path names and consider using full paths.

For information on the arguments that can be passed to the the simulation program (if using sogen-deterministic/deterministic) please see 'sogen-deterministic/README.md' or, if you have already compiled that simulation program, navigate to the sogen-deterministic package directory and run
	
	"./deterministic -h".

3. Creating figures
-------------------

4. Authorship and licensing
---------------------------
Licensing:

This is a Local Sensitivity Analysis progam, designed for use with the Deterministic simulator for zebrafish segmentation.

Copyright (C) 2013 Ahmet Ay, Jack Holland, Adriana Sperlea, Sebastian Sangervasi

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.	If not, see <http://www.gnu.org/licenses/>.

