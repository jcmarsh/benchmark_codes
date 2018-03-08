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
// CONTACT INFO:  hquinn at lanl dot gov
// LAST EDITED: 12/21/2015
//
// aes.c
//
// This piece of code is part of the mitigation working group benchmark
// suite.  This program executes a 128-AES code from Texas Instruments
// using the National Institute of Standards and Technology.  While we
// have converted the test vector suites to header files, two codes need
// to be downloaded and linked to:
//
// TI AES-128 code: http://www.ti.com/tool/AES-128
// Tiny printf: http://www.43oh.com/forum/viewtopic.php?f=10&t=1732

// Download those files and compile them with the rest of the AES code.
//
// This software is otimized for TI microcontrollers, specifically the
// MSP430F2619.
//
// The output is designed to go out the UART at a speed of 9,600 baud
// and uses a tiny printf to reduce printf footprint.  All of the
// output is YAML parsable.
//
//
//*****************************************************************************

// Modified to run on a Zybo board. - JM
// UART changed to 115200, 8 bits, no parity, 1 stop - JM

#include "platform.h"
#include <xgpio.h>
#include "TI_aes_128.h"
#include "ECBGFSbox128.h"
#include "ECBKeySbox128.h"
#include "ECBVarKey128.h"
#include "ECBVarTxt128.h"

//all of the printing is YAML parsable.  The robust printing variable solely determines how much text you get.
#define ROBUST_PRINTING	1
#define	LOOP_COUNT	2
#define	CHANGE_RATE	250

unsigned long ind = 0;
int local_errors = 0;
int in_block = 0;

void aes_test(int loops);
void check_arrays(char array1[], char array2[], int lim, char pre);

void check_arrays(char *array1, char *array2, int lim, char pre) {
  int first_error = 0;
  int i = 0;
  int numberOfErrors = 0;
  
  for (i = 0; i < lim; i++) {
    if (array1[i] != array2[i]) {
      //block of code for printing errors
      if (!first_error) {
	if (!in_block && ROBUST_PRINTING) {
	  xil_printf(" - i: %lu\r\n", ind);
	  xil_printf("   %c: {%x: %x,", pre, array1[i], array2[i]);
	  first_error = 1;
	  in_block = 1;  
	}
	else if (in_block && ROBUST_PRINTING){
	  xil_printf("   %c: {%x: %x,", pre, array1[i], array2[i]);
	  first_error = 1;
	}
      }
      else{
	if (ROBUST_PRINTING)
	  xil_printf("%x: %x,", array1[i], array2[i]);
      }
      local_errors++;
      numberOfErrors++;
    }
  }
  if (first_error && ROBUST_PRINTING) {
    //finish YAML block
    xil_printf("}\r\n");
    first_error = 0;
  }
  
  if (!ROBUST_PRINTING && numberOfErrors > 0) {
    //less prolific printing
    if (!in_block) {
      xil_printf(" - i: %lu\r\n", ind);
      xil_printf("   %c: %i\r\n", pre, numberOfErrors);
      in_block = 1;
    }
    else {
      xil_printf("   %c: %i\r\n", pre, numberOfErrors);
    }
  }
}

