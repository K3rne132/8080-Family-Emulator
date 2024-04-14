#include "pch.h"
#include "Intel8080FixtureTests.hpp"

TEST_F(Intel8080FixtureTests, RegisterPairsEndiannessTest) {
	// init
	InitializePairs();

	// check - all register pairs are in big endian order (due to register
	// ordering i.e. B = 0, C = 1, D = 2, E = 3)
	ASSERT_EQ(CPU.B, 0x01);
	ASSERT_EQ(CPU.C, 0x23);

	ASSERT_EQ(CPU.D, 0x45);
	ASSERT_EQ(CPU.E, 0x67);

	ASSERT_EQ(CPU.H, 0x89);
	ASSERT_EQ(CPU.L, 0xAB);

	ASSERT_EQ(CPU.F, 0xCD);
	ASSERT_EQ(CPU.A, 0xEF);
}

TEST_F(Intel8080FixtureTests, RegistersByteWordIndexingTest) {
	// init
	InitializePairs();

	// check
	ASSERT_EQ(CPU.B, CPU.REG[le_reg(REG_B)]);
	ASSERT_EQ(CPU.C, CPU.REG[le_reg(REG_C)]);
	ASSERT_EQ(CPU.D, CPU.REG[le_reg(REG_D)]);
	ASSERT_EQ(CPU.E, CPU.REG[le_reg(REG_E)]);
	ASSERT_EQ(CPU.H, CPU.REG[le_reg(REG_H)]);
	ASSERT_EQ(CPU.L, CPU.REG[le_reg(REG_L)]);
	ASSERT_EQ(CPU.F, CPU.REG[le_reg(REG_M)]); // flags <=> memory
	ASSERT_EQ(CPU.A, CPU.REG[le_reg(REG_A)]);

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

TEST_F(Intel8080FixtureTests, CarryBitInstructionsTest) {
	// init check
	ASSERT_EQ(CPU.status.C, 0);

	// then check
	ASSERT_EQ(cmc(&CPU), 1);
	ASSERT_EQ(CPU.status.C, 1);
	cmc(&CPU);
	ASSERT_EQ(CPU.status.C, 0);

	// then check
	ASSERT_EQ(stc(&CPU), 1);
	ASSERT_EQ(CPU.status.C, 1);
	stc(&CPU);
	ASSERT_EQ(CPU.status.C, 1);
}

TEST_F(Intel8080FixtureTests, SingleRegisterInstructionsTest) {
	// init
	CPU.MEM[CPU.PC] = 0b00000000; // 0 -> B (rest of bits does not matter here)
	
	// then check B
	ASSERT_EQ(inr(&CPU), 1);
	ASSERT_EQ(CPU.B, 1);
	ASSERT_EQ(CPU.F, 0b00000010);
	ASSERT_EQ(dcr(&CPU), 1);
	ASSERT_EQ(CPU.B, 0);
	ASSERT_EQ(CPU.F, 0b01000110);
	dcr(&CPU); // underflow
	ASSERT_EQ(CPU.B, 0xFF);
	ASSERT_EQ(CPU.F, 0b10010111);
	inr(&CPU); // overflow
	ASSERT_EQ(CPU.B, 0);
	ASSERT_EQ(CPU.F, 0b01010111);

	// init
	CPU.MEM[CPU.PC] = 0b00111000; // 7 -> A (rest of bits does not matter here)

	// then check A
	inr(&CPU);
	ASSERT_EQ(CPU.A, 1);
	dcr(&CPU);
	ASSERT_EQ(CPU.A, 0);

	// init
	CPU.MEM[CPU.PC] = 0b00110000; // 6 -> M (rest of bits does not matter here)
	CPU.HL = 0x8000;
	CPU.MEM[CPU.HL] = 100;

	// then check memory
	inr(&CPU);
	ASSERT_EQ(CPU.MEM[CPU.HL], 101);
	dcr(&CPU);
	ASSERT_EQ(CPU.MEM[CPU.HL], 100);

	// init (opcode does not matter)
	CPU.A = 0b10101010;

	// then check
	ASSERT_EQ(cma(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b01010101);
	cma(&CPU);
	ASSERT_EQ(CPU.A, 0b10101010);

	// init
	CPU.A = 0x91; // both decimal numbers

	// then check
	ASSERT_EQ(daa(&CPU), 1);
	ASSERT_EQ(CPU.A, 0x91); // nothing changed
	ASSERT_EQ(CPU.F, 0b10000010);

	// init
	CPU.A = 0x9C; // second hex will cause 9 to overflow

	// then check
	daa(&CPU); // second changed with overflow
	ASSERT_EQ(CPU.A, 0x02); // then 9 + 1 = Ah => 0
	ASSERT_EQ(CPU.F, 0b00010011);
}

TEST_F(Intel8080FixtureTests, DataTransferInstructionsTest) {
	// init
	CPU.MEM[CPU.PC] = 0b01011100; // dst = E, src = H
	CPU.H = 0x12;

	// then check
	mov(&CPU);
	ASSERT_EQ(CPU.E, 0x12);
	ASSERT_EQ(CPU.H, 0x12);

	// then check
	CPU.MEM[CPU.PC] = 0b01110011; // dst = M, src = E
	mov(&CPU);
	ASSERT_EQ(CPU.MEM[CPU.HL], 0x12);

	// then check
	CPU.MEM[CPU.PC] = 0b01001110; // dst = C, src = M
	mov(&CPU);
	ASSERT_EQ(CPU.C, 0x12);

	// init
	CPU.MEM[CPU.PC] = 0b00000000; // 0 = BC, other bits does not matter
	CPU.BC = 0xB000;
	CPU.DE = 0xD000;
	CPU.A = 0xEA;

	// then check
	ASSERT_EQ(stax(&CPU), 1);
	ASSERT_EQ(CPU.MEM[CPU.BC], 0xEA);

	// then check
	CPU.MEM[CPU.BC] = 0xAB;
	ASSERT_EQ(ldax(&CPU), 1);
	ASSERT_EQ(CPU.A, 0xAB);

	// init
	CPU.MEM[CPU.PC] = 0b00010000; // 1 = DE, other bits does not matter

	// then check
	stax(&CPU);
	ASSERT_EQ(CPU.MEM[CPU.BC], 0xAB);

	// then check
	CPU.MEM[CPU.DE] = 0xEA;
	ldax(&CPU);
	ASSERT_EQ(CPU.A, 0xEA);
}

TEST_F(Intel8080FixtureTests, RegisterOrMemoryToAccumulatorInstructionsTest) {
	// init
	CPU.MEM[CPU.PC] = REG_A; // rest of bits does not matter
	CPU.HL = 0x8000;
	CPU.MEM[CPU.HL] = 0x12;
	CPU.A = 0x10;

	// then check
	ASSERT_EQ(add(&CPU), 1); // A + A = 0x20
	ASSERT_EQ(CPU.A, 0x20);
	ASSERT_EQ(CPU.F, 0b00000010);
	ASSERT_EQ(adc(&CPU), 1); // A + A + 0 = 0x40
	ASSERT_EQ(CPU.A, 0x40);
	ASSERT_EQ(CPU.F, 0b00000010);

	// then check
	CPU.MEM[CPU.PC] = REG_M; // rest of bits does not matter
	add(&CPU); // A + M = 0x52
	ASSERT_EQ(CPU.A, 0x52);
	ASSERT_EQ(CPU.F, 0b00000010);
	CPU.status.C = 1;
	adc(&CPU); // A + M + 1 = 0x65
	ASSERT_EQ(CPU.A, 0x65);
	ASSERT_EQ(CPU.F, 0b00000110);

	// init
	CPU.MEM[CPU.PC] = REG_E; // rest of bits does not matter
	CPU.E = 0x1D;
	CPU.A = 0xFC;

	// then check
	ASSERT_EQ(cmp(&CPU), 1);
	ASSERT_EQ(CPU.F, 0b10010010);
	ASSERT_EQ(sub(&CPU), 1); // A - E = 0xDF
	ASSERT_EQ(CPU.A, 0xDF);
	ASSERT_EQ(CPU.F, 0b10010010);
	ASSERT_EQ(sbb(&CPU), 1); // A - E + 0 = 0xC2
	ASSERT_EQ(CPU.A, 0xC2);
	ASSERT_EQ(CPU.F, 0b10000010);

	// then check
	CPU.MEM[CPU.PC] = REG_M; // rest of bits does not matter
	cmp(&CPU);
	ASSERT_EQ(CPU.F, 0b10000010);
	sub(&CPU); // A - M = 0xB0
	ASSERT_EQ(CPU.A, 0xB0);
	ASSERT_EQ(CPU.F, 0b10000010);
	CPU.status.C = 1;
	sbb(&CPU); // A - M + 1 = 0x9F
	ASSERT_EQ(CPU.A, 0x9F);
	ASSERT_EQ(CPU.F, 0b10010110);

	// init
	CPU.MEM[CPU.PC] = REG_B;
	CPU.A = 0x4B;
	CPU.B = 0xEA;
	CPU.C = 0x9C;
	CPU.D = 0x89;

	// then check
	ASSERT_EQ(ana(&CPU), 1);
	ASSERT_EQ(CPU.A, 0x4A);

	CPU.MEM[CPU.PC] = REG_C;
	ASSERT_EQ(xra(&CPU), 1);
	ASSERT_EQ(CPU.A, 0xD6);

	CPU.MEM[CPU.PC] = REG_D;
	ASSERT_EQ(ora(&CPU), 1);
	ASSERT_EQ(CPU.A, 0xDF);
}

TEST_F(Intel8080FixtureTests, RotateAccumulatorInstructionsTest) {
	// init
	CPU.A = 0b11110010;

	// check
	ASSERT_EQ(rlc(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b11100101);
	ASSERT_EQ(CPU.status.C, 1);

	// init
	CPU.A = 0b11110010;

	// check
	ASSERT_EQ(rrc(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b01111001);
	ASSERT_EQ(CPU.status.C, 0);

	// init
	CPU.A = 0b10110101;
	CPU.status.C = 0;

	// check
	ASSERT_EQ(ral(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b01101010);
	ASSERT_EQ(CPU.status.C, 1);

	// init
	CPU.A = 0b01101010;
	CPU.status.C = 1;

	// check
	ASSERT_EQ(rar(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b10110101);
	ASSERT_EQ(CPU.status.C, 0);
}

TEST_F(Intel8080FixtureTests, RegisterPairInstructionsTest) {
	// init
	InitializePairs();
	CPU.SP = 0x8000;
	CPU.MEM[CPU.PC] = 0b00000000; // BC

	// check
	ASSERT_EQ(push(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x7FFE);
	ASSERT_EQ(GetWORDFromMemory(CPU.SP), 0x0123);

	CPU.BC = 0;
	ASSERT_EQ(pop(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x8000);
	ASSERT_EQ(CPU.BC, 0x0123);

	// when SP not padded to WORD
	CPU.SP++;
	CPU.MEM[CPU.PC] = 0b00010000; // DE

	// check
	ASSERT_EQ(push(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x7FFF);
	ASSERT_EQ(GetWORDFromMemory(CPU.SP), 0x4567);

	CPU.BC = 0;
	ASSERT_EQ(pop(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x8001);
	ASSERT_EQ(CPU.DE, 0x4567);

	// init
	CPU.F = 0b00000010;

	// check
	ASSERT_EQ(dad(&CPU), 1); // HL = DE + HL
	ASSERT_EQ(CPU.HL, 0xCF12);
	ASSERT_EQ(CPU.F, 0b00000010);

	// init
	CPU.MEM[CPU.PC] = 0b00110000; // SP
	CPU.SP = 0x8000;

	// check
	ASSERT_EQ(inx(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x8001);
	ASSERT_EQ(dcx(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x8000);

	// init
	CPU.HL = 0x1234;
	CPU.DE = 0xABCD;
	SetWORDInMemory(0x9876, CPU.SP);

	// check
	ASSERT_EQ(xchg(&CPU), 1);
	ASSERT_EQ(CPU.HL, 0xABCD);
	ASSERT_EQ(CPU.DE, 0x1234);
	ASSERT_EQ(xthl(&CPU), 1);
	ASSERT_EQ(CPU.HL, 0x9876);
	ASSERT_EQ(GetWORDFromMemory(CPU.SP), 0xABCD);
	ASSERT_EQ(sphl(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x9876);
}

TEST_F(Intel8080FixtureTests, JumpInstructionsTest) {
	// init
	SetWORDInMemory(0x1234, 0x0001); // opcode does not matter here
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
	SetWORDInMemory(0x1234, CPU.PC + 1); // opcode does not matter here
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
	SetWORDInMemory(0x1234, CPU.SP);
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
		SetWORDInMemory(0x1234, CPU.SP);
		CPU.F = 0;

		// then check for unset flag
		CPU.PC += return_cc[i * 2](&CPU); // flag
		ASSERT_EQ(CPU.PC, 0x0001);
		CPU.PC = 0;
		SetWORDInMemory(0x1234, CPU.SP);
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
		SetWORDInMemory(0x1234, CPU.SP);
		CPU.PC += return_cc[i * 2](&CPU); // flag
		ASSERT_EQ(CPU.PC, 0x1234);
		CPU.PC = 0;
		SetWORDInMemory(0x1234, CPU.SP);
		CPU.PC += return_cc[i * 2 + 1](&CPU); // not flag
		ASSERT_EQ(CPU.PC, 0x0001);
	}
}
