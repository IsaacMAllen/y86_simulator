#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Stage.h"
#include "WritebackStage.h"
#include "Status.h"
#include "Debug.h"
#include "W.h"
#include "Instructions.h"

/*
 * doClockLow:
 * Performs the Writeback stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    W * wreg = (W *) pregs[WREG];
     
    bool isHlt = wreg -> geticode() -> getOutput() == IHALT;
    return isHlt;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void WritebackStage::doClockHigh(PipeReg ** pregs)
{
    bool error = false;
    W * wreg = (W *) pregs[WREG];
    RegisterFile * reggiejr = RegisterFile::getInstance();
    uint64_t W_icode = wreg->geticode()->getOutput();

    if(!(W_icode == IRMMOVQ || W_icode == ICALL)){
	reggiejr->writeRegister(wreg->getvalE()->getOutput(), (int32_t) wreg->getdstE()->getOutput(), error); 
    }
    
	reggiejr->writeRegister(wreg->getvalM()->getOutput(), (int32_t) wreg->getdstM()->getOutput(), error); 
    
}

