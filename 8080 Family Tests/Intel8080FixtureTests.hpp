#pragma once
#include "pch.h"
#include "i8080.h"
#include <cstdlib>
#include <cstring>

typedef uint16_t WORD;
typedef uint8_t  BYTE;

class Intel8080FixtureTests : public testing::Test {
protected:
    INTEL_8080 CPU;
    
    void InitializePairs() {
        CPU.BC = 0x0123;
        CPU.DE = 0x4567;
        CPU.HL = 0x89AB;
        CPU.PSW = 0xCDEF;
    }

    void SetWORDInMemory(WORD word, WORD address) {
        CPU.MEM[address] = (BYTE)word;
        CPU.MEM[address + 1] = word >> 8;
    }

    // return WORD in big endian order
    WORD GetWORDFromMemory(WORD address) {
        WORD result = 0;
        result = CPU.MEM[address + 1] << 8;
        result |= CPU.MEM[address];
        return result;
    }
    
    void SetUp() override {
        std::memset(&CPU, 0, sizeof(INTEL_8080));
        CPU.F = 0b00000010;
        CPU.MEM = (BYTE*)calloc(0x10000, sizeof(BYTE));
        CPU.PORT = (BYTE*)calloc(0x100, sizeof(BYTE));
    }

    void TearDown() override {
        free(CPU.MEM);
        free(CPU.PORT);
    }
};
