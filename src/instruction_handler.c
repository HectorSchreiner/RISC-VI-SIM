#include "instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct InstructionHandler *create_instruction_handler()
{
    struct InstructionHandler *ih = malloc(sizeof(struct InstructionHandler));
    if (!ih)
        return NULL;
    memset(ih->registers, 0, sizeof ih->registers);
    ih->memory = calloc(MEMORY_SIZE, 1);
    if (!ih->memory)
    {
        free(ih);
        return NULL;
    }
    ih->halted = 0;
    ih->pc_changed = 0;
    ih->decode_instruction = decode_instruction;
    ih->load_memory = load_memory;
    ih->store_memory = store_memory;
    return ih;
}

int load_memory(struct InstructionHandler *ih, unsigned addr, int bytes)
{
    if (addr > MEMORY_SIZE - bytes)
    {
        printf("Memory Read Out Of Bounds: %u\n", addr);
        return 0;
    }

    unsigned value = 0;
    for (int i = 0; i < bytes; i++)
    {
        value |= (unsigned)ih->memory[addr + i] << (8 * i);
    }
    return value;
}

void store_memory(struct InstructionHandler *ih, unsigned addr, int bytes, int value)
{
    if (addr > MEMORY_SIZE - bytes)
    {
        printf("Memory Write Out Of Bounds: %u\n", addr);
        return;
    }

    for (int i = 0; i < bytes; i++)
    {
        ih->memory[addr + i] = ((unsigned)value >> (8 * i)) & 0xFF;
    }
}

void execute_format_i1(struct InstructionHandler *ih);
void execute_format_i2(struct InstructionHandler *ih);
void execute_format_s(struct InstructionHandler *ih);
void execute_format_i3(struct InstructionHandler *ih, struct ProgramCounter *pc);
void execute_format_i4(struct InstructionHandler *ih);
void execute_format_r(struct InstructionHandler *ih);

void execute_format_b(struct InstructionHandler *ih, struct ProgramCounter *pc);
void execute_format_j(struct InstructionHandler *ih, struct ProgramCounter *pc);
void execute_format_u1(struct InstructionHandler *ih);
void execute_format_u2(struct InstructionHandler *ih, struct ProgramCounter *pc);

instruction_err_t decode_instruction(struct InstructionHandler *ih, int bits, struct ProgramCounter *pc)
{
    ih->rs1 = (bits >> 15) & 0b11111;
    ih->rs2 = (bits >> 20) & 0b11111;
    ih->rd = (bits >> 7) & 0b11111;
    ih->funct3 = (bits >> 12) & 0b111;
    ih->funct7 = (bits >> 25) & 0b1111111;

    switch (bits & 0b1111111)
    {
    case 0b0110011: // function type R
        execute_format_r(ih);
        break;

    case 0b0010011: // function type I-1 (Immediate Operations)
        ih->imm = bits >> 20; // arithmetic shift sign-extends
        execute_format_i1(ih);
        break;

    case 0b0000011: // function type I-2 (Load Store)
        ih->imm = bits >> 20; // arithmetic shift sign-extends
        execute_format_i2(ih);
        break;

    case 0b1100111: // function type I-3 (Jump and Link reg)
        ih->imm = bits >> 20; // arithmetic shift sign-extends
        execute_format_i3(ih, pc);
        break;

    case 0b1110011: // function type I-4 (Enviroment Calls)
        ih->imm = bits >> 20; // arithmetic shift sign-extends
        execute_format_i4(ih);
        break;

    case 0b0100011: // function type S
        ih->imm = ((bits >> 25) << 5) | ((bits >> 7) & 0b11111); // arithmetic shift sign-extends
        execute_format_s(ih);
        break;

    case 0b1100011: // function type B
        ih->imm = (((bits >> 31) & 0b1) << 12) | (((bits >> 7) & 0b1) << 11) | (((bits >> 8) & 0b1111) << 1) | (((bits >> 25) & 0b111111) << 5);
        execute_format_b(ih, pc);
        break;

    case 0b1101111: // function type J
        ih->imm = (((bits >> 31) & 0b1) << 20) | (((bits >> 21) & 0b1111111111) << 1) | (((bits >> 20) & 0b1) << 11) | (((bits >> 12) & 0b11111111) << 12);
        execute_format_j(ih, pc);
        break;

    case 0b0110111: // function type U (Load Upper Imm)
        ih->imm = bits & 0xFFFFF000;
        execute_format_u1(ih);
        break;
    case 0b0010111: // function type U (Add Upper Imm To PC)
        ih->imm = bits & 0xFFFFF000;
        execute_format_u2(ih, pc);
        break;

    default:
        printf("Function Type Not Existing: %d\n", (bits & 0b1111111));
        return UNKNOWN;
    }

    ih->registers[0] = 0; // x0 is hardwired to zero
    return OK;
}

