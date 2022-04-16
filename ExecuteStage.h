//class to perform the combinational logic of
//the Fetch stage
class ExecuteStage: public Stage
{
    private:
        void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM);
	bool M_bubble;
    public:
        bool doClockLow(PipeReg ** pregs, Stage ** stages);
        void doClockHigh(PipeReg ** pregs);
	static uint64_t valE;
	static uint64_t dstE;
	static uint64_t e_Cnd;

	static uint64_t gete_dstE();
	static uint64_t gete_valE();
	static uint64_t gete_Cnd();

};
