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
#include "ConditionCodes.h"
#include "Tools.h"

/*
 * doClockLow:
 * Performs the Execute stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
*/

uint64_t performOp(uint64_t e_icode, uint64_t val_rA, uint64_t val_rB, bool & error);
bool setcc(uint64_t E_icode);
uint64_t getAluFun(E * ereg, uint64_t E_icode);
uint64_t getAluA(E * ereg, uint64_t E_icode);
uint64_t getAluB(E * ereg, uint64_t E_icode);

static uint64_t valE;
uint64_t dstE;

bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    
    bool error = false;
    uint64_t icode = ereg->geticode()->getOutput();
    uint64_t cnd = 0;
    valE = ereg->getvalC()->getOutput();
    dstE = ereg->getdstE()->getOutput();
    if (setcc(icode)){
	valE = performOp(ereg->getifun()->getOutput(), getAluA(ereg, icode), getAluB(ereg, icode), error);	
    }

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

uint64_t performOp(uint64_t e_ifun, uint64_t val_rA, uint64_t val_rB, bool & error) {
    /* I put logic for assigning cnd but I'm not sure if this is correct but I'm just gonna leave it commented out in some bizzare chance I was on the right track -Isaac */
    ConditionCodes * CC = ConditionCodes::getInstance();
    uint64_t result = 0;
    switch(e_ifun) {
	case ADDQ:
	    result = val_rA + val_rB;
	    // TODO: Logic for CC setting
	    if(((Tools::sign(val_rA) == 0 && Tools::sign(val_rB) == 0) || (Tools::sign(val_rA) == 1 && Tools::sign(val_rB) == 1)) &&
		    ((Tools::sign(result) == 1 && Tools::sign(val_rA) == 0) || (Tools::sign(result) == 0 && Tools::sign(val_rA) == 1))){
		CC->setConditionCode(true,OF,error);
		//cnd = Tools::setBits(cnd, OF, OF);
	    }
	    else {
		CC->setConditionCode(false,OF,error);
		//cnd = Tools::clearBits(cnd, OF, OF);
	    }
	    break;
	case SUBQ:
	    result = val_rB - val_rA;
	    // TODO: Logic for CC setting
	    if(((Tools::sign(val_rA) == 0 && Tools::sign(val_rB) == 1) || (Tools::sign(val_rA) == 1 && Tools::sign(val_rB) == 0)) 
		    && ((Tools::sign(result) == 1 && Tools::sign(val_rB) == 0) || (Tools::sign(result) == 0 && Tools::sign(val_rB) == 1))){
		CC->setConditionCode(true,OF,error);
		//cnd = Tools::setBits(cnd, OF, OF);
	    }
	    else {
		CC->setConditionCode(false,OF,error);
		//cnd = Tools::clearBits(cnd, OF, OF);
	    }
	    break;
	case XORQ:
	    result = val_rA ^ val_rB;
	    // TODO: Logic for CC setting
	    CC->setConditionCode(false,OF,error);
	    //cnd = Tools::clearBits(cnd, OF, OF);
	    break;
	case ANDQ:
	    result = val_rA & val_rB;
	    // TODO: Logic for CC setting
	    CC->setConditionCode(false,OF,error);
	    //cnd = Tools::clearBits(cnd, OF, OF);
	    break;
    }

    if(result == 0) {
	CC->setConditionCode(true,ZF,error);
	//cnd = Tools::setBits(cnd, ZF, ZF);
    }
    else {
	CC->setConditionCode(false,ZF,error);	
	//cnd = Tools::clearBits(cnd, ZF, ZF);
    }

    if(Tools::sign(result) == 1) {
	CC->setConditionCode(true,SF,error);
	//cnd = Tools::setBits(cnd, SF, SF);
    }
    else {
	CC->setConditionCode(false,SF,error);
	//cnd = Tools::clearBits(cnd, SF, SF);
    }
    return result;
}

uint64_t gete_valE(){
    return valE;
}

uint64_t gete_dstE(){
    return dstE;
}

