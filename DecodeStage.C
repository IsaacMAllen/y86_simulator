#include <iostream>
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
#include "ExecuteStage.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "MemoryStage.h"


/*
 * doClockLow:
 * Performs the Decode stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 * @return: unused
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    ExecuteStage * e = (ExecuteStage *) stages[ESTAGE];
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];
    valA = dvalA(pregs, stages);
    valB = dvalB(pregs, stages); 
    dstE = getDstE(dreg);
    dstM = getDstM(dreg);
    d_srcA = getSrcA(dreg);
    d_srcB = getSrcB(dreg);
    calculateControlSignals(E_bubble, ereg);
    setEInput(ereg, dreg -> getstat() -> getOutput(), dreg -> geticode() -> getOutput(), dreg -> getifun() -> getOutput(), dreg -> getvalC() -> getOutput(), valA, valB, dstE, dstM, d_srcA, d_srcB); 
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
    if (E_bubble){
	bubbleE(ereg);
    } 
    else {
        ereg -> getstat() -> normal();
        ereg -> geticode() -> normal();
        ereg -> getifun() -> normal();
        ereg -> getvalC() -> normal();
        ereg -> getvalA() -> normal();
        ereg -> getvalB() -> normal();
        ereg -> getdstE() -> normal();
        ereg -> getdstM() -> normal();
        ereg -> getsrcA() -> normal();
        ereg -> getsrcB() -> normal();
    }
}

/*
 * setEInput:
 * sets the input of the E pipeline register 
 *
 * @param: ereg - the E pipeline register object
 * @param: stat - status code 
 * @param: icode - instruction code 
 * @param: ifun - function code 
 * @param: valC - immediate value
 * @param: valA - rA's value
 * @param: valB - rB's value
 * @param: dstE - destination register of value computed in E
 * @param: dstM - destination register of value loaded in M
 * @param: srcA - rA's address
 * @param: srcB - rB's address
 */
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB) 
{
    ereg -> getstat() -> setInput(stat);
    ereg -> geticode() -> setInput(icode);
    ereg -> getifun() -> setInput(ifun);
    ereg -> getvalC() -> setInput(valC);
    ereg -> getvalA() -> setInput(valA);
    ereg -> getvalB() -> setInput(valB);
    ereg -> getdstE() -> setInput(dstE);
    ereg -> getdstM() -> setInput(dstM);
    ereg -> getsrcA() -> setInput(srcA);
    ereg -> getsrcB() -> setInput(srcB);

}

/*
 * getSrcA:
 * determines what the value of the address of rA will be
 *
 * @param: dreg - the D pipeline register object
 * @return: the address of rA
 */
uint64_t DecodeStage::getSrcA(D * dreg) {
	
	uint64_t d_icode = dreg -> geticode() -> getOutput();
	
	if (d_icode == IRRMOVQ || d_icode == IRMMOVQ || d_icode == IOPQ 	
	    || d_icode == IPUSHQ) {
	    return dreg -> getrA() -> getOutput();
	}
	if (d_icode == IPOPQ || d_icode == IRET) {
	    return RSP;
	}
	
	return RNONE;
}


/*
 * getSrcB:
 * determines what the value of the address of rB will be
 *
 * @param: dreg - the D pipeline register object
 * @return: the address of rB
 */
uint64_t DecodeStage::getSrcB(D * dreg) {
 
    uint64_t d_icode = dreg -> geticode() -> getOutput();

    if (d_icode == IOPQ || d_icode == IRMMOVQ || d_icode == IMRMOVQ) {
	return dreg -> getrB() -> getOutput();
    }

    if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET) {
	return RSP;
    }

    return RNONE;
}

/*
 * getDstE:
 * determines what the value of the address of the register that will hold the calculated value in ExecuteStage will be
 *
 * @param: dreg - the D pipeline register object
 * @return: the address of dstE
 */
uint64_t DecodeStage::getDstE(D * dreg) {
    
    uint64_t d_icode = dreg -> geticode() -> getOutput();
    
    if (d_icode == IRRMOVQ || d_icode == IIRMOVQ || d_icode == IOPQ) {
	    return dreg -> getrB() -> getOutput();
    }

    if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET) {
	return RSP;
    }

    return RNONE;
}

/*
 * getDstM:
 * determines what the value of the address of the register that will hold the loaded value in MemoryStage will be
 *
 * @param: dreg - the D pipeline register object
 * @return: the address of dstM
 */
uint64_t DecodeStage::getDstM(D * dreg) {
    
    uint64_t d_icode = dreg -> geticode() -> getOutput();
    
    if (d_icode == IMRMOVQ || d_icode == IPOPQ) {
	    return dreg -> getrA() -> getOutput();
    }

    return RNONE;
}

