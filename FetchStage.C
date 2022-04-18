#include <iostream>
#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Instructions.h"
#include "Stage.h"
#include "FetchStage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Memory.h"
#include "Tools.h"

uint64_t selectPC(F * freg, M * mreg, W * wreg);
bool needRegIds(uint64_t f_icode);
bool needValC(uint64_t f_icode);
uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
uint64_t PCincrement(uint64_t f_pc, uint64_t f_icode, bool needRegIds, bool needValC);
void getRegIds(uint64_t pc, bool & error, uint64_t & rA, uint64_t & rB, uint64_t f_icode);
uint64_t buildValC(uint64_t instruction, uint64_t f_icode, bool & error); 
uint64_t fStat(uint64_t icode, bool mem_error);
uint64_t fIcode(uint64_t mem_icode, bool mem_error); 
uint64_t fIFun(uint64_t mem_ifun, bool mem_error);
void Dbubble(D * D, E * E, M * M, bool & D_bubble);
void calculateControlSignals(bool & F_stall, bool & D_stall, bool & D_bubble, D * D, E * E, M * M); 
void bubbleD(D * dreg);
void normalD(D * dreg, F * freg, bool & F_stall);

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages) {
    F * freg = (F *) pregs[FREG];
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    uint64_t f_icode = 0, f_ifun = 0, valC = 0, valP = 0;
    uint64_t rA = RNONE, rB = RNONE, stat = SAOK;
    
    //code missing here to select the value of the PC
    //and fetch the instruction from memory
    //Fetching the instruction will allow the icode, ifun,
    //rA, rB, and valC to be set.
    //The lab assignment describes what methods need to be
    //written.
    uint64_t f_pc = selectPC(freg, mreg, wreg);
    bool mem_error = false;
    Memory * mem = Memory::getInstance();
    uint8_t icodeifun = mem -> getByte((int) f_pc, mem_error);
    f_icode = icodeifun >> 4;
    f_ifun = icodeifun & 0x0F;
    f_icode = fIcode(f_icode, mem_error);
    f_ifun = fIFun(f_ifun, mem_error);
    valC = buildValC(f_pc, f_icode, mem_error);
    //The value passed to setInput below will need to be changed
    getRegIds(f_pc,mem_error,rA,rB,f_icode);
    stat = fStat(f_icode,mem_error);    
    valP = PCincrement(f_pc, f_icode, needRegIds(f_icode), needValC(f_icode));
    freg->getpredPC()->setInput(predictPC(f_icode, valC, valP));
    //provide the input values for the D register
    calculateControlSignals(F_stall, D_stall, D_bubble, dreg, ereg, mreg);
    setDInput(dreg, stat, f_icode, f_ifun, rA, rB, valC, valP);
    return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs) {
    F * freg = (F *) pregs[FREG];
    D * dreg = (D *) pregs[DREG];
    if (!F_stall) freg->getpredPC()->normal();
    if(D_bubble) {
	bubbleD(dreg);	
    }	
    else if (!D_stall) {
	normalD(dreg, freg, F_stall);
    }
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
 */
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t f_icode, 
	uint64_t f_ifun, uint64_t rA, uint64_t rB, uint64_t valC, uint64_t valP) {
    dreg->getstat()->setInput(stat);
    dreg->geticode()->setInput(f_icode);
    dreg->getifun()->setInput(f_ifun);
    dreg->getrA()->setInput(rA);
    dreg->getrB()->setInput(rB);
    dreg->getvalC()->setInput(valC);
    dreg->getvalP()->setInput(valP);
}

uint64_t selectPC(F * freg, M * mreg, W * wreg) {
    if (mreg->geticode()->getOutput() == IJXX && !(mreg->getCnd()->getOutput()))
    {
	return mreg->getvalA()->getOutput();
    }
    else if (wreg->geticode()->getOutput() == IRET)
    {
	return wreg->getvalM()->getOutput();
    }
    return freg->getpredPC()->getOutput();
}

bool needRegIds(uint64_t f_icode) {
    return (f_icode == IRRMOVQ) || (f_icode == IOPQ) || (f_icode == IPUSHQ) || (f_icode == IPOPQ) || (f_icode == IIRMOVQ) || (f_icode == IRMMOVQ) || (f_icode == IMRMOVQ); 
}

bool needValC(uint64_t f_icode) {

    return (f_icode == IIRMOVQ) || (f_icode == IRMMOVQ) || (f_icode == IMRMOVQ) || (f_icode == IJXX) || (f_icode == ICALL); 
}

uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP) {
    if ((f_icode == IJXX) || (f_icode == ICALL)) return f_valC;
    return f_valP;
}

