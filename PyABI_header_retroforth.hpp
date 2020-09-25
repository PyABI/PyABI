/***


***/

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <array>
#include <map>

/***

TOS, NOS and TORS are defined as macros

***/

// Shortcut for top item on stack
#define TOS  data[sp]

// Shortcut for second item on stack
#define NOS  data[sp-1]

// Shortcut for top item on address stack
#define TORS address[rp]

/***


***/

typedef void (*Handler)(void);

/***


***/


/***

Nga ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copyright (c) 2008 - 2019, Charles Childers
Copyright (c) 2009 - 2010, Luke Parrish
Copyright (c) 2010,        Marc Simpson
Copyright (c) 2010,        Jay Skeer
Copyright (c) 2011,        Kenneth Keating

***/

template <class CELL, int64_t IMAGE_SIZE, int64_t STACK_DEPTH, int64_t ADDRESSES>
class RETRO_VM {

public:

  RETRO_VM(CELL CELL_MIN, CELL CELL_MAX)
    : ip(0), sp(0), rp(0), image_size(IMAGE_SIZE + 1), cell_min(CELL_MIN), cell_max(CELL_MAX)
  {

    IO_deviceHandlers[0] = &generic_output;
    IO_deviceHandlers[1] = &generic_input;

    IO_queryHandlers[0] = &generic_output_query;
    IO_queryHandlers[1] = &generic_input_query;

  }

  /***



  ***/

  CELL ngaLoadImage(const char* imageFile, int64_t ngaImage[], int64_t ngaImageCells) {
    FILE* fp;
    CELL imageSize;
    long fileLen;
    CELL i;
    if (imageFile && (fp = fopen(imageFile, "rb")) != NULL) {
      /* Determine length (in cells) */
      fseek(fp, 0, SEEK_END);
      fileLen = ftell(fp) / sizeof(CELL);
      rewind(fp);
      /* Read the file into memory */
      imageSize = fread(&memory, sizeof(CELL), fileLen, fp);
      fclose(fp);
    }
    else {
      for (i = 0; i < ngaImageCells; i++)
        memory[i] = ngaImage[i];
      imageSize = i;
    }
    return imageSize;
  }

  /***



  ***/

  void execute(CELL cell) {
    CELL opcode;
    rp = 1;
    ip = cell;
    while (ip < IMAGE_SIZE) {
      opcode = memory[ip];
      if (ngaValidatePackedOpcodes(opcode) != 0) {
        ngaProcessPackedOpcodes(opcode);
      }
      else {
        printf("Invalid instruction!\n");
        exit(1);
      }
      ip++;
      if (rp == 0)
        ip = IMAGE_SIZE;
    }
  }


  /***



  ***/




  /***



  ***/


protected:


  /***



  ***/


  CELL stack_pop() {
    sp--;
    return data[sp + 1];
  }

  void stack_push(CELL value) {
    sp++;
    data[sp] = value;
  }


  /***



  ***/

  void generic_output() {
    std::cout << (char)stack_pop();
  }

  void generic_output_query() {
    stack_push(0);
    stack_push(0);
  }

  void generic_input() {
    char in;
    std::cin >> in;
    stack_push(in);
    if (TOS == 127) TOS = 8;
  }

  void generic_input_query() {
    stack_push(0);
    stack_push(1);
  }

protected:


  /***



  ***/

private:

  enum vm_opcode {
    VM_NOP, VM_LIT, VM_DUP, VM_DROP, VM_SWAP, VM_PUSH, VM_POP,
    VM_JUMP, VM_CALL, VM_CCALL, VM_RETURN, VM_EQ, VM_NEQ, VM_LT,
    VM_GT, VM_FETCH, VM_STORE, VM_ADD, VM_SUB, VM_MUL, VM_DIVMOD,
    VM_AND, VM_OR, VM_XOR, VM_SHIFT, VM_ZRET, VM_HALT, VM_IE,
    VM_IQ, VM_II
  };
#define NUM_OPS VM_II + 1

  void inst_nop() {
  }

  void inst_lit() {
    sp++;
    ip++;
    TOS = memory[ip];
  }

  void inst_dup() {
    sp++;
    data[sp] = NOS;
  }

  void inst_drop() {
    data[sp] = 0;
    if (--sp < 0)
      ip = IMAGE_SIZE;
  }

  void inst_swap() {
    CELL a;
    a = TOS;
    TOS = NOS;
    NOS = a;
  }

  void inst_push() {
    rp++;
    TORS = TOS;
    inst_drop();
  }

  void inst_pop() {
    sp++;
    TOS = TORS;
    rp--;
  }

  void inst_jump() {
    ip = TOS - 1;
    inst_drop();
  }

