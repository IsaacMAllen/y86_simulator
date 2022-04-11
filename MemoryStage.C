#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Memory.h"
#include "Instructions.h"

uint64_t MemoryStage::valM;
uint64_t MemoryStage::stat;
bool memWrite(M * mreg);
bool memRead(M * mreg);
uint64_t memAddr(M * mreg);
uint64_t m_stat(bool mem_error, uint64_t M_stat);

/*
 * doClockLow:
 * Performs the Memory stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
	M * mreg = (M *) pregs[MREG];
	W * wreg = (W *) pregs[WREG];
	valM = 0;
	uint64_t addr = memAddr(mreg);
	bool mem_error = false;

	if(memRead(mreg)){
	    Memory * mem = Memory::getInstance();
	    valM = mem->getLong((int32_t)addr, mem_error);
	}
	else if(memWrite(mreg)){
	    Memory * mem = Memory::getInstance();
	    mem->putLong(mreg->getvalA()->getOutput(),addr, mem_error); 
	    
	}
	stat = m_stat(mem_error, mreg->getstat()->getOutput());
	setWInput(wreg, stat, mreg->geticode()->getOutput(), mreg->getvalE()->getOutput(), valM, mreg->getdstE()->getOutput(), mreg->getdstM()->getOutput());
	return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{

	W * wreg = (W *) pregs[WREG];

	wreg->getstat()->normal();
	wreg->geticode()->normal();
	wreg->getvalE()->normal();
	wreg->getvalM()->normal();
	wreg->getdstE()->normal();
	wreg->getdstM()->normal();
}

void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, uint64_t dstE, uint64_t dstM)
{
	wreg->getstat()->setInput(stat);
	wreg->geticode()->setInput(icode);
	wreg->getvalE()->setInput(valE);
	wreg->getvalM()->setInput(valM);
	wreg->getdstE()->setInput(dstE);
	wreg->getdstM()->setInput(dstM);


}

uint64_t MemoryStage::getm_valM() {
    return MemoryStage::valM;
}

uint64_t MemoryStage::getm_stat() {
    return MemoryStage::stat;
}

uint64_t memAddr(M * mreg) {

    uint64_t M_icode = mreg->geticode()->getOutput();

    if(M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL || M_icode == IMRMOVQ) return mreg->getvalE()->getOutput();
    if(M_icode == IPOPQ || M_icode == IRET) return mreg->getvalA()->getOutput();
    return 0;

}

bool memRead(M * mreg) {
    
    uint64_t M_icode = mreg->geticode()->getOutput();
    return (M_icode == IMRMOVQ || M_icode == IPOPQ || M_icode == IRET);
    
}

bool memWrite(M * mreg) {
 
    uint64_t M_icode = mreg->geticode()->getOutput();
    return (M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL);
}

uint64_t m_stat(bool mem_error, uint64_t M_stat) {
    if (mem_error) return SADR;
    return M_stat;
}
