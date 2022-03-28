#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "M.h"
#include "E.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

/*
 * doClockLow:
 * Performs the Execute stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];

    uint64_t cnd = 0;
    uint64_t valE = ereg->getvalC()->getOutput();

    setMInput(mreg, ereg -> getstat() -> getOutput(), ereg -> geticode() -> getOutput(), cnd, valE, ereg -> getvalA() -> getOutput(), ereg -> getdstE() -> getOutput(), ereg -> getdstM() -> getOutput());

    return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
	M * mreg = (M *) pregs[MREG];

	mreg->getstat()->normal();
	mreg->geticode()->normal();
	mreg->getCnd()->normal();
	mreg->getvalE()->normal();
	mreg->getvalA()->normal();
	mreg->getdstE()->normal();
	mreg->getdstM()->normal();


}


void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM) {

	mreg->getstat()->setInput(stat);
	mreg->geticode()->setInput(icode);
	mreg->getCnd()->setInput(cnd);
	mreg->getvalE()->setInput(valE);
	mreg->getvalA()->setInput(valA);
	mreg->getdstE()->setInput(dstE);
	mreg->getdstM()->setInput(dstM);
	


}

uint64_t getAluA(E * ereg, uint64_t E_icode) {
    if(E_icode == IRRMOVQ || E_icode == IOPQ) return ereg->getvalA()->getOutput();
    if(E_icode == IIRMOVQ || E_icode == IRMMOVQ || E_icode == IMRMOVQ) return ereg->getvalC()->getOutput();
    if(E_icode == ICALL || E_icode == IPUSHQ) return -8;
    if(E_icode == IRET || E_icode == IPOPQ) return 8;
    return 0;

}

uint64_t getAluB(E * ereg, uint64_t E_icode) {
    if(E_icode == IRMMOVQ || E_icode == IMRMOVQ || E_icode == IOPQ || E_icode == ICALL || E_icode == IPUSHQ || E_icode == IRET || E_icode == IPOPQ) return ereg->getvalB()->getOutput();
    return 0;
}

// Returns e_icode
uint64_t getAluFun(E * ereg, uint64_t E_icode) {
    if (E_icode == IOPQ) return ereg->getifun()->getOutput();
    return ADDQ;
}
// If True CC will be used to set the Condition Codes
bool setcc(uint64_t E_icode) {
    return E_icode == IOPQ;
}

uint64_t eDstE(E * ereg, uint64_t E_icode, uint64_t e_Cnd) {
    if(E_icode == IRRMOVQ && !e_Cnd) return RNONE;
    return ereg->getdstE()->getOutput();

}

uint64_t performOp(uint64_t e_icode, int64_t val_rA, int64_t val_rB, bool & error) {
    uint64_t result = 0;
    switch(e_icode) {
	case ADDQ:
	    result = val_rA + val_rB;
	    // TODO: Logic for CC setting
	    if((val_rA >= 0 && val_rB >= 0 || val_rA < 0 && val_rB < 0) && (result < 0 && val_rA >= 0 || result >= 0 && val_rA < 0)){
		ConditionCodes::setConditionCodes(1,OF,error);
	    }
	    else {
		ConditionCodes::setConditionCodes(0,OF,error);
	    }
	    break;
	case SUBQ:
	    result = val_rA - val_rB;
	    // TODO: Logic for CC setting
	    break;
	case XORQ:
	    result = val_rA ^ val_rB;
	    // TODO: Logic for CC setting
	    break;
	case ANDQ:
	    result = val_rA & val_rB;
	    // TODO: Logic for CC setting
	    break;
    }

    if(result == 0) {
	ConditionCodes::setConditionCodes(1,ZF,error);
    }
    else {
	ConditionCodes::setConditionCodes(0,ZF,error);	
    }

    if(result < 0) {
	ConditionCodes::setConditionCodes(1,SF,error);
    }
    else {
	ConditionCodes::setConditionCodes(0,SF,error);
    }

    

    return result;
}
