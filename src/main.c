#include "file_handler.h"
#include "instructions.h"
#include "program_counter.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  int instruction_cnt = 0;

  struct FileHandler *fh = create_filehandler();
  struct ProgramCounter *pc = create_counter();
  struct InstructionHandler *ih = create_instruction_handler();

  int *instructions_b = fh->read_file(fh, &instruction_cnt);
  if (!instructions_b || !ih)
    return 1;

  if (instruction_cnt > MEMORY_SIZE) {
    printf("program does not fit in memory\n");
    return 1;
  }
  memcpy(ih->memory, instructions_b, instruction_cnt); // program is loaded at address 0

  pc->cnt = 0;
  pc->increment = 4;

  instruction_cnt /= 4;
  printf("the program is %d instructions\n", instruction_cnt);

  while (!ih->halted && pc->cnt / 4 < instruction_cnt) {
    ih->pc_changed = 0;
    printf("PC[+%d] - ", pc->cnt);
    ih->decode_instruction(ih, ih->load_memory(ih, pc->cnt, 4), pc);
    if (!ih->pc_changed)
      pc->increment_counter(pc);
  }

  return 0;
}
