#include "file_handler.h"
#include "instructions.h"
#include "program_counter.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static void dump_registers(struct InstructionHandler *ih) {
  printf("\nRegisters:\n");
  for (int i = 0; i < 32; i++) {
    printf("x%02d = 0x%08x", i, ih->registers[i]);
    if (ih->registers[i] != 0) {
      printf("  <-- Non-zero\n");
    } else {
      printf("\n");
    }
  }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
    printf("Usage: %s <input file> <output file>\n", argv[0]);
    return 1;
  }

  int instruction_cnt = 0;

  struct FileHandler *fh = create_filehandler();
  struct ProgramCounter *pc = create_counter();
  struct InstructionHandler *ih = create_instruction_handler();

  int *instructions_b = fh->read_file(fh, argv[1], &instruction_cnt);
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

  dump_registers(ih);

  int n = sizeof(ih->registers) / sizeof(ih->registers[0]);
  fh->write_file(argv[2], ih->registers, n);

  return 0;
}
