//class to perform the combinational logic of
//the Fetch stage
class DecodeStage: public Stage
{
    private: 
	void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC, 
		uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB); 
	uint64_t d_srcA;
	uint64_t d_srcB;
	bool E_bubble = false;	
	uint64_t valA;
	uint64_t valB;
	uint64_t dstM;
	uint64_t dstE;
	uint64_t getSrcA(D * dreg);
	uint64_t getSrcB(D * dreg); 
	uint64_t getDstE(D * dreg); 
	uint64_t getDstM(D * dreg); 
	uint64_t getValA(D *dreg, bool & error);
	uint64_t getValB(D * dreg, bool & error);
	uint64_t dvalA(PipeReg ** pregs, Stage ** stages);
	uint64_t dvalB(PipeReg ** pregs, Stage ** stages);
	bool doClockLow(PipeReg ** pregs, Stage ** stages);
	void doClockHigh(PipeReg ** pregs);
	void calculateControlSignals(bool & E_bubble, E * E, Stage ** stages);
	void bubbleE(E * ereg); 
    public:
	uint64_t getd_srcA();
	uint64_t getd_srcB();

};
