#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "program_counter.h"

#define MEMORY_SIZE (1 << 20) // 1 MB of byte addressable memory

typedef enum InstructionErr {
    UNKNOWN,
    OK,
} instruction_err_t;

struct InstructionHandler {
    int registers[32];
    unsigned char *memory;
    int rs1;
    int rs2;
    int rd;
    int funct7;
    int funct3;
    int imm;
    int halted;
    int pc_changed;
    instruction_err_t (*decode_instruction)(struct InstructionHandler *self, int bits, struct ProgramCounter* pc); // also something that resembles the instruction in bits
    int (*load_memory)(struct InstructionHandler *self, unsigned addr, int bytes);
    void (*store_memory)(struct InstructionHandler *self, unsigned addr, int bytes, int value);
};

struct InstructionHandler* create_instruction_handler();

/// @brief Takes the inputs bits, and converts it into the `Instruction` type.
/// @param self 
/// @return `instruction_err_t
instruction_err_t decode_instruction(struct InstructionHandler *self, int bits, struct ProgramCounter* pc);

/// @brief Reads `bytes` (1, 2 or 4) bytes from memory at `addr` in little endian order.
/// @param self
/// @param addr byte address
/// @param bytes
/// @return the value, zero extended
int load_memory(struct InstructionHandler *self, unsigned addr, int bytes);

/// @brief Writes the lowest `bytes` (1, 2 or 4) bytes of `value` to memory at `addr` in little endian order.
/// @param self
/// @param addr byte address
/// @param bytes
/// @param value
void store_memory(struct InstructionHandler *self, unsigned addr, int bytes, int value);

#endif
