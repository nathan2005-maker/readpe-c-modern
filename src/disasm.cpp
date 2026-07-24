#include "disasm.h"
 
#include <Zydis/Zydis.h>
#include <cstdio>
 
extern "C" void
disasm_print_bytes(const uint8_t* code, size_t size, uint64_t runtime_address) {


    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    ZyanUSize offset = 0;
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];



}