uint64_t PCincrement(uint64_t f_pc, uint64_t f_icode, bool needRegIds, bool needValC) {
    if (needValC && f_icode != IJXX && f_icode != ICALL) {
	f_pc += 10;
    }
    else if (f_icode == IJXX || f_icode == ICALL) {
	f_pc += 9;
    }
    else if (needRegIds && !needValC) {
	f_pc += 2;
    }
    else {
	f_pc += 1;
    }
    return f_pc;
}

void getRegIds(uint64_t pc, bool & error, uint64_t & rA, uint64_t & rB, uint64_t icode) {

    Memory * mem = Memory::getInstance();

    if (needRegIds(icode)) {
	uint8_t reg = mem->getByte(pc + 1, error);
	uint8_t rBmask = 0x0F;
	rA = reg >> 4;
	rB = rBmask & reg;    	    
    }
}

uint64_t buildValC(uint64_t instruction, uint64_t icode, bool & error) {
    Memory * mem = Memory::getInstance();
    uint8_t valCArray[8];
    uint64_t valC = 0;
    int start = 0;
    int n = 0;
    int offset = 0;
    if (needValC(icode)) {
    switch (icode) {
	case IJXX:
	case ICALL:
	    start = 1;
	    offset = -1;
	    n = 9;
	    break;
	default:
	    start = 2;
	    n = 10;
	    offset = -2;
	    break;
    }
    for (; start < n; ++start) {
	valCArray[start + offset] = mem->getByte((int32_t) instruction + start, error);
    }
	valC = Tools::buildLong(valCArray);
	return valC;
    }
    return valC;
}

bool instr_valid(uint64_t icode) {

    return (icode == INOP || icode == IHALT || icode == IRRMOVQ || icode == IIRMOVQ 
	    || icode == IRMMOVQ || icode == IMRMOVQ || icode == IOPQ || icode == IJXX 
	    || icode == ICALL || icode == IRET || icode == IPUSHQ || icode == IPOPQ);
}

uint64_t fStat(uint64_t icode, bool mem_error) {
    //to do: if mem_error return SADR
    if(mem_error) return SADR;
    if(!(instr_valid(icode))) return SINS;
    if(icode == IHALT) return SHLT;
    return SAOK;
}

uint64_t fIcode(uint64_t mem_icode, bool mem_error) {
    if (mem_error) return INOP;
    return mem_icode;
}

uint64_t fIFun(uint64_t mem_ifun, bool mem_error) {
    if (mem_error) return FNONE;
    return mem_ifun;
}

void Dbubble(D * D, E * E, M * M, bool & D_bubble) {

    uint64_t D_icode = D -> geticode() -> getOutput();
    uint64_t E_icode = E -> geticode() -> getOutput();
    uint64_t M_icode = M -> geticode() -> getOutput();
    uint64_t E_dstM = E -> getdstM() -> getOutput();

    D_bubble = (E_icode == IJXX && !ExecuteStage::gete_Cnd()) || 
		(!((E_icode == IMRMOVQ || E_icode == IPOPQ) &&
		(E_dstM == DecodeStage::getd_srcA() || E_dstM == DecodeStage::getd_srcB())) &&
		(IRET == D_icode || IRET == E_icode || IRET == M_icode));
}

void calculateControlSignals(bool & F_stall, bool & D_stall, bool & D_bubble, D * D, E * E, M * M) {
    uint64_t E_icode = E -> geticode() -> getOutput();
    uint64_t E_dstM = E -> getdstM() -> getOutput();
    uint64_t D_icode = D -> geticode() -> getOutput();
    uint64_t M_icode = M -> geticode() -> getOutput();
    Dbubble(D, E, M, D_bubble); 
    F_stall = ((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == DecodeStage::getd_srcA() || E_dstM == DecodeStage::getd_srcB())) || 
	    (IRET == D_icode || IRET == E_icode || IRET == M_icode);
    D_stall = (E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == DecodeStage::getd_srcA() || E_dstM == DecodeStage::getd_srcB());
}

void bubbleD(D * dreg) {
        dreg->getstat()->bubble(SAOK);
        dreg->geticode()->bubble(INOP);
        dreg->getifun()->bubble(FNONE);
        dreg->getrA()->bubble(RNONE);
        dreg->getrB()->bubble(RNONE);
        dreg->getvalC()->bubble(0);
        dreg->getvalP()->bubble(0);
    
}

void normalD(D * dreg, F * freg, bool & F_stall) {
        dreg->getstat()->normal();
        dreg->geticode()->normal();
	dreg->getifun()->normal();
        dreg->getrA()->normal();
        dreg->getrB()->normal();
        dreg->getvalC()->normal();
        dreg->getvalP()->normal();

}
