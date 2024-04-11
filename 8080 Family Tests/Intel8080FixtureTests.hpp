#pragma once
#include "pch.h"
#include "i8080.h"

class Intel8080FixtureTests : public testing::Test {
protected:
    INTEL_8080 CPU;

    void InitializePairs() {
        CPU.BC = 0x0123;
        CPU.DE = 0x4567;
        CPU.HL = 0x89AB;
        CPU.PSW = 0xCDEF;
    }
    
    void SetUp() override {
        std::memset(&CPU, 0, sizeof(CPU));
    }
};
