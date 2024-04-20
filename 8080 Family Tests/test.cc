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

	ASSERT_EQ(CPU.A, 0xCD);
	ASSERT_EQ(CPU.F, 0xEF);
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

TEST_F(Intel8080FixtureTests, CMC_STC_InstructionsTest) {
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

TEST_F(Intel8080FixtureTests, INR_DCR_InstructionsTest) {
	// init
	CPU.MEM[CPU.PC] = 0b00000000; // 0 -> B (rest of bits does not matter here)

	// then check B
	ASSERT_EQ(inr(&CPU), 1);
	ASSERT_EQ(CPU.B, 1);
	ASSERT_EQ(CPU.F, 0b00000010);
	ASSERT_EQ(dcr(&CPU), 1);
	ASSERT_EQ(CPU.B, 0);
	ASSERT_EQ(CPU.F, 0b01010110);
	dcr(&CPU); // underflow
	ASSERT_EQ(CPU.B, 0xFF);
	ASSERT_EQ(CPU.F, 0b10000110); // carry not affected
	inr(&CPU); // overflow
	ASSERT_EQ(CPU.B, 0);
	ASSERT_EQ(CPU.F, 0b01010110); // carry not affected

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
}

TEST_F(Intel8080FixtureTests, CMA_InstructionTest) {
	// init (opcode does not matter)
	CPU.A = 0b10101010;

	// then check
	ASSERT_EQ(cma(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b01010101);
	cma(&CPU);
	ASSERT_EQ(CPU.A, 0b10101010);
}

TEST_F(Intel8080FixtureTests, DAA_InstructionTest) {
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

TEST_F(Intel8080FixtureTests, MOV_InstructionTest) {
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
}

TEST_F(Intel8080FixtureTests, STAX_LDAX_InstructionsTest) {
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

TEST_F(Intel8080FixtureTests, ADD_ADC_InstructionsTest) {
	// init
	CPU.MEM[CPU.PC] = REG_A; // rest of bits does not matter
	CPU.A = 0x10;
	CPU.HL = 0x8000;
	CPU.MEM[CPU.HL] = 0x12;
	
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
}

TEST_F(Intel8080FixtureTests, SUB_SBB_CMP_InstructionsTest) {
	// init
	CPU.MEM[CPU.PC] = REG_E; // rest of bits does not matter
	CPU.E = 0x1D;
	CPU.A = 0xFC;
	CPU.HL = 0x8000;
	CPU.MEM[CPU.HL] = 0x12;

	// then check
	ASSERT_EQ(cmp(&CPU), 1);
	ASSERT_EQ(CPU.F, 0b10000010);
	ASSERT_EQ(sub(&CPU), 1); // A - E = 0xDF
	ASSERT_EQ(CPU.A, 0xDF);
	ASSERT_EQ(CPU.F, 0b10000010);
	ASSERT_EQ(sbb(&CPU), 1); // A - E - 0 = 0xC2
	ASSERT_EQ(CPU.A, 0xC2);
	ASSERT_EQ(CPU.F, 0b10010010);

	// then check
	CPU.MEM[CPU.PC] = REG_M; // rest of bits does not matter
	cmp(&CPU);
	ASSERT_EQ(CPU.F, 0b10010010);
	sub(&CPU); // A - M = 0xB0
	ASSERT_EQ(CPU.A, 0xB0);
	ASSERT_EQ(CPU.F, 0b10010010);
	CPU.status.C = 1;
	sbb(&CPU); // A - M - 1 = 0x9D
	ASSERT_EQ(CPU.A, 0x9D);
	ASSERT_EQ(CPU.F, 0b10000010);
}

TEST_F(Intel8080FixtureTests, ANA_XRA_ORA_InstructionsTest) {
	// init
	CPU.MEM[CPU.PC] = REG_B;
	CPU.A = 0x4B;
	CPU.B = 0xEA;
	CPU.C = 0x9C;
	CPU.D = 0x89;

	// then check
	ASSERT_EQ(ana(&CPU), 1);
	ASSERT_EQ(CPU.A, 0x4A);
	ASSERT_EQ(CPU.F, 0b00010010);

	CPU.MEM[CPU.PC] = REG_C;
	ASSERT_EQ(xra(&CPU), 1);
	ASSERT_EQ(CPU.A, 0xD6);
	ASSERT_EQ(CPU.F, 0b10000010);

	CPU.MEM[CPU.PC] = REG_D;
	ASSERT_EQ(ora(&CPU), 1);
	ASSERT_EQ(CPU.A, 0xDF);
	ASSERT_EQ(CPU.F, 0b10000010);
}

TEST_F(Intel8080FixtureTests, RLC_InstructionTest) {
	// init
	CPU.A = 0b11110010;

	// check
	ASSERT_EQ(rlc(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b11100101);
	ASSERT_EQ(CPU.status.C, 1);
}

TEST_F(Intel8080FixtureTests, RRC_InstructionTest) {
	// init
	CPU.A = 0b11110010;

	// check
	ASSERT_EQ(rrc(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b01111001);
	ASSERT_EQ(CPU.status.C, 0);
}

TEST_F(Intel8080FixtureTests, RAL_InstructionTest) {
	// init
	CPU.A = 0b10110101;
	CPU.status.C = 0;

	// check
	ASSERT_EQ(ral(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b01101010);
	ASSERT_EQ(CPU.status.C, 1);
}

TEST_F(Intel8080FixtureTests, RAR_InstructionTest) {
	// init
	CPU.A = 0b01101010;
	CPU.status.C = 1;

	// check
	ASSERT_EQ(rar(&CPU), 1);
	ASSERT_EQ(CPU.A, 0b10110101);
	ASSERT_EQ(CPU.status.C, 0);
}

TEST_F(Intel8080FixtureTests, PUSH_POP_InstructionsTest) {
	// init
	InitializePairs();
	CPU.SP = 0x8000;
	CPU.MEM[CPU.PC] = 0b00000000; // BC

	// check
	ASSERT_EQ(push(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x7FFE);
	ASSERT_EQ(GetWORDFromMemory(CPU.SP), CPU.BC);
	ASSERT_EQ(CPU.MEM[CPU.SP], CPU.C);
	ASSERT_EQ(CPU.MEM[CPU.SP + 1], CPU.B);

	CPU.BC = 0;
	ASSERT_EQ(pop(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x8000);
	ASSERT_EQ(CPU.BC, 0x0123);

	// when SP not padded to WORD
	CPU.SP++;
	CPU.MEM[CPU.PC] = 0b00010000; // DE

	// check
	push(&CPU);
	ASSERT_EQ(CPU.SP, 0x7FFF);
	ASSERT_EQ(GetWORDFromMemory(CPU.SP), CPU.DE);
	ASSERT_EQ(CPU.MEM[CPU.SP], CPU.E);
	ASSERT_EQ(CPU.MEM[CPU.SP + 1], CPU.D);

	CPU.BC = 0;
	pop(&CPU);
	ASSERT_EQ(CPU.SP, 0x8001);
	ASSERT_EQ(CPU.DE, 0x4567);

	// check for PSW
	CPU.MEM[CPU.PC] = 0b00110000; // PSW
	push(&CPU);
	ASSERT_EQ(GetWORDFromMemory(CPU.SP), CPU.PSW);
	ASSERT_EQ(CPU.MEM[CPU.SP], CPU.F);
	ASSERT_EQ(CPU.MEM[CPU.SP + 1], CPU.A);
}

TEST_F(Intel8080FixtureTests, DAD_InstructionsTest) {
	// init
	InitializePairs();
	CPU.MEM[CPU.PC] = 0b00010000; // DE
	CPU.F = 0b00000010;

	// check
	ASSERT_EQ(dad(&CPU), 1); // HL = DE + HL
	ASSERT_EQ(CPU.HL, 0xCF12);
	ASSERT_EQ(CPU.status.C, 0);
}

TEST_F(Intel8080FixtureTests, INX_DCX_InstructionsTest) {
	// init
	CPU.MEM[CPU.PC] = 0b00110000; // SP
	CPU.SP = 0x8000;

	// check
	ASSERT_EQ(inx(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x8001);
	ASSERT_EQ(dcx(&CPU), 1);
	ASSERT_EQ(CPU.SP, 0x8000);
}

TEST_F(Intel8080FixtureTests, XCHG_XTHL_SPHL_InstructionsTest) {
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

TEST_F(Intel8080FixtureTests, LXI_InstructionTest) {
	// init
	CPU.MEM[CPU.PC] = 0b00000000; // register pair BC
	SetWORDInMemory(0xABCD, CPU.PC + 1); // immediate

	// check
	ASSERT_EQ(lxi(&CPU), 3);
	ASSERT_EQ(CPU.BC, 0xABCD);

	// then check
	CPU.MEM[CPU.PC] = 0b00110000; // register SP
	lxi(&CPU);
	ASSERT_EQ(CPU.SP, 0xABCD);
}

TEST_F(Intel8080FixtureTests, MVI_InstructionTest) {
	// init
	CPU.MEM[CPU.PC] = 0b00011000; // register E
	CPU.MEM[CPU.PC + 1] = 0x97; // immediate

	// check
	ASSERT_EQ(mvi(&CPU), 2);
	ASSERT_EQ(CPU.E, 0x97);

	// then check
	CPU.MEM[CPU.PC] = 0b00110000; // memory
	mvi(&CPU);
	ASSERT_EQ(CPU.MEM[CPU.HL], 0x97);
}

TEST_F(Intel8080FixtureTests, ADI_ACI_InstructionsTest) {
	// init
	CPU.MEM[CPU.PC + 1] = 0xAC; // immediate

	// check
	ASSERT_EQ(adi(&CPU), 2);
	ASSERT_EQ(CPU.A, 0xAC);
	ASSERT_EQ(CPU.F, 0b10000110);
	ASSERT_EQ(aci(&CPU), 2);
	ASSERT_EQ(CPU.A, 0x58);
	ASSERT_EQ(CPU.F, 0b00010011);

	// when carry is set
	aci(&CPU);
	ASSERT_EQ(CPU.A, 0x05);
	ASSERT_EQ(CPU.F, 0b00010111);
}

TEST_F(Intel8080FixtureTests, SUI_SBI_CPI_InstructionsTest) {
	// init
	CPU.MEM[CPU.PC + 1] = 0xD1; // immediate

	// check when carry is set
	ASSERT_EQ(cpi(&CPU), 2);
	ASSERT_EQ(CPU.A, 0x00);
	ASSERT_EQ(CPU.F, 0b00000011);
	ASSERT_EQ(sui(&CPU), 2);
	ASSERT_EQ(CPU.A, 0x2F);
	ASSERT_EQ(CPU.F, 0b00000011);
	ASSERT_EQ(sbi(&CPU), 2);
	ASSERT_EQ(CPU.A, 0x5D);
	ASSERT_EQ(CPU.F, 0b00010011);

	// when carry is reset
	CPU.status.C = 0;
	sbi(&CPU);
	ASSERT_EQ(CPU.A, 0x8C);
	ASSERT_EQ(CPU.F, 0b10010011);
}

TEST_F(Intel8080FixtureTests, ANI_XRI_ORI_InstructionsTest) {
	// init
	CPU.MEM[CPU.PC + 1] = 0x67; // immediate
	CPU.A = 0xBD;

	// then check
	ASSERT_EQ(ani(&CPU), 2);
	ASSERT_EQ(CPU.A, 0x25);
	ASSERT_EQ(CPU.F, 0b00010010);

	ASSERT_EQ(xri(&CPU), 2);
	ASSERT_EQ(CPU.A, 0x42);
	ASSERT_EQ(CPU.F, 0b00000110);

	ASSERT_EQ(ori(&CPU), 2);
	ASSERT_EQ(CPU.A, 0x67);
	ASSERT_EQ(CPU.F, 0b00000010);
}

TEST_F(Intel8080FixtureTests, STA_LDA_InstructionsTest) {
	// init
	SetWORDInMemory(0xA7B3, CPU.PC + 1);
	CPU.A = 0x34;

	// check
	ASSERT_EQ(sta(&CPU), 3);
	ASSERT_EQ(CPU.MEM[0xA7B3], 0x34);
	
	// then
	CPU.A = 0;

	// check
	ASSERT_EQ(lda(&CPU), 3);
	ASSERT_EQ(CPU.A, 0x34);
}

TEST_F(Intel8080FixtureTests, SHLD_LHLD_InstructionsTest) {
	// init
	SetWORDInMemory(0xB3C3, CPU.PC + 1);
	CPU.HL = 0x1234;

	// check
	ASSERT_EQ(shld(&CPU), 3);
	ASSERT_EQ(GetWORDFromMemory(0xB3C3), 0x1234);

	// then
	CPU.HL = 0;

	// check
	ASSERT_EQ(lhld(&CPU), 3);
	ASSERT_EQ(CPU.HL, 0x1234);
}

TEST_F(Intel8080FixtureTests, PCHL_InstructionTest) {
	// init
	CPU.HL = 0x1234;

	// check
	ASSERT_EQ(pchl(&CPU), 0);
	ASSERT_EQ(CPU.PC, 0x1234);
}

TEST_F(Intel8080FixtureTests, Jump_InstructionsTest) {
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

TEST_F(Intel8080FixtureTests, CallSubroutine_InstructionsTest) {
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

TEST_F(Intel8080FixtureTests, ReturnFromSubroutine_InstructionsTest) {
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

TEST_F(Intel8080FixtureTests, RST_InstructionTest) {
	// init
	CPU.MEM[CPU.PC] = 0b11000111; // RST 0

	// check
	ASSERT_EQ(rst(&CPU), 0);
	ASSERT_EQ(CPU.PC, 0x0000);
	ASSERT_EQ(CPU.SP, 0xFFFE);
	ASSERT_EQ(GetWORDFromMemory(CPU.SP), 0x0001);
}

TEST_F(Intel8080FixtureTests, EI_DI_InstructionTest) {
	// check
	ASSERT_EQ(ei(&CPU), 1);
	ASSERT_EQ(CPU.INT, 1);
	ASSERT_EQ(di(&CPU), 1);
	ASSERT_EQ(CPU.INT, 0);
}

TEST_F(Intel8080FixtureTests, HLT_InstructionTest) {
	// check
	ASSERT_EQ(hlt(&CPU), 1);
	ASSERT_EQ(CPU.HALT, 1);
}
