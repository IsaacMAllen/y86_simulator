//class to perform the combinational logic of
//the Fetch stage
class ExecuteStage: public Stage
{
    private:
	uint64_t valE;
	uint64_t dstE;
	uint64_t e_Cnd;
        void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM);
	bool M_bubble;
	uint64_t performOp(uint64_t e_icode, uint64_t val_rA, uint64_t val_rB, bool & error);
	bool setcc(uint64_t E_icode, W * W);
	uint64_t getAluFun(E * ereg, uint64_t E_icode);
	uint64_t getAluA(E * ereg, uint64_t E_icode);
	uint64_t getAluB(E * ereg, uint64_t E_icode);
	bool cond(uint64_t icode, uint64_t ifun);
	uint64_t eDstE(E * ereg, uint64_t E_icode, uint64_t e_Cnd);
	bool calculateControlSignals(uint64_t m_stat, uint64_t W_stat);
    public:
        bool doClockLow(PipeReg ** pregs, Stage ** stages);
        void doClockHigh(PipeReg ** pregs);
	uint64_t gete_dstE();
	uint64_t gete_valE();
	uint64_t gete_Cnd();

};