void execute_format_r(struct InstructionHandler *ih)
{
    int src_1 = ih->registers[ih->rs1];
    int src_2 = ih->registers[ih->rs2];

    switch (ih->funct3)
    {
    case 0:
        if (ih->funct7 == 0)
        { // ADD
            ih->registers[ih->rd] = src_1 + src_2;
            printf("ADD x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        }
        if (ih->funct7 == 0x20)
        { // SUB
            ih->registers[ih->rd] = src_1 - src_2;
            printf("SUB x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        }
        break;
    case 0x4:
        // XOR
        ih->registers[ih->rd] = src_1 ^ src_2;
        printf("XOR x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x6:
        // OR
        ih->registers[ih->rd] = src_1 | src_2;
        printf("OR x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x7:
        // OR
        ih->registers[ih->rd] = src_1 & src_2;
        printf("AND x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x1:
        // SLL
        ih->registers[ih->rd] = src_1 << src_2;
        printf("SLL x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x5:
        // SRL
        if (ih->funct7 == 0)
        {
            ih->registers[ih->rd] = src_1 >> src_2;
            printf("SRL x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        }
        // SRA
        if (ih->funct7 == 0x20)
        {
            // TODO (NOT WORKING!!!)
            ih->registers[ih->rd] = src_1 >> src_2;
            printf("SRA x%d(%d), x%d(%d) -> x%d(%d)\n -- NOT IMPLEMENTED CORRECTLY", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        }
        break;

    case 0x2:
        // SLT
        ih->registers[ih->rd] = (src_1 < src_2 ? 1 : 0);
        printf("SLT x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x3:
        // SLTU TODO NOT WORKING!!
        ih->registers[ih->rd] = (src_1 < src_2 ? 1 : 0);
        printf("SLTU x%d(%d), x%d(%d) -> x%d(%d) -- NOT IMPLEMENTED CORRECTLY\n", ih->rs1, src_1, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        break;

    default:
        printf("UNKOWN FUNC: %d\n", ih->funct3);
        break;
    }
}

void execute_format_i1(struct InstructionHandler *ih)
{
    int src = ih->registers[ih->rs1];
    int src_2 = ih->imm;

    switch (ih->funct3)
    {
    case 0:
        // ADDI
        ih->registers[ih->rd] = src + ih->imm;
        printf("ADDI x%d(%d) + %d -> x%d(%d)\n", ih->rs1, src, ih->imm, ih->rd, ih->registers[ih->rd]);
        break;
    case 0x4:
        // XORI
        ih->registers[ih->rd] = src ^ ih->imm;
        printf("XORI x%d(%d) + %d -> x%d(%d)\n", ih->rs1, src, ih->imm, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x6:
        // OR
        ih->registers[ih->rd] = src | ih->imm;
        printf("ORI x%d(%d) + %d -> x%d(%d)\n", ih->rs1, src, ih->imm, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x7:
        // ANDI
        ih->registers[ih->rd] = src & ih->imm;
        printf("ANDI x%d(%d) + %d -> x%d(%d)\n", ih->rs1, src, ih->imm, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x1:
        // SLLI
        ih->registers[ih->rd] = src << (ih->imm & 0b11111);
        printf("SLLI x%d(%d) + %d -> x%d(%d)\n", ih->rs1, src, ih->imm, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x5:
        // SRLI
        if (((ih->imm >> 5) & 0b11111111) == 0x00)
        {
            // SLLI
            ih->registers[ih->rd] = src >> (ih->imm & 0b11111);
            printf("SRLI x%d(%d) + %d -> x%d(%d)\n", ih->rs1, src, ih->imm, ih->rd, ih->registers[ih->rd]);
            break;
        }
        // SRAI
        if (((ih->imm >> 5) & 0b11111111) == 0x20)
        {
            // TODO (NOT WORKING!!!)
            ih->registers[ih->rd] = src >> (ih->imm & 0b11111);
            printf("SRAI x%d(%d) + %d -> x%d(%d)\n -- NOT IMPLEMENTED CORRECTLY", ih->rs1, src, ih->imm, ih->rd, ih->registers[ih->rd]);
        }
        break;

    case 0x2:
        // SLTI
        ih->registers[ih->rd] = (src < src_2 ? 1 : 0);
        printf("SLTI x%d(%d), x%d(%d) -> x%d(%d)\n", ih->rs1, src, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x3:
        // SLTIU TODO NOT WORKING!!
        ih->registers[ih->rd] = (src < src_2 ? 1 : 0);
        printf("SLTU x%d(%d), x%d(%d) -> x%d(%d)\n -- NOT IMPLEMENTED CORRECTLY", ih->rs1, src, ih->rs2, src_2, ih->rd, ih->registers[ih->rd]);
        break;

    default:
        printf("UNKOWN FUNC: %d\n", ih->funct3);
        break;
    }
}

void execute_format_i2(struct InstructionHandler *ih)
{
    unsigned addr = ih->registers[ih->rs1] + ih->imm;

    switch (ih->funct3)
    {
    case 0:
        // LB
        ih->registers[ih->rd] = (signed char)ih->load_memory(ih, addr, 1);

        printf("LB M[%u] -> x%d(%d)\n", addr, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x1:
        // LH
        ih->registers[ih->rd] = (short)ih->load_memory(ih, addr, 2);

        printf("LH M[%u] -> x%d(%d)\n", addr, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x2:
        // LW
        ih->registers[ih->rd] = ih->load_memory(ih, addr, 4);

        printf("LW M[%u] -> x%d(%d)\n", addr, ih->rd, ih->registers[ih->rd]);
        break;

    case 0x4:
        // LBU
        ih->registers[ih->rd] = ih->load_memory(ih, addr, 1);

        printf("LBU M[%u] -> x%d(%d)\n", addr, ih->rd, ih->registers[ih->rd]);

        break;

    case 0x5:
        // LHU
        ih->registers[ih->rd] = ih->load_memory(ih, addr, 2);

        printf("LHU M[%u] -> x%d(%d)\n", addr, ih->rd, ih->registers[ih->rd]);
        break;

    default:
        printf("UNKOWN FUNC: %d\n", ih->funct3);
        break;
    }
}

void execute_format_s(struct InstructionHandler *ih)
{
    unsigned addr = ih->registers[ih->rs1] + ih->imm;
    int value = ih->registers[ih->rs2];

    switch (ih->funct3)
    {
    case 0x0:
        // SB
        ih->store_memory(ih, addr, 1, value);

        printf("SB x%d(%d) -> M[%u]\n", ih->rs2, value & 0xFF, addr);
        break;

    case 0x1:
        // SH
        ih->store_memory(ih, addr, 2, value);

        printf("SH x%d(%d) -> M[%u]\n", ih->rs2, value & 0xFFFF, addr);
        break;

    case 0x2:
        // SW
        ih->store_memory(ih, addr, 4, value);

        printf("SW x%d(%d) -> M[%u]\n", ih->rs2, value, addr);
        break;

    default:
        printf("UNKOWN FUNC: %d\n", ih->funct3);
        break;
    }
}

void execute_format_i3(struct InstructionHandler *ih, struct ProgramCounter *pc)
{
    int target = (ih->registers[ih->rs1] + ih->imm) & ~1; // read rs1 before writing rd
    ih->registers[ih->rd] = pc->cnt + 4;

    pc->cnt = target;
    ih->pc_changed = 1;
    printf("JALR x%d, %d(x%d) -> PC(%d)\n", ih->rd, ih->imm, ih->rs1, pc->cnt);
}

void execute_format_i4(struct InstructionHandler *ih)
{
    printf("E");
    if ((ih->imm & 0b1111) == 0x0)
    {
        printf("CALL\n");
        if (ih->registers[17] == 10)
        {
            ih->halted = 1;
        }
        sleep(0);
    }
    else if ((ih->imm & 0b1111) == 0x1)
    {
        printf("BREAK\n");
    }
}

void execute_format_b(struct InstructionHandler *ih, struct ProgramCounter *pc)
{
    if (ih->imm & 0b1000000000000)
    {
        ih->imm |= ~0b1111111111111;
    }

    int a = ih->registers[ih->rs1];
    int b = ih->registers[ih->rs2];
    int taken = 0;

    switch (ih->funct3)
    {
    case 0x0:
        taken = a == b;
        printf("BEQ\n");
        break;
    case 0x1:
        taken = a != b;
        printf("BNE\n");
        break;
    case 0x4:
        taken = a < b;
        printf("BLT\n");
        break;
    case 0x5:
        taken = a >= b;
        printf("BGE\n");
        break;
    case 0x6:
        taken = (unsigned)a < (unsigned)b;
        printf("BLTU\n");
        break;
    case 0x7:
        taken = (unsigned)a >= (unsigned)b;
        printf("BGEU\n");
        break;
    default:
        printf("Unknown branch funct3: %d\n", ih->funct3);
        break;
    }

    if (taken)
    {
        pc->cnt += ih->imm;
        ih->pc_changed = 1;
    }
}

void execute_format_j(struct InstructionHandler *ih, struct ProgramCounter *pc)
{
    if (ih->imm & 0x100000)
    {
        ih->imm |= ~0x1FFFFF;
    }

    ih->registers[ih->rd] = pc->cnt + 4;
    pc->cnt += ih->imm;
    ih->pc_changed = 1;
    printf("JAL x%d, %d -> PC(%d)\n", ih->rd, ih->imm, pc->cnt);
}

void execute_format_u1(struct InstructionHandler *ih)
{
    // LUI
    ih->registers[ih->rd] = ih->imm;
    printf("LUI %d -> x%d(%d)\n", ih->imm, ih->rd, ih->registers[ih->rd]);
}

void execute_format_u2(struct InstructionHandler *ih, struct ProgramCounter *pc)
{
    // AUIPC
    ih->registers[ih->rd] = pc->cnt + ih->imm;
    printf("AUIPC PC(%d) + %d -> x%d(%d)\n", pc->cnt, ih->imm, ih->rd, ih->registers[ih->rd]);
}