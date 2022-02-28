#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "Stage.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"


/*
 * doClockLow:
 * Performs the Decode stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];
    uint64_t valA = 0;
    uint64_t valB = 0; 
    uint64_t dstE = RNONE;
    uint64_t dstM = RNONE;
    uint64_t srcA = RNONE;
    uint64_t srcB = RNONE;
    setEInput(ereg, dreg -> getstat() -> getOutput(), dreg -> geticode() -> getOutput(), dreg -> getifun() -> getOutput(), dreg -> getvalC() -> getOutput(), valA, valB, dstE, dstM, srcA, srcB); 
    return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{

}

void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB) 
{

}
