//class to perform the combinational logic of
//the Fetch stage
class DecodeStage: public Stage
{
    private: 
        void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC, 
                uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB); 
	static uint64_t d_srcA;
	static uint64_t d_srcB;	
    public:
        bool doClockLow(PipeReg ** pregs, Stage ** stages);
        void doClockHigh(PipeReg ** pregs);
	static uint64_t getd_srcA();
	static uint64_t getd_srcB();

};
