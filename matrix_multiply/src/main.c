/*
Copyright (c) 2015, Los Alamos National Security, LLC
All rights reserved.

Copyright 2015. Los Alamos National Security, LLC. This software was
produced under U.S. Government contract DE-AC52-06NA25396 for Los
Alamos National Laboratory (LANL), which is operated by Los Alamos
National Security, LLC for the U.S. Department of Energy. The
U.S. Government has rights to use, reproduce, and distribute this
software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
derivative works, such modified software should be clearly marked, so
as not to confuse it with the version available from LANL.

Additionally, redistribution and use in source and binary forms, with
or without modification, are permitted provided that the following
conditions are met:

• Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.

• Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer
         in the documentation and/or other materials provided with the
         distribution.

• Neither the name of Los Alamos National Security, LLC, Los Alamos
         National Laboratory, LANL, the U.S. Government, nor the names
         of its contributors may be used to endorse or promote
         products derived from this software without specific prior
         written permission.

THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LOS
ALAMOS NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

//*****************************************************************************
//
// AUTHOR:  Heather Quinn
// CONTACT INFO:  hquinn@lanl.gov
// LAST EDITED: 12/21/15
//
// main.c
//
// This test is a simple program for calculating matrix multiplies.  Right now
// it takes approximately the same amount of memory as the cache test. 
//
// This software is otimized for the MSP430F2619.
//
// The output is designed to go out the UART at a speed of 9,600 baud and uses a tiny
// print to reduce the printf footprint.  The tiny printf can be downloaded from 
// http://www.43oh.com/forum/viewtopic.php?f=10&t=1732  All of the output is YAML
// parsable.
//
// The input is currently using random numbers that change values every few seconds 
// in a repeatable pattern.
//
//*****************************************************************************

// Modified to run on a Zybo board. - JM
// UART changed to 115200, 8 bits, no parity, 1 stop - JM

#include "platform.h"
#include <xgpio.h>
#include <stdlib.h>

#define		SIDE				12
#define		ROBUST_PRINTING			1
#define		LOOP_COUNT			2
#define		CHANGE_RATE			1


int first_matrix[SIDE][SIDE];
int second_matrix[SIDE][SIDE];
unsigned long results_matrix[SIDE][SIDE];
unsigned long golden_matrix[SIDE][SIDE];

unsigned long int ind = 0;
int local_errors = 0;
int in_block = 0;
int seed_value = -1;

void init_matrices() {
  int i = 0;
  int j = 0;

  //seed the random number generator
  //the method is designed to reset SEUs in the matrices, using the current seed value
  //that way each test starts error free
  if (seed_value == -1) {
    srand(ind);
    seed_value = ind;
  }
  else {
    srand(seed_value);
  }

  //fill the matrices
  for ( i = 0; i < SIDE; i++ ){
    for (j = 0; j < SIDE; j++) {
      first_matrix[i][j] = rand();
      second_matrix[i][j] = rand();
    }
  }
}

void matrix_multiply(int f_matrix[][SIDE], int s_matrix[][SIDE], unsigned long r_matrix[][SIDE]) {
  int i = 0;
  int j = 0;
  int k = 0;
  unsigned long sum = 0;
  
  //MM
  for ( i = 0 ; i < SIDE ; i++ ) {
    for ( j = 0 ; j < SIDE ; j++ ) {
      for ( k = 0 ; k < SIDE ; k++ ) {
	sum = sum + f_matrix[i][k]*s_matrix[k][j];
      }
      
      r_matrix[i][j] = sum;
      sum = 0;
    }
  }
}

int checker(unsigned long golden_matrix[][SIDE], unsigned long results_matrix[][SIDE]) {
  int first_error = 0;
  int num_of_errors = 0;
  int i = 0;
  int j = 0;

  for(i=0; i<SIDE; i++) {
    for (j = 0; j < SIDE; j++) {
      if (golden_matrix[i][j] != results_matrix[i][j]) {
	//checker found an error, print results to screen
	if (!first_error) {
	  if (!in_block && ROBUST_PRINTING) {
	    xil_printf(" - i: %lu\r\n", ind);
	    xil_printf("   E: {%i_%i: [%x, %x],", i, j, golden_matrix[i][j], results_matrix[i][j]);
	    first_error = 1;
	    in_block = 1;
	  }
	  else if (in_block && ROBUST_PRINTING){
	    xil_printf("   E: {%i_%i: [%x, %x],", i, j, golden_matrix[i][j], results_matrix[i][j]);
	    first_error = 1;
	  }
	}
	else {
	  if (ROBUST_PRINTING)
	    xil_printf("%i_%i: [%x, %x],", i, j, golden_matrix[i][j], results_matrix[i][j]);
	  
	}
	num_of_errors++;
      }
    }
  }
  
  if (first_error) {
    xil_printf("}\r\n");
    first_error = 0;
  }
  
  if (!ROBUST_PRINTING && (num_of_errors > 0)) {
    if (!in_block) {
      xil_printf(" - i: %lu\r\n", ind);
      xil_printf("   E: %i\r\n", num_of_errors);
      in_block = 1;
    }
    else {
      xil_printf("   E: %i\r\n", num_of_errors);
    }
  }
  
  return num_of_errors;
}

void matrix_multiply_test(int loops) {
  
  //initialize variables
  int total_errors = 0;
  
  init_matrices();
  //setup golden values
  matrix_multiply(first_matrix, second_matrix, golden_matrix);
  
  while (ind < loops) {
    matrix_multiply(first_matrix, second_matrix, results_matrix);
    local_errors = checker(golden_matrix, results_matrix);
    
    //if there is an error, fix the input matrics
    //golden is recomputed so that the code doesn't
    //have to figure out if the error was in the results
    //or golden matrix
    if (local_errors > 0) {
      init_matrices();
      matrix_multiply(first_matrix, second_matrix, golden_matrix);
    }
    
    //acking to see if alive, as well as changing input values
    if (ind % CHANGE_RATE == 0) {
      xil_printf("# %lu, %i\r\n", ind, total_errors);
      seed_value = -1;
      init_matrices();
      //have to recompute the golden
      matrix_multiply(first_matrix, second_matrix, golden_matrix);
    }
    
    //reset vars and such
    ind++;
    total_errors += local_errors;
    local_errors = 0;
    in_block = 0;
  }

}

int main()
{
  //init part
  init_platform();

  //print the YAML header
  print("\r\n---\r\n");
  print("hw: Zybo ZYNQ7010\r\n");
  print("test: MM\r\n");
  print("mit: none\r\n");
  xil_printf("printing: %i\r\n", ROBUST_PRINTING);
  xil_printf("SIDE matrix size: %i\r\n", SIDE);
  xil_printf("Loop count: %i\r\n", LOOP_COUNT);
  xil_printf("input change rate: %i\r\n", CHANGE_RATE);
  print("ver: 0.1-zybo\r\n");
  print("fac: GSFC 2017\r\n");
  print("d:\r\n");

  /* Set a breakpoint on this label to let DrSEUS restart exectuion when readdy. */
  asm("drseus_start_tag:");

  //start test
  matrix_multiply_test(LOOP_COUNT);

  asm("drseus_end_tag:");
  print("safeword ");
  cleanup_platform();
  
  return 0;

}
