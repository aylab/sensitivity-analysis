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
memory.h contains function declarations for memory.cpp.
*/

#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdlib> //(Needed for size_t)
#define EXIT_MEMORY_ERROR 1
using namespace std;

void* mallocate(size_t);
void mfree(void*);
#if defined(MEMTRACK)
	void print_heap_usage();
#endif

#endif

