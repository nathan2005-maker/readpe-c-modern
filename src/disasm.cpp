#include "disasm.h"
 
#include <Zydis/Zydis.h>
#include <cstdio>
 
extern "C" void
disasm_print_bytes(const uint8_t* code, size_t size, uint64_t runtime_address)