/*
 * getValA:
 * dereferences the appropriate register for valA based on D_icode
 *
 * @param: dreg - the D pipeline register object
 * @param: errror - register file accessing error signaling field
 * @return: valA
 */
uint64_t DecodeStage::getValA(D * dreg, bool & error) {
    
    RegisterFile * reggie = RegisterFile::getInstance();
    uint64_t D_icode = dreg -> geticode() -> getOutput();
    if (D_icode != IPOPQ && D_icode != IRET) {
	return reggie -> readRegister((int32_t)dreg -> getrA() -> getOutput(), error);
    }
    return reggie -> readRegister(RSP, error);
}

/*
 * getValB:
 * dereferences the appropriate register for valB based on D_icode
 *
 * @param: dreg - the D pipeline register object
 * @param: errror - register file accessing error signaling field
 * @return: valB
 */
uint64_t DecodeStage::getValB(D * dreg, bool & error) {

    RegisterFile * reggie = RegisterFile::getInstance();
    uint64_t D_icode = dreg -> geticode() -> getOutput();
    if (D_icode != IPOPQ && D_icode != IPUSHQ && D_icode != IRET && D_icode != ICALL) {
	return reggie -> readRegister((int32_t)dreg -> getrB() -> getOutput(), error);
    }
    return reggie -> readRegister(RSP, error);
}

/*
 * dval
 */
uint64_t DecodeStage::dvalA(PipeReg ** pregs, Stage ** stages) {
    ExecuteStage * e = (ExecuteStage *) stages[ESTAGE];
    MemoryStage * m = (MemoryStage *) stages[MSTAGE]; 
    D * dreg = (D *) pregs[DREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    uint64_t D_icode = dreg -> geticode() -> getOutput();
    uint64_t d_srcA = getSrcA(dreg);
    bool error = false;
    if(D_icode == ICALL || D_icode == IJXX) return dreg -> getvalP() -> getOutput();
    if(d_srcA == RNONE) return 0;
    if(d_srcA == e -> gete_dstE()) return e -> gete_valE();
    if(d_srcA == mreg -> getdstM() -> getOutput()) return m -> getm_valM();
    if(d_srcA == mreg -> getdstE() -> getOutput()) return mreg -> getvalE() -> getOutput();
    if(d_srcA == wreg -> getdstM() -> getOutput()) return wreg -> getvalM() -> getOutput();
    if(d_srcA == wreg -> getdstE() -> getOutput()) return wreg -> getvalE() -> getOutput();
    return getValA(dreg,error);

}

uint64_t DecodeStage::dvalB(PipeReg ** pregs, Stage ** stages) {
    ExecuteStage * e = (ExecuteStage *) stages[ESTAGE];
    MemoryStage * m = (MemoryStage *) stages[MSTAGE];
    D * dreg = (D *) pregs[DREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    uint64_t d_srcB = getSrcB(dreg);
    bool error = false;
    if(d_srcB == RNONE) return 0;
    if(d_srcB == e -> gete_dstE()) return e -> gete_valE();
    if(d_srcB == mreg -> getdstM() -> getOutput()) return m -> getm_valM();
    if(d_srcB == mreg -> getdstE() -> getOutput()) return mreg -> getvalE() -> getOutput();
    if(d_srcB == wreg -> getdstM() -> getOutput()) return wreg -> getvalM() -> getOutput();
    if(d_srcB == wreg -> getdstE() -> getOutput()) return wreg -> getvalE() -> getOutput();
    return getValB(dreg,error);
}

uint64_t DecodeStage::getd_srcA(){
    return DecodeStage::d_srcA;
}

uint64_t DecodeStage::getd_srcB(){
    return DecodeStage::d_srcB;
}

void DecodeStage::calculateControlSignals(bool & E_bubble, E * E){
    uint64_t E_icode = E -> geticode() -> getOutput();
    uint64_t E_dstM = E -> getdstM() -> getOutput();
    E_bubble = (E_icode == IJXX && !ExecuteStage::gete_Cnd()) || ((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == DecodeStage::getd_srcA() || E_dstM == DecodeStage::getd_srcB()));
}

void DecodeStage::bubbleE(E * ereg) {
    ereg -> getstat() -> bubble(SAOK);
    ereg -> geticode() -> bubble(INOP);
    ereg -> getifun() -> bubble(FNONE);
    ereg -> getvalC() -> bubble(0);
    ereg -> getvalA() -> bubble(0);
    ereg -> getvalB() -> bubble(0);
    ereg -> getdstE() -> bubble(RNONE);
    ereg -> getdstM() -> bubble(RNONE);
    ereg -> getsrcA() -> bubble(RNONE);
    ereg -> getsrcB() -> bubble(RNONE);
}
