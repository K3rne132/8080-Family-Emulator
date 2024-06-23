#pragma once
#include "i8080.h"

enum HARDWARE_INTERRUPT {
	TRAP = 0x24,
	RST_75 = 0x3C,
	RST_65 = 0x34,
	RST_55 = 0x2C
};

#pragma pack(push, 1)

// Set Interrupt Mask bits
typedef struct _SIM_BITS {
	struct {
		uint8_t SOD : 1;
		uint8_t SOE : 1;
		uint8_t : 1;
		uint8_t RST75 : 1;
		uint8_t MSE : 1;
		uint8_t M75 : 1;
		uint8_t M65 : 1;
		uint8_t M55 : 1;
	};
	struct {
		uint8_t : 5;
		uint8_t MASKS : 3;
	};
} SIM_BITS;

// Read Interrupt Mask bits
typedef union _RIM_BITS {
	struct {
		uint8_t SID : 1;
		uint8_t P75 : 1;
		uint8_t P65 : 1;
		uint8_t P55 : 1;
		uint8_t IE : 1;
		uint8_t M75 : 1;
		uint8_t M65 : 1;
		uint8_t M55 : 1;
	};
	struct {
		uint8_t : 1;
		uint8_t PENDING : 3;
		uint8_t : 1;
		uint8_t MASKS : 3;
	};
} RIM_BITS;

// Intel 8085 registers set
typedef struct _INTEL_8085 {
	INTEL_8080 CORE;
	uint8_t SERIAL_IN;
	uint8_t SERIAL_OUT;
	RIM_BITS RIM;
	uint8_t TRAP;
	
} INTEL_8085;

#pragma pack(pop)

uint16_t sim(INTEL_8080* i8080);
uint16_t rim(INTEL_8080* i8080);

uint16_t arhl(INTEL_8080* i8080);
uint16_t dsub(INTEL_8080* i8080);
uint16_t jnui(INTEL_8080* i8080);
uint16_t jui(INTEL_8080* i8080);
uint16_t ldhi(INTEL_8080* i8080);
uint16_t ldsi(INTEL_8080* i8080);
uint16_t lhlx(INTEL_8080* i8080);
uint16_t rdel(INTEL_8080* i8080);
uint16_t rstv(INTEL_8080* i8080);
uint16_t shlx(INTEL_8080* i8080);



