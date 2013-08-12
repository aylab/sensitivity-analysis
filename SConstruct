"""
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
"""

compile_flags = '-Wall -O2 '
link_flags = ''
if ARGUMENTS.get('profiling', 0):
	compile_flags += '-pg'
	link_flags += '-pg'
elif ARGUMENTS.get('debug', 0):
	compile_flags += '-g'
elif ARGUMENTS.get('memtrack', 0):
	compile_flags += '-D MEMTRACK'

env = Environment(CXX='g++')
env.Append(CXXFLAGS=compile_flags, LINKFLAGS=link_flags)
env.Program(target='sensitivity', source=['source/analysis.cpp', 'source/init.cpp', 'source/io.cpp', 'source/memory.cpp', 'finite-difference/finite-difference.cpp'])
