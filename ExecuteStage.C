#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "M.h"
#include "E.h"
#include "W.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "ConditionCodes.h"
#include "Tools.h"
#include "MemoryStage.h"

uint64_t performOp(uint64_t e_icode, uint64_t val_rA, uint64_t val_rB, bool & error);
bool setcc(uint64_t E_icode, W * W);
uint64_t getAluFun(E * ereg, uint64_t E_icode);
uint64_t getAluA(E * ereg, uint64_t E_icode);
uint64_t getAluB(E * ereg, uint64_t E_icode);
bool cond(uint64_t icode, uint64_t ifun);
uint64_t eDstE(E * ereg, uint64_t E_icode, uint64_t e_Cnd);
bool calculateControlSignals(uint64_t m_stat, uint64_t W_stat);

uint64_t ExecuteStage::valE; 
uint64_t ExecuteStage::dstE; 
uint64_t ExecuteStage::e_Cnd;
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
    W * wreg = (W *) pregs[WREG];
    //MemoryStage * m = (MemoryStage *) stages[MSTAGE];
    
    bool error = false;
    uint64_t icode = ereg->geticode()->getOutput();
    uint64_t ifun = ereg->getifun()->getOutput();
    e_Cnd = cond(icode, ifun); 
    if (icode != IJXX) {
	ExecuteStage::valE = ereg->getvalC()->getOutput();
    }
    dstE = eDstE(ereg,icode,e_Cnd);
    if(icode == IRRMOVQ){
	ExecuteStage::valE = ereg->getvalA()->getOutput();
    }
    if(icode == IMRMOVQ || icode == IRMMOVQ) {
	ExecuteStage::valE = ereg->getvalC()->getOutput() + ereg->getvalB()->getOutput();
    }

    if(icode == IPOPQ) {
	ExecuteStage::valE = 8 + getAluB(ereg, icode);
    }
    
    if (icode == IPUSHQ) {
	ExecuteStage::valE = getAluB(ereg, icode) - 8;
    }
    
    if (icode == ICALL || icode == IRET) {
	ExecuteStage::valE = getAluA(ereg, icode) + getAluB(ereg, icode);
    }

    if (icode == IJXX) {
	ExecuteStage::valE = 0;
    }

    if (setcc(icode, wreg)){
	ExecuteStage::valE = performOp(ifun, getAluA(ereg, icode), getAluB(ereg, icode), error);	
    }

    M_bubble = calculateControlSignals(MemoryStage::getm_stat(),wreg->getstat()->getOutput());

    setMInput(mreg, ereg -> getstat() -> getOutput(), icode, e_Cnd, ExecuteStage::valE, ereg -> getvalA() -> getOutput(), ExecuteStage::dstE, ereg -> getdstM() -> getOutput());

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
	
	if(!M_bubble) {
	    mreg->getstat()->normal();
	    mreg->geticode()->normal();
	    mreg->getCnd()->normal();
	    mreg->getvalE()->normal();
	    mreg->getvalA()->normal();
	    mreg->getdstE()->normal();
	    mreg->getdstM()->normal();
	}
	else {
	    mreg->getstat()->bubble(SAOK);
	    mreg->geticode()->bubble(INOP);
	    mreg->getCnd()->bubble();
	    mreg->getvalE()->bubble();
	    mreg->getvalA()->bubble();
	    mreg->getdstE()->bubble(RNONE);
	    mreg->getdstM()->bubble(RNONE);
	}


}

/*
 * setMInput
 * sets input for Memory Stage
 *
 * @param: mreg - pointer to MemoryStage pipereg. stat, icode, valE, valA, dstE, dstM all recieved from doClockLow.
 */
void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM) {

	mreg->getstat()->setInput(stat);
	mreg->geticode()->setInput(icode);
	mreg->getCnd()->setInput(cnd);
	mreg->getvalE()->setInput(valE);
	mreg->getvalA()->setInput(valA);
	mreg->getdstE()->setInput(dstE);
	mreg->getdstM()->setInput(dstM);
	


}

/*
 * getAluA
 * retrieve correct value for ALU arithmatic A operand.
 *
 * @param: ereg - pointer to ExecuteStage pipe register for icode and valA/valC acces.
 * 		E_icode passed in for comparison.
 */
uint64_t getAluA(E * ereg, uint64_t E_icode) {
    if(E_icode == IRRMOVQ || E_icode == IOPQ) return ereg->getvalA()->getOutput();
    if(E_icode == IIRMOVQ || E_icode == IRMMOVQ || E_icode == IMRMOVQ) return ereg->getvalC()->getOutput();
    if(E_icode == ICALL || E_icode == IPUSHQ) return -8;
    if(E_icode == IRET || E_icode == IPOPQ) return 8;
    return 0;

}

/*
 * getAluB
 * retrieve correct value for ALU arithmatic A operand.
 *
 * @param: ereg - pointer to ExecuteStage pipe register for icode and valA/valC acces.
 * 		E_icode passed in for comparison.
 */
