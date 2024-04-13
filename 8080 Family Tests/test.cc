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
	ASSERT_EQ(CPU.B, CPU.REG[REG_B]);
	ASSERT_EQ(CPU.C, CPU.REG[REG_C]);
	ASSERT_EQ(CPU.D, CPU.REG[REG_D]);
	ASSERT_EQ(CPU.E, CPU.REG[REG_E]);
	ASSERT_EQ(CPU.H, CPU.REG[REG_H]);
	ASSERT_EQ(CPU.L, CPU.REG[REG_L]);
	ASSERT_EQ(CPU.F, CPU.REG[REG_M]); // flags <=> memory
	ASSERT_EQ(CPU.A, CPU.REG[REG_A]);

	ASSERT_EQ(CPU.BC,  CPU.REG_W[REG_PAIR_BC]);
	ASSERT_EQ(CPU.DE,  CPU.REG_W[REG_PAIR_DE]);
	ASSERT_EQ(CPU.HL,  CPU.REG_W[REG_PAIR_HL]);
	ASSERT_EQ(CPU.PSW, CPU.REG_W[REG_PAIR_SP]); // psw <=> sp
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

TEST_F(Intel8080FixtureTests, JumpInstructionsTest) {
	// init
	SetWORDInMemory(0x34, 0x12, 0x0001); // opcode does not matter here
	INSTRUCTION jump_cc[8] = { jc, jnc, jz, jnz, jm, jp, jpe, jpo };
	const char* cc_name[4] = { "CARRY", "ZERO", "SIGN", "PARITY" };
	
	// then
	CPU.PC += jmp(&CPU);

	// check
	ASSERT_EQ(CPU.PC, 0x1234);

	for (BYTE i = 0; i < 4; i++) {
		std::cerr << "Testing " << cc_name[i] << " flag jump\n";
		// init for jump conditions
		CPU.PC = 0;
		CPU.F = 0;

		// then check for unset flag
		CPU.PC += jump_cc[i * 2](&CPU); // flag
		ASSERT_EQ(CPU.PC, 0x0003);
		CPU.PC = 0;
		CPU.PC += jump_cc[i * 2 + 1](&CPU); // not flag
		ASSERT_EQ(CPU.PC, 0x1234);

		// set proper flag
		switch (i) {
		case 0: CPU.status.C = 1; break; // CARRY
		case 1: CPU.status.Z = 1; break; // ZERO
		case 2: CPU.status.S = 1; break; // SIGN
		case 3: CPU.status.P = 1; break; // PARITY
		}

		// then check for set flag
		CPU.PC = 0;
		CPU.PC += jump_cc[i * 2](&CPU); // flag
		ASSERT_EQ(CPU.PC, 0x1234);
		CPU.PC = 0;
		CPU.PC += jump_cc[i * 2 + 1](&CPU); // not flag
		ASSERT_EQ(CPU.PC, 0x0003);
	}
}

TEST_F(Intel8080FixtureTests, CallSubroutineInstructionsTest) {
	// init current PC at address 0x5678
	CPU.PC = 0x5678;
	const WORD next_inst_addr = 0x5678 + 3;
	SetWORDInMemory(0x34, 0x12, CPU.PC + 1); // opcode does not matter here
	INSTRUCTION call_cc[8] = { cc, cnc, cz, cnz, cm, cp, cpe, cpo };
	const char* cc_name[4] = { "CARRY", "ZERO", "SIGN", "PARITY" };

	// then
	CPU.PC += call(&CPU);

	// check
	ASSERT_EQ(CPU.PC, 0x1234);
	ASSERT_EQ(GetWORDFromMemory(CPU.SP), next_inst_addr);

	for (BYTE i = 0; i < 4; i++) {
		std::cerr << "Testing " << cc_name[i] << " flag call\n";
		// init for call conditions
		CPU.PC = 0x5678;
		CPU.F = 0;

		// then check for unset flag
		CPU.PC += call_cc[i * 2](&CPU); // flag
		ASSERT_EQ(CPU.PC, next_inst_addr);
		CPU.PC = 0x5678;
		CPU.PC += call_cc[i * 2 + 1](&CPU); // not flag
		ASSERT_EQ(CPU.PC, 0x1234);
		ASSERT_EQ(GetWORDFromMemory(CPU.SP), next_inst_addr);

		// set proper flag
		switch (i) {
		case 0: CPU.status.C = 1; break; // CARRY
		case 1: CPU.status.Z = 1; break; // ZERO
		case 2: CPU.status.S = 1; break; // SIGN
		case 3: CPU.status.P = 1; break; // PARITY
		}

		// then check for set flag
		CPU.PC = 0x5678;
		CPU.PC += call_cc[i * 2](&CPU); // flag
		ASSERT_EQ(CPU.PC, 0x1234);
		ASSERT_EQ(GetWORDFromMemory(CPU.SP), next_inst_addr);
		CPU.PC = 0x5678;
		CPU.PC += call_cc[i * 2 + 1](&CPU); // not flag
		ASSERT_EQ(CPU.PC, next_inst_addr);
	}
}

TEST_F(Intel8080FixtureTests, ReturnFromSubroutineInstructionsTest) {
	// init
	CPU.SP = 0x8000;
	SetWORDInMemory(0x34, 0x12, CPU.SP);
	INSTRUCTION return_cc[8] = { rc, rnc, rz, rnz, rm, rp, rpe, rpo };
	const char* cc_name[4] = { "CARRY", "ZERO", "SIGN", "PARITY" };

	// then
	CPU.PC += ret(&CPU);

	// check
	ASSERT_EQ(CPU.PC, 0x1234);

	for (BYTE i = 0; i < 4; i++) {
		std::cerr << "Testing " << cc_name[i] << " flag return\n";
		// init for return conditions
		CPU.PC = 0;
		SetWORDInMemory(0x34, 0x12, CPU.SP);
		CPU.F = 0;

		// then check for unset flag
		CPU.PC += return_cc[i * 2](&CPU); // flag
		ASSERT_EQ(CPU.PC, 0x0001);
		CPU.PC = 0;
		SetWORDInMemory(0x34, 0x12, CPU.SP);
		CPU.PC += return_cc[i * 2 + 1](&CPU); // not flag
		ASSERT_EQ(CPU.PC, 0x1234);

		// set proper flag
		switch (i) {
		case 0: CPU.status.C = 1; break; // CARRY
		case 1: CPU.status.Z = 1; break; // ZERO
		case 2: CPU.status.S = 1; break; // SIGN
		case 3: CPU.status.P = 1; break; // PARITY
		}

		// then check for set flag
		CPU.PC = 0;
		SetWORDInMemory(0x34, 0x12, CPU.SP);
		CPU.PC += return_cc[i * 2](&CPU); // flag
		ASSERT_EQ(CPU.PC, 0x1234);
		CPU.PC = 0;
		SetWORDInMemory(0x34, 0x12, CPU.SP);
		CPU.PC += return_cc[i * 2 + 1](&CPU); // not flag
		ASSERT_EQ(CPU.PC, 0x0001);
	}
}
