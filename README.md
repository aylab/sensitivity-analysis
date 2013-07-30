Sensitivity Analysis
====================

Repository for creating the program(s) necessary for analyzing the sensitivity of sogen-deterministic simulations to each parameter.

Table of contents
-----------------

0. Compatibility and system requirements
  0. Sensitivity progam requirements
  1. Simulation requirements
1. Compilation
  0. Compiling with and without SCons
  1. Compilation options
  
2. Running Sensitivity Analysis
  
3. Creating figures
  
4. Authorship and licensing

0. Compatibility and system requirements
----------------------------------------

0.0. Sensitivity progam requirements
***********************************
This program, s\_a, has been designed for Unix based machines. It has been tested on Fedora 18 with GCC version 4.7.2 and OSX 10.7 (Lion) with GCC 4.2.1 using 64-bit processors.
The dependencies of s\_a are entirely standard C/C++ libraries. The default behavior makes numerous assumpitions about the machine it is running on, however these assumptions can be easily overwritten through appropriate use of commandline arguments.

0.1. Simulation  requirements
***********************************
This package can be compiled independntly. However, to run the program there must be a valid executible file to be used for collecting simulation data (not included). 
The format of how the external program accepts input and writes output files is inherently assumed by s\_a to match the format of sogen-deterministic/deterministic.
To modify the manner in which s\_a sends data to the external executable, the functions simulate\_samples() and simulate\_nominal() in io.cpp would need to be customized.
To modify teh manner in which s\_a reads simulation results, the function load\_output() in io.cpp would need to be customized.

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
TBA

2. Running Sensitivity Analysis
-------------------------------

2.0. Overview of Local Sensitivity Analysis
*******************************************
The goal of Local Sensitivity Analysis is to quantify the degree to which a simulation is influenced by an input parameter. 
If Y is the output function (amplitude, period, etc), p_ j is the j’th parameter, and p’ is the nominal parameter set, then non-dimensional sensitivity S_ j can be evaluated by:
	S_ j = (p’_ j/Y(p’)) * (∆Y(p’)/∆p_j)
Here, ∆ refers to taking the delta of a partial derivative, but is approximate because we are evaluating the output at finitely many points.
To normalize the sensitivities across the parameter set, for m parameters, the following gives a quantitative measure of ranking:
	N_ j = (S_ j) / (m j =1|S_ j|)


2.1. Command-line arguments
***************************
The following arguments may be passed when calling the sensitivity program:
	-n, --nominal-file   [filename]   : the relative name of the file from which the nominal parameter set should be read, default=nominal.params.
	-d, --sense-dir      [filename]   : the relative name of the directory to which the sensitivity results will be stored, default=sensitivities.
	-D, --data-dir       [filename]   : the relative name of the directory to which the raw simulation data will be stored, default=sim-data. WARNING: IF RUNNING MULTIPLE INSTANCES OF THIS PROGRAM (e.g. on cluster) EACH MUST HAVE A UNIQUE DATA DIRECTORY TO AVOID CONFLICT.
	-p, --percentage     [float]      : the maximum percentage by which nominal values will be perturbed (+/-), default=5.
	-P, --points         [int]        : the number of data points to collect on either side (+/-) of the nominal set, default=10.
	-c, --nominal-count  [int]        : the number of nominal sets to read from the file, default=1.
	-k, --skip           [int]        : the number of nominal sets in the file to skip over, a.k.a. the index of the line you would like to start reading from, default=0.
	-s, --random-seed    [int]        : the postivie integer value to be used as a seed in the random number generation for simulations, default is randomly generated based on system time and process id.
	-l, --processes      [int]        : the number of processes to which parameter sets can be sent for parallel data collection, default=2.
	-y, --recycle        [N/A]        : include this if the simulation output has already been generated FOR EXACTLY THE SAME FILES AND ARGUMENTS YOU ARE USING NOW, disabled by default
	-g, --generate_only  [N/A]        : include this to generate oscillations features files for perturbed parameter values without calculating sensitivity. This is the opposite of recycle. Including this command in conjunction with --recycle will cause the program to do nothing, disabled by default.
	-z, --delete-data    [N/A]        : include this to delete oscillation features data when the program exits. This will preserve sensitivity directory but remove the directory specified by -D, disabled by default.
	-q, --quiet          [N/A]        : include this to turn off printing messages to standard output, disabled by default.
	-e, --exec           [path]       : if included, the simulations are run by executing the program specified by path. The path argument should be the full path, but the default uses the relative path: "../sogen-deterministic/deterministic".
	-a, --sim-args       [args]       : if included, any argument after this will be passed to the simulation program. If -h is one of these arguments, the simulation help will be printed and the program will not run.
	-h, --help           [N/A]        : print out this help information.
For example, the following may be a valid call to program:
	"./s\_a -c 2 -k 4 --processes 6 -p 100 -P 10 -s 112358 -n ~/sensitivity-analysis/nominal.params -d  ~/sensitivity-analysis/sensitivity\_data -D  ~/sensitivity-analysis/simulation\_data -e ~/sogen-deterministic/deterministic --sim-args -u ~/sogen-deterministic/input.perturb"
where the short and long names may be interchanged with their short/long counterparts. Note that this example assumes the file system supports the "~/" path prefix -- for safety, it may be necessary to use full paths for file and directory arguments. This is partiuclarly encouraged when running on a cluster.
For information on the arguments that can be passed to the the simulation program (if using sogen-deterministic/deterministic) please see the README.md contained in that package or, if you have already complied that simulation program, navigate to the sogen-deterministic package directory and run "./deterministic -h".

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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

