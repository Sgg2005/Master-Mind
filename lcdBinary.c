/* ***************************************************************************** */
/* You can use this file to define the low-level hardware control fcts for       */
/* LED, button and LCD devices.                                                  */ 
/* Note that these need to be implemented in Assembler.                          */
/* You can use inline Assembler code, or use a stand-alone Assembler file.       */
/* Alternatively, you can implement all fcts directly in master-mind.c,          */  
/* using inline Assembler code there.                                            */
/* The Makefile assumes you define the functions here.                           */
/* ***************************************************************************** */


#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif

#define	PAGE_SIZE		(4*1024)
#define	BLOCK_SIZE		(4*1024)

#define	INPUT			 0
#define	OUTPUT			 1

#define	LOW			 0
#define	HIGH			 1


// APP constants   ---------------------------------

// Wiring (see call to lcdInit in main, using BCM numbering)
// NB: this needs to match the wiring as defined in master-mind.c

#define STRB_PIN 24
#define RS_PIN   25
#define DATA0_PIN 23
#define DATA1_PIN 10
#define DATA2_PIN 27
#define DATA3_PIN 22

// -----------------------------------------------------------------------------
// includes 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

// -----------------------------------------------------------------------------
// prototypes

int failure (int fatal, const char *message, ...);

// -----------------------------------------------------------------------------
// Functions to implement here (or directly in master-mind.c)

/* this version needs gpio as argument, because it is in a separate file */
void digitalWrite (uint32_t *gpio, int pin, int value) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
  int offset = (value == HIGH) ? 7 : 10;
  asm volatile(                           // Inline assembly to set the pin value
    "mov r1, #1 \n"                       //moves value 1 into the register 1
    "lsl r1, r1, %2 \n"                   // shifts the positon of r1 depending the position told by the pin
    "str r1, [%0, %1, lsl #2]\n"          //stores the value of r1 into the memory address
    :
    : "r" (gpio), "r" (offset), "r" (pin)
    : "r1" , "memory"
  );
}

// adapted from setPinMode
void pinMode(uint32_t *gpio, int pin, int mode /*, int fSel, int shift */) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
  int fSel = pin / 10;
  int shift = (pin % 10) * 3;
  
  asm volatile (
    "ldr r1, [%0, %1, lsl #2] \n"   //loads the value of r1
    "mov r2, #7 \n"                 //moves the value 7 into r2
    "lsl r2, r2, %2 \n"             //shifts r2 depending on the position
    "bic r1, r1, r2 \n"             //clears the bits of r1
    "mov r2, %3 \n"                 //moves the mode value into r2
    "lsl r2, r2, %2 \n"             
    "orr r1, r1, r2 \n"             //sets the bits in r1 to match with the pin's function
    "add r3, %0, %1, lsl #2 \n"     //calulates the address of the function 
    "str r1, [r3] \n"               //stores the modified value into that specific register
    :                              
    : "r" (gpio), "r" (fSel), "r" (shift), "r" (mode)  
    : "r1", "r2", "r3", "memory"   
  );
}

void writeLED(uint32_t *gpio, int led, int value) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
  int offset = (value == HIGH) ? 7 : 10;  // SET or CLEAR registers
  
  asm volatile (
    "mov r1, #1 \n"                  //moves value of r1
    "lsl r1, r1, %2 \n"              //shifts the value of r1 based on the number of positions
    "str r1, [%0, %1, lsl #2] \n"    //stores the value of r1 into the memory
    :                             
    : "r" (gpio), "r" (offset), "r" (led)  
    : "r1", "memory"              
  );
}

int readButton(uint32_t *gpio, int button) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
  int result;
  asm volatile (
    "ldr r1, [%1, #0x34] \n"         //loads the value from the memory address
    "and %0, r1, %2 \n"              //performs bitwise and between r1 
    "cmp %0, #0 \n"                  //compares the results with 0
    "movne %0, #1 \n"                //checks if the result is 0 if not sets value to  1
    :"=r" (result)                   // shows results
    : "r" (gpio), "r" (1 << button)  //
    : "r1", "cc", "memory" 
  );
  return result;
}

void waitForButton(uint32_t *gpio, int button) {
  /* ***  COMPLETE the code here, just C no Assembler; you can use readButton ***  */
  printf("Waiting for button press on GPIO pin %d...\n", button);
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 10000000L; // 10ms
  
  // Continuously check the button state
  while (1) {
      if (readButton(gpio, button)) {
          printf("Button on GPIO pin %d pressed!\n", button);
          break;
      }
      // Sleep for 10ms to avoid busy-waiting
  }
}