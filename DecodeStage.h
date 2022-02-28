//class to perform the combinational logic of
//the Fetch stage
class DecodeStage: public Stage
{
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
