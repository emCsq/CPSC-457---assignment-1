/*The code for the disassembling was used from Udis86 Disassembler Library. Retrieved from udis86.sourceforge.net*/

#include <udis86.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <string.h>

int main (int argc, char* argv[]) {
  struct user_regs_struct regs;
  long pid, ret;
  int rand, check;
  int stat;
  long register1, register2;
  int size, i, j;
  long peeking;
  
  //invalid entry was entered into the cmd. Exits
  if (argv[1] == NULL) {
    printf("Improper entry. Require pid of traced process");
    exit(1);
  }
  
  //converts into long and informs user that the pid is being traced.
  pid = strtol(argv[2], NULL, 10);
  printf("Tracing pid %d \n", pid);
  
  //attaches to process and ensure that child has stopped
  ptrace(PTRACE_ATTACH, pid, NULL, NULL); 
  check = waitpid(pid, &rand, WUNTRACED); 
  
  //If invalid pid, a message will be printed before program exits.
  if (check != pid) {
    printf("Unable to trace.\n");
    exit(1);
  } 
  //else the following trace will start
  else {
    ptrace(PTRACE_GETREGS, pid, NULL, &regs); //Get register at the beginning of the process
    register1 = regs.eip; //retrieve the eip of the first instruction

    //essentially an infinite loop that will retrieve the instructions and disassembles into x86
    while(1) {
      ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL); //executes next instruction before waiting
      waitpid(pid, &stat, 0);
      ptrace(PTRACE_GETREGS, pid, NULL, &regs); //Get registers
      register2 = regs.eip; //retrieve the pid of the next step
      size = register2 - register1;	//gets the size of the instruction
      
      //if the size exceeds 15 and or is less than 0, then the size defaults to 5
      if (size > 15 || size < 0) {
	size = 5;
      }
      
      //A char array is created such that we can retrieve the instruction 
      char buffer[size];
      for (i = 0; i < size; i++) {
	buffer[i] = ptrace(PTRACE_PEEKTEXT, pid, regs.eip+i, NULL); //Retrieves part of instruction and stores into the buffer
	memcpy(buffer+(i*4), &buffer[i], 4);
      }
      
      //disassembles the instruction and translate it into x86 code before printing it
      ud_t ud_obj;
      ud_init(&ud_obj);
      ud_set_input_buffer(&ud_obj, buffer, size);
      ud_set_mode(&ud_obj, 32);
      ud_set_syntax(&ud_obj, UD_SYN_INTEL);
      ud_disassemble(&ud_obj);
      printf("\t%s\n", ud_insn_asm(&ud_obj));

      //sets the new register to the old register such that we can start again with the next register when the loop repeats
      register1 = register2;
    }
  }
  ptrace(PTRACE_DETACH, pid, NULL, NULL); //if the loop ever ends, the ptrace will detach from the process
}