  void inst_call() {
    rp++;
    TORS = ip;
    ip = TOS - 1;
    inst_drop();
  }

  void inst_ccall() {
    CELL a, b;
    a = TOS; inst_drop();  /* False */
    b = TOS; inst_drop();  /* Flag  */
    if (b != 0) {
      rp++;
      TORS = ip;
      ip = a - 1;
    }
  }

  void inst_return() {
    ip = TORS;
    rp--;
  }

  void inst_eq() {
    NOS = (NOS == TOS) ? -1 : 0;
    inst_drop();
  }

  void inst_neq() {
    NOS = (NOS != TOS) ? -1 : 0;
    inst_drop();
  }

  void inst_lt() {
    NOS = (NOS < TOS) ? -1 : 0;
    inst_drop();
  }

  void inst_gt() {
    NOS = (NOS > TOS) ? -1 : 0;
    inst_drop();
  }

  void inst_fetch() {
    switch (TOS) {
    case -1: TOS = sp - 1; break;
    case -2: TOS = rp; break;
    case -3: TOS = image_size; break;
    case -4: TOS = cell_min; break;
    case -5: TOS = cell_max; break;
    default: TOS = memory[TOS]; break;
    }
  }

  void inst_store() {
    if (TOS <= IMAGE_SIZE && TOS >= 0) {
      memory[TOS] = NOS;
      inst_drop();
      inst_drop();
    }
    else {
      ip = IMAGE_SIZE;
    }
  }

  void inst_add() {
    NOS += TOS;
    inst_drop();
  }

  void inst_sub() {
    NOS -= TOS;
    inst_drop();
  }

  void inst_mul() {
    NOS *= TOS;
    inst_drop();
  }

  void inst_divmod() {
    CELL a, b;
    a = TOS;
    b = NOS;
    TOS = b / a;
    NOS = b % a;
  }

  void inst_and() {
    NOS = TOS & NOS;
    inst_drop();
  }

  void inst_or() {
    NOS = TOS | NOS;
    inst_drop();
  }

  void inst_xor() {
    NOS = TOS ^ NOS;
    inst_drop();
  }

  void inst_shift() {
    CELL y = TOS;
    CELL x = NOS;
    if (TOS < 0)
      NOS = NOS << (TOS * -1);
    else {
      if (x < 0 && y > 0)
        NOS = x >> y | ~(~0U >> y);
      else
        NOS = x >> y;
    }
    inst_drop();
  }

  void inst_zret() {
    if (TOS == 0) {
      inst_drop();
      ip = TORS;
      rp--;
    }
  }

  void inst_halt() {
    ip = IMAGE_SIZE;
  }

  void inst_ie() {
    sp++;
    TOS = IO_queryHandlers.count();
  }

  void inst_iq() {
    CELL Device = TOS;
    inst_drop();
    IO_queryHandlers[Device]();
  }

  void inst_ii() {
    CELL Device = TOS;
    inst_drop();
    IO_deviceHandlers[Device]();
  }

  Handler instructions[NUM_OPS] = {
    inst_nop, inst_lit, inst_dup, inst_drop, inst_swap, inst_push, inst_pop,
    inst_jump, inst_call, inst_ccall, inst_return, inst_eq, inst_neq, inst_lt,
    inst_gt, inst_fetch, inst_store, inst_add, inst_sub, inst_mul, inst_divmod,
    inst_and, inst_or, inst_xor, inst_shift, inst_zret, inst_halt, inst_ie,
    inst_iq, inst_ii
  };

  void ngaProcessOpcode(CELL opcode) {
    if (opcode != 0)
      instructions[opcode]();
  }

  int ngaValidatePackedOpcodes(CELL opcode) {
    CELL raw = opcode;
    CELL current;
    int valid = -1;
    int i;
    for (i = 0; i < 4; i++) {
      current = raw & 0xFF;
      if (!(current >= 0 && current <= 29))
        valid = 0;
      raw = raw >> 8;
    }
    return valid;
  }

  void ngaProcessPackedOpcodes(CELL opcode) {
    CELL raw = opcode;
    int i;
    for (i = 0; i < 4; i++) {
      ngaProcessOpcode(raw & 0xFF);
      raw = raw >> 8;
    }
  }

private:

  int64_t sp, rp, ip, image_size;

  CELL cell_min, cell_max;

  std::array<CELL, STACK_DEPTH> data;

  std::array<CELL, ADDRESSES> address;

  std::array<CELL, IMAGE_SIZE + 1> memory;

  std::map<int, Handler> IO_deviceHandlers;

  std::map<int, Handler> IO_queryHandlers;

};