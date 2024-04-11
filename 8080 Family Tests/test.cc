#include "pch.h"
#include "Intel8080FixtureTests.hpp"

TEST_F(Intel8080FixtureTests, RegisterPairsEndiannessTest) {
	// init
	InitializePairs();

	// check
	ASSERT_EQ(CPU.B, 0x23);
	ASSERT_EQ(CPU.C, 0x01);

	ASSERT_EQ(CPU.D, 0x67);
	ASSERT_EQ(CPU.E, 0x45);

	ASSERT_EQ(CPU.H, 0xAB);
	ASSERT_EQ(CPU.L, 0x89);

	ASSERT_EQ(CPU.F, 0xEF);
	ASSERT_EQ(CPU.A, 0xCD);
}

TEST_F(Intel8080FixtureTests, RegistersByteWordIndexingTest) {
	// init
	InitializePairs();

	// check
	ASSERT_EQ(CPU.B, CPU.reg_b[REG_B]);
	ASSERT_EQ(CPU.C, CPU.reg_b[REG_C]);
	ASSERT_EQ(CPU.D, CPU.reg_b[REG_D]);
	ASSERT_EQ(CPU.E, CPU.reg_b[REG_E]);
	ASSERT_EQ(CPU.H, CPU.reg_b[REG_H]);
	ASSERT_EQ(CPU.L, CPU.reg_b[REG_L]);
	ASSERT_EQ(CPU.F, CPU.reg_b[REG_M]); // flags <=> memory
	ASSERT_EQ(CPU.A, CPU.reg_b[REG_A]);

	ASSERT_EQ(CPU.BC,  CPU.reg_w[REG_PAIR_BC]);
	ASSERT_EQ(CPU.DE,  CPU.reg_w[REG_PAIR_DE]);
	ASSERT_EQ(CPU.HL,  CPU.reg_w[REG_PAIR_HL]);
	ASSERT_EQ(CPU.PSW, CPU.reg_w[REG_PAIR_SP]); // psw <=> sp
}

TEST_F(Intel8080FixtureTests, StatusFlagTest) {
	// init
	CPU.F = 0b01010001;

	// check
	ASSERT_EQ(CPU.status.S,  0);
	ASSERT_EQ(CPU.status.Z,  1);
	ASSERT_EQ(CPU.status.AC, 1);
	ASSERT_EQ(CPU.status.P,  0);
	ASSERT_EQ(CPU.status.C,  1);

	CPU.F = 0;
	CPU.status.S  = 1;
	ASSERT_EQ(CPU.F, 0b10000000);

	CPU.status.Z  = 1;
	ASSERT_EQ(CPU.F, 0b11000000);

	CPU.status.AC = 1;
	ASSERT_EQ(CPU.F, 0b11010000);

	CPU.status.P  = 1;
	ASSERT_EQ(CPU.F, 0b11010100);

	CPU.status.C  = 1;
	ASSERT_EQ(CPU.F, 0b11010101);
}

TEST_F(Intel8080FixtureTests, CarryBitInstructionsTest) {
	
}

TEST_F(Intel8080FixtureTests, SingleRegisterInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, DataTransferInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, RegisterOrMemoryToAccumulatorInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, RotateAccumulatorInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, RegisterPairInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, RImmediateInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, DirectAddressingInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, JumpInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, CallInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, ReturnInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, RSTInstructionTest) {

}

TEST_F(Intel8080FixtureTests, InterruptFlipFlopInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, InputOutputInstructionsTest) {

}

TEST_F(Intel8080FixtureTests, HLTInstructionTest) {

}
