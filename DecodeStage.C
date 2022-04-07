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
#include "Instructions.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"

uint64_t getSrcA(D * dreg);
uint64_t getSrcB(D * dreg); 
uint64_t getDstE(D * dreg); 
uint64_t getDstM(D * dreg); 
uint64_t getValA(D *dreg, bool & error);
uint64_t getValB(D * dreg, bool & error);
uint64_t dvalA(D * dreg, PipeReg ** pregs);
uint64_t dvalB(D * dreg, PipeReg ** pregs);

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
    uint64_t valA = dvalA(dreg,pregs);
    uint64_t valB = dvalB(dreg,pregs); 
    uint64_t dstE = getDstE(dreg);
    uint64_t dstM = getDstM(dreg);
    setEInput(ereg, dreg -> getstat() -> getOutput(), dreg -> geticode() -> getOutput(), dreg -> getifun() -> getOutput(), dreg -> getvalC() -> getOutput(), valA, valB, dstE, dstM, getSrcA(dreg), getSrcB(dreg)); 
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
    E * ereg = (E *) pregs[EREG];
    
    ereg -> getstat()->normal();
    ereg -> geticode()->normal();
    ereg -> getifun()->normal();
    ereg -> getvalC()->normal();
    ereg -> getvalA()->normal();
    ereg -> getvalB()->normal();
    ereg -> getdstE()->normal();
    ereg -> getdstM()->normal();
    ereg -> getsrcA()->normal();
    ereg -> getsrcB()->normal();
    

}

void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB) 
{
    ereg -> getstat()->setInput(stat);
    ereg -> geticode()->setInput(icode);
    ereg -> getifun()->setInput(ifun);
    ereg -> getvalC()->setInput(valC);
    ereg -> getvalA()->setInput(valA);
    ereg -> getvalB()->setInput(valB);
    ereg -> getdstE()->setInput(dstE);
    ereg -> getdstM()->setInput(dstM);
    ereg -> getsrcA()->setInput(srcA);
    ereg -> getsrcB()->setInput(srcB);

}

uint64_t getSrcA(D * dreg) {
	
	uint64_t d_icode = dreg->geticode()->getOutput();

	if (d_icode == IRRMOVQ || d_icode == IRMMOVQ || d_icode == IOPQ 	
	    || d_icode == IPUSHQ) {
	    return dreg->getrA()->getOutput();
	}
	
	if (d_icode == IPOPQ || d_icode == IRET) {
	    return RSP;
	}

	return RNONE;


}

uint64_t getSrcB(D * dreg) {
 
    uint64_t d_icode = dreg->geticode()->getOutput();

    if (d_icode == IOPQ || d_icode == IRMMOVQ || d_icode == IMRMOVQ) {
	return dreg->getrB()->getOutput();
    }

    if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET) {
	return RSP;
    }

    return RNONE;
}

uint64_t getDstE(D * dreg) {
    
    uint64_t d_icode = dreg->geticode()->getOutput();
    
    if (d_icode == IRRMOVQ || d_icode == IIRMOVQ || d_icode == IOPQ) {
	    return dreg->getrB()->getOutput();
    }

    if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET) {
	return RSP;
    }

    return RNONE;
}

uint64_t getDstM(D * dreg) {
    
    uint64_t d_icode = dreg->geticode()->getOutput();
    
    if (d_icode == IMRMOVQ || d_icode == IPOPQ) {
	    return dreg->getrA()->getOutput();
    }

    return RNONE;
}

uint64_t getValA(D * dreg, bool & error) {
    
    RegisterFile * reggie = RegisterFile::getInstance();

    return reggie->readRegister((int32_t)dreg->getrA()->getOutput(), error);
}

uint64_t getValB(D * dreg, bool & error) {

    RegisterFile * reggie = RegisterFile::getInstance();
    return reggie->readRegister((int32_t)dreg->getrB()->getOutput(), error);


}

uint64_t dvalA(D * dreg, PipeReg ** pregs) {
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    uint64_t D_icode = dreg->geticode()->getOutput();
    uint64_t d_srcA = getSrcA(dreg);
    bool error = false;
    // icode == rrmove ccmove ret 0
    //uint64_t icode = dreg->geticode()->getOutput();
    if (D_icode == IIRMOVQ || D_icode == IHALT) return 0;
    if(D_icode == ICALL || D_icode == IJXX) return dreg->getvalP()->getOutput();
    if(d_srcA == ExecuteStage::gete_dstE()) return ExecuteStage::gete_valE();
    if(d_srcA == mreg->getdstM()->getOutput()) return MemoryStage::getm_valM();
    if(d_srcA == mreg->getdstE()->getOutput()) return mreg->getvalE()->getOutput();
    if(d_srcA == wreg->getdstM()->getOutput()) return wreg->getvalM()->getOutput();
    if(d_srcA == wreg->getdstE()->getOutput()) return wreg->getvalE()->getOutput();
    return getValA(dreg,error);

}

uint64_t dvalB(D * dreg, PipeReg ** pregs) {
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];

    uint64_t d_srcB = getSrcB(dreg);
    bool error = false;
    if(d_srcB == RNONE) return 0;
    if(d_srcB == ExecuteStage::gete_dstE()) return ExecuteStage::gete_valE();
    if(d_srcB == mreg->getdstE()->getOutput()) return mreg->getvalE()->getOutput();
    if(d_srcB == wreg->getdstE()->getOutput()) return wreg->getvalE()->getOutput();
    return getValB(dreg,error);
}



