uint64_t getAluB(E * ereg, uint64_t E_icode) {
    if(E_icode == IRMMOVQ || E_icode == IMRMOVQ || E_icode == IOPQ || E_icode == ICALL || E_icode == IPUSHQ || E_icode == IRET || E_icode == IPOPQ) return ereg->getvalB()->getOutput();
    return 0;
}

/**
 * getAluFun
 * retrieve correct instruction function value.
 *
 * @param: ereg - pointer to ExecuteStage pipe register for icode.
 */
uint64_t getAluFun(E * ereg, uint64_t E_icode) {
    if (E_icode == IOPQ) return ereg->getifun()->getOutput();
    return ADDQ;
}

/**
 * setcc
 *
 * returns true or false if condition codes needs to be set.
 *
 * @param: E_icode for comparison, W for access to getstat().
 */
bool setcc(uint64_t E_icode, W * W) {
    uint64_t m_stat = MemoryStage::getm_stat();
    uint64_t W_stat = W->getstat()->getOutput();
    return E_icode == IOPQ && (m_stat != SADR && m_stat != SINS && m_stat != SHLT) && (W_stat != SADR && W_stat != SINS && W_stat != SHLT);
}

/**
 * eDstE
 *
 * returns correct dstE value dependent on E_icode and condition code.
 *
 * @param: ereg - for getdstE() access, E_icode and e_Cnd for icode and condition code access.
 */
uint64_t eDstE(E * ereg, uint64_t E_icode, uint64_t e_Cnd) {
    if(E_icode == IRRMOVQ && !e_Cnd) return RNONE;
    return ereg->getdstE()->getOutput();

}

/**
 *
 * performOp
 *
 * sets condition codes to corrent values and returns result.
 *
 * @param e_ifun - function code, val_rA - value in register a, val_rB - value in register B, error - reference the error.
 * @return result - result of operation.  
 */
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

/**
 * gete_valE
 *
 * getter for valE
 *
 * @return valE
 */
uint64_t ExecuteStage::gete_valE(){
    return ExecuteStage::valE;
}

/**
 * gete_dstE
 *
 * getter for dstE
 *
 * @return dstE
 */
uint64_t ExecuteStage::gete_dstE(){
    return ExecuteStage::dstE;
}

/**
 * gete_Cnd
 *
 * getter for e_Cnd
 *
 * @return e_Cnd
 */
uint64_t ExecuteStage::gete_Cnd(){
    return ExecuteStage::e_Cnd;
}

/**
 * cond
 *
 * used to set e_Cnd to correct condition code for instructions.
 *
 * @param icode - instruction code, ifun - function code.
 * @return 1 or 0.
 */
bool cond(uint64_t icode, uint64_t ifun) {

    ConditionCodes * codes = ConditionCodes::getInstance();
    bool error = false;
    if(!(icode == IJXX || icode ==  ICMOVXX)) return 0;
    
    if((icode == IJXX || icode == IRRMOVQ) && ifun == UNCOND) {
	return true;	
    }
    if((icode == IJXX && ifun == LESSEQ) || (icode == ICMOVXX && ifun == LESSEQ)) {
	return (codes->getConditionCode(SF, error) ^ codes->getConditionCode(OF, error)) || codes->getConditionCode(ZF,error);
    }
    if((icode == IJXX && ifun == LESS) || (icode == ICMOVXX && ifun == LESS)) {
	return (codes->getConditionCode(SF, error) ^ codes->getConditionCode(OF, error));
    }   
    if((icode == IJXX && ifun == EQUAL) || (icode == ICMOVXX && ifun == EQUAL)) {
	return codes->getConditionCode(ZF, error);
    }   
    if((icode == IJXX && ifun == NOTEQUAL) || (icode == ICMOVXX && ifun == NOTEQUAL)) {
	return !codes->getConditionCode(ZF, error);
    }   
    if((icode == IJXX && ifun == GREATER) || (icode == ICMOVXX && ifun == GREATER)) {
	return !(codes->getConditionCode(SF, error) ^ codes->getConditionCode(OF, error)) && !codes->getConditionCode(ZF, error);
    }   
    if((icode == IJXX && ifun == GREATEREQ) || (icode == ICMOVXX && ifun == GREATEREQ)) {
	return !(codes->getConditionCode(SF, error) ^ codes->getConditionCode(OF, error));
    }   
    return false;    
}

/**
 * calculateControlSignals
 *
 * used to determine bubbling for MemoryStage dependent on m_stat and W_stat
 *
 * @param m_stat W_stat
 * @return true or false
 */
bool calculateControlSignals(uint64_t m_stat, uint64_t W_stat) {
    return (m_stat == SADR || m_stat == SINS || m_stat == SHLT) || (W_stat == SADR 
	    || W_stat == SINS || W_stat == SHLT);       
}
