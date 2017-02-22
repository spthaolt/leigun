#include <stdint.h>
typedef void MCS51_InstructionProc(void);

typedef struct MCS51_Instruction {
	MCS51_InstructionProc *iproc;
	int cycles;
	uint8_t icode;
	uint8_t mask;
	char *name;
	int len;
} MCS51_Instruction;

extern MCS51_InstructionProc **mcs51_iProcTab;
extern MCS51_Instruction **mcs51_instrTab;

static inline MCS51_InstructionProc *
MCS51_InstructionProcFind(uint16_t icode)
{
	return mcs51_iProcTab[icode];
}

static inline MCS51_Instruction *
MCS51_InstructionFind(uint16_t icode)
{
	return mcs51_instrTab[icode];
}

void MCS51_IDecoderNew(unsigned int cycles_multiplicator);