#ifdef E_I8085
const EMUL_STRUCT EMUL_DATA[256] = {
	{"NOP", 1, nop}, {"LXI B,%hXh", 3, lxi}, {"STAX B", 1, stax}, {"INX B", 1, inx},
	{"INR B", 1, inr}, {"DCR B", 1, dcr}, {"MVI B,%hhXh", 2, mvi}, {"RLC", 1, rlc},
	{"DSUB", 1, dsub}, {"DAD B", 1, dad}, {"LDAX B", 1, ldax}, {"DCX B", 1, dcx},
	{"INR C", 1, inr}, {"DCR C", 1, dcr}, {"MVI C,%hhXh", 2, mvi}, {"RRC", 1, rrc},
	{"ARHL", 1, arhl}, {"LXI D,%hXh", 3, lxi}, {"STAX D", 1, stax}, {"INX D", 1, inx},
	{"INR D", 1, inr}, {"DCR D", 1, dcr}, {"MVI D,%hhXh", 2, mvi}, {"RAL", 1, ral},
	{"RDEL", 1, rdel}, {"DAD D", 1, dad}, {"LDAX D", 1, ldax}, {"DCX D", 1, dcx},
	{"INR E", 1, inr}, {"DCR E", 1, dcr}, {"MVI E,%hhXh", 2, mvi}, {"RAR", 1, rar},
	{"RIM", 1, rim}, {"LXI H,%hXh", 3, lxi}, {"SHLD %hXh", 3, shld}, {"INX H", 1, inx},
	{"INR H", 1, inr}, {"DCR H", 1, dcr}, {"MVI H,%hhXh", 2, mvi}, {"DAA", 1, daa},
	{"LDHI L,%hhXh", 2, ldhi}, {"DAD H", 1, dad}, {"LHLD %hXh", 3, lhld}, {"DCX H", 1, dcx},
	{"INR L", 1, inr}, {"DCR L", 1, dcr}, {"MVI L,%hhXh", 2, mvi}, {"CMA", 1, cma},
	{"SIM", 1, sim}, {"LXI SP,%hXh", 3, lxi}, {"STA %hXh", 3, sta}, {"INX SP", 1, inx},
	{"INR M", 1, inr}, {"DCR M", 1, dcr}, {"MVI M,%hhXh", 2, mvi}, {"STC", 1, stc},
	{"LDSI L,%hhXh", 2, ldsi}, {"DAD SP", 1, dad}, {"LDA %hXh", 3, lda}, {"DCX SP", 1, dcx},
	{"INR A", 1, inr}, {"DCR A", 1, dcr}, {"MVI A,%hhXh", 2, mvi}, {"CMC", 1, cmc},
	{"MOV B,B", 1, mov}, {"MOV B,C", 1, mov}, {"MOV B,D", 1, mov}, {"MOV B,E", 1, mov},
	{"MOV B,H", 1, mov}, {"MOV B,L", 1, mov}, {"MOV B,M", 1, mov}, {"MOV B,A", 1, mov},
	{"MOV C,B", 1, mov}, {"MOV C,C", 1, mov}, {"MOV C,D", 1, mov}, {"MOV C,E", 1, mov},
	{"MOV C,H", 1, mov}, {"MOV C,L", 1, mov}, {"MOV C,M", 1, mov}, {"MOV C,A", 1, mov},
	{"MOV D,B", 1, mov}, {"MOV D,C", 1, mov}, {"MOV D,D", 1, mov}, {"MOV D,E", 1, mov},
	{"MOV D,H", 1, mov}, {"MOV D,L", 1, mov}, {"MOV D,M", 1, mov}, {"MOV D,A", 1, mov},
	{"MOV E,B", 1, mov}, {"MOV E,C", 1, mov}, {"MOV E,D", 1, mov}, {"MOV E,E", 1, mov},
	{"MOV E,H", 1, mov}, {"MOV E,L", 1, mov}, {"MOV E,M", 1, mov}, {"MOV E,A", 1, mov},
	{"MOV H,B", 1, mov}, {"MOV H,C", 1, mov}, {"MOV H,D", 1, mov}, {"MOV H,E", 1, mov},
	{"MOV H,H", 1, mov}, {"MOV H,L", 1, mov}, {"MOV H,M", 1, mov}, {"MOV H,A", 1, mov},
	{"MOV L,B", 1, mov}, {"MOV L,C", 1, mov}, {"MOV L,D", 1, mov}, {"MOV L,E", 1, mov},
	{"MOV L,H", 1, mov}, {"MOV L,L", 1, mov}, {"MOV L,M", 1, mov}, {"MOV L,A", 1, mov},
	{"MOV M,B", 1, mov}, {"MOV M,C", 1, mov}, {"MOV M,D", 1, mov}, {"MOV M,E", 1, mov},
	{"MOV M,H", 1, mov}, {"MOV M,L", 1, mov}, {"HLT", 1, hlt}, {"MOV M,A", 1, mov},
	{"MOV A,B", 1, mov}, {"MOV A,C", 1, mov}, {"MOV A,D", 1, mov}, {"MOV A,E", 1, mov},
	{"MOV A,H", 1, mov}, {"MOV A,L", 1, mov}, {"MOV A,M", 1, mov}, {"MOV A,A", 1, mov},
	{"ADD B", 1, add}, {"ADD C", 1, add}, {"ADD D", 1, add}, {"ADD E", 1, add},
	{"ADD H", 1, add}, {"ADD L", 1, add}, {"ADD M", 1, add}, {"ADD A", 1, add},
	{"ADC B", 1, adc}, {"ADC C", 1, adc}, {"ADC D", 1, adc}, {"ADC E", 1, adc},
	{"ADC H", 1, adc}, {"ADC L", 1, adc}, {"ADC M", 1, adc}, {"ADC A", 1, adc},
	{"SUB B", 1, sub}, {"SUB C", 1, sub}, {"SUB D", 1, sub}, {"SUB E", 1, sub},
	{"SUB H", 1, sub}, {"SUB L", 1, sub}, {"SUB M", 1, sub}, {"SUB A", 1, sub},
	{"SBB B", 1, sbb}, {"SBB C", 1, sbb}, {"SBB D", 1, sbb}, {"SBB E", 1, sbb},
	{"SBB H", 1, sbb}, {"SBB L", 1, sbb}, {"SBB M", 1, sbb}, {"SBB A", 1, sbb},
	{"ANA B", 1, ana}, {"ANA C", 1, ana}, {"ANA D", 1, ana}, {"ANA E", 1, ana},
	{"ANA H", 1, ana}, {"ANA L", 1, ana}, {"ANA M", 1, ana}, {"ANA A", 1, ana},
	{"XRA B", 1, xra}, {"XRA C", 1, xra}, {"XRA D", 1, xra}, {"XRA E", 1, xra},
	{"XRA H", 1, xra}, {"XRA L", 1, xra}, {"XRA M", 1, xra}, {"XRA A", 1, xra},
	{"ORA B", 1, ora}, {"ORA C", 1, ora}, {"ORA D", 1, ora}, {"ORA E", 1, ora},
	{"ORA H", 1, ora}, {"ORA L", 1, ora}, {"ORA M", 1, ora}, {"ORA A", 1, ora},
	{"CMP B", 1, cmp}, {"CMP C", 1, cmp}, {"CMP D", 1, cmp}, {"CMP E", 1, cmp},
	{"CMP H", 1, cmp}, {"CMP L", 1, cmp}, {"CMP M", 1, cmp}, {"CMP A", 1, cmp},
	{"RNZ", 1, rnz}, {"POP B", 1, pop}, {"JNZ %hXh", 3, jnz}, {"JMP %hXh", 3, jmp},
	{"CNZ %hXh", 3, cnz}, {"PUSH B", 1, push}, {"ADI %hhXh", 2, adi}, {"RST 0", 1, rst},
	{"RZ", 1, rz}, {"RET", 1, ret}, {"JZ %hXh", 3, jz}, {"RSTV", 1, rstv},
	{"CZ %hXh", 3, cz}, {"CALL %hXh", 3, call}, {"ACI %hhXh", 2, aci}, {"RST 1", 1, rst},
	{"RNC", 1, rnc}, {"POP D", 1, pop}, {"JNC %hXh", 3, jnc}, {"OUT %hhXh", 2, out},
	{"CNC %hXh", 3, cnc}, {"PUSH D", 1, push}, {"SUI %hhXh", 2, sui}, {"RST 2", 1, rst},
	{"RC", 1, rc}, {"SHLX", 1, shlx}, {"JC %hXh", 3, jc}, {"IN %hhXh", 2, in},
	{"CC %hXh", 3, cc}, {"JNUI %hXh", 3, jnui}, {"SBI %hhXh", 2, sbi}, {"RST 3", 1, rst},
	{"RPO", 1, rpo}, {"POP H", 1, pop}, {"JPO %hXh", 3, jpo}, {"XTHL", 1, xthl},
	{"CPO %hXh", 3, cpo}, {"PUSH H", 1, push}, {"ANI %hhXh", 2, ani}, {"RST 4", 1, rst},
	{"RPE", 1, rpe}, {"PCHL", 1, pchl}, {"JPE %hXh", 3, jpe}, {"XCHG", 1, xchg},
	{"CPE %hXh", 3, cpe}, {"LHLX", 1, lhlx}, {"XRI %hhXh", 2, xri}, {"RST 5", 1, rst},
	{"RP", 1, rp}, {"POP PSW", 1, pop}, {"JP %hXh", 3, jp}, {"DI", 1, di},
	{"CP %hXh", 3, cp}, {"PUSH PSW", 1, push}, {"ORI %hhXh", 2, ori}, {"RST 6", 1, rst},
	{"RM", 1, rm}, {"SPHL", 1, sphl}, {"JM %hXh", 3, jm}, {"EI", 1, ei},
	{"CM %hXh", 3, cm}, {"JUI %hXh", 3, jui}, {"CPI %hhXh", 2, cpi}, {"RST 7", 1, rst}
};
#endif