void aes_test(int loops)
{
  int i = 0;
  int j = 0;
  int k = 0;
  unsigned int count = 0;
  unsigned int ret = 0;
  int total_errors = 0;
  int first_error = 0;
  
  unsigned char key[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  unsigned char key2[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  unsigned char gold_cypher[] = {0x08, 0xa4, 0xe2, 0xef, 0xec, 0x8a, 0x8e, 0x33, 0x12, 0xca, 0x74, 0x60, 0xb9, 0x04, 0x0b, 0xbf };
  unsigned char gold_plain[] = {0x58, 0xc8, 0xe0, 0x0b, 0x26, 0x31, 0x68, 0x6d, 0x54, 0xea, 0xb8, 0x4b, 0x91, 0xf0, 0xac, 0xa1 };
  unsigned char input[] = {0x58, 0xc8, 0xe0, 0x0b, 0x26, 0x31, 0x68, 0x6d, 0x54, 0xea, 0xb8, 0x4b, 0x91, 0xf0, 0xac, 0xa1 };
  
  while(ind < loops) {
    for (k = 0; k < 4; k++) {
      if (k == 0) {
	count = ECBGFSbox128_count;
      }
      else if (k == 1) {
	count = ECBKeySbox128_count;
      }
      else if (k == 2) {
	count = ECBVarKey128_count;
      }
      else {
	count = ECBVarTxt128_count;
      }
      
      for (j = 0; j < count; j++){	
	//read data arrays out of flash
	for (i = 0; i < 16; i++) {
	  if (k == 0) {
	    key[i] = ECBGFSbox128[i + j*80];
	    key2[i] = ECBGFSbox128[i + 16 + j*80];
	    gold_cypher[i] = ECBGFSbox128[i + 32 + j*80];
	    gold_plain[i] = ECBGFSbox128[i + 48 + j*80];
	    input[i] = ECBGFSbox128[i + 64 + j*80];
	  }
	  if (k == 1) {
	    key[i] = ECBKeySbox128[i + j*80];
	    key2[i] = ECBKeySbox128[i + 16 + j*80];
	    gold_cypher[i] = ECBKeySbox128[i + 32 + j*80];
	    gold_plain[i] = ECBKeySbox128[i + 48 + j*80];
	    input[i] = ECBKeySbox128[i + 64 + j*80];
	  }
	  if (k == 2) {
	    key[i] = ECBVarKey128[i + j*80];
	    key2[i] = ECBVarKey128[i + 16 + j*80];
	    gold_cypher[i] = ECBVarKey128[i + 32 + j*80];
	    gold_plain[i] = ECBVarKey128[i + 48 + j*80];
	    input[i] = ECBVarKey128[i + 64 + j*80];
	  }
	  if (k == 3) {
	    key[i] = ECBVarTxt128[i + j*80];
	    key2[i] = ECBVarTxt128[i + 16 + j*80];
	    gold_cypher[i] = ECBVarTxt128[i + 32 + j*80];
	    gold_plain[i] = ECBVarTxt128[i + 48 + j*80];
	    input[i] = ECBVarTxt128[i + 64 + j*80];
	  }
	  
	}
	
	check_arrays(input, gold_plain, sizeof(input), 'S');
	
	aes_enc_dec(input,key,0);
	
	check_arrays(input, gold_cypher, sizeof(input), 'E');
	
	aes_enc_dec(input,key2,1);
	
	check_arrays(input, gold_plain, sizeof(input), 'D');
	
	total_errors += local_errors;
	local_errors = 0;
	in_block = 0;
	ind++;

	//print an "I am alive" message every once in a awhile
	//if (ind % CHANGE_RATE == 0 && ind != 0) {
	//  xil_printf("# %lu, %i\r\n", ind, total_errors);
	//}
      }
    }
  }
}

int main( void )
{
  
  //init part
  init_platform();
  
  //print YAML header
  print("\r\n---\r\n");
  print("hw: Zybo ZYNQ7010\r\n");
  print("test: aes\r\n");
  print("mit: none\r\n");
  xil_printf("printing: %i\r\n", ROBUST_PRINTING);
  xil_printf("Loop count: %i\r\n", LOOP_COUNT);
  xil_printf("input change rate: %i\r\n", CHANGE_RATE);
  print("ver: 0.1-zybo\r\n");
  print("fac: GSFC 2017\r\n");
  print("d:\r\n");

  // Warmup run
  aes_test(LOOP_COUNT);

  // Set a breakpoint on this label to let DrSEUS restart exectuion when ready
  asm("drseus_start_tag:");
  aes_test(LOOP_COUNT);
  asm("drseus_end_tag:");

  print("safeword ");

  return 0;
}
