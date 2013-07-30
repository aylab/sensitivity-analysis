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

2.0. Command-line arguments
***************************
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
-g, --generate_only  [N/A]        : include this to generate oscillations features files for\n\
                                     perturbed parameter values without calculating sensitivity.\n\
                                     This is the opposite of recycle. Including this command in\n\
                                     conjunction with --recycle will cause the program to do nothing,\n\
                                     disabled by default.\n\
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

