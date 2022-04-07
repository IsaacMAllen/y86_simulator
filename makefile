CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o PipeReg.o PipeRegField.o WritebackStage.o MemoryStage.o ExecuteStage.o DecodeStage.o FetchStage.o \
	  Simulate.o D.o E.o F.o M.o W.o Memory.o Tools.o RegisterFile.o ConditionCodes.o Loader.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Debug.h PipeReg.h Stage.h Simulate.h Memory.h RegisterFile.h \
ConditionCodes.h Loader.h

Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h \
DecodeStage.h FetchStage.h WritebackStage.h Simulate.h RegisterFile.h ConditionCodes.h

PipeReg.o: PipeReg.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h

E.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h E.h Status.h

F.o: PipeRegField.h PipeReg.h F.h

M.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h M.h Status.h

W.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h W.h Status.h

PipeRegField.o: PipeRegField.h

WritebackStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h WritebackStage.h Status.h Debug.h W.h

DecodeStage.o: MemoryStage.h ExecuteStage.h PipeReg.h PipeRegField.h D.h F.h M.h W.h E.h Stage.h Status.h Debug.h DecodeStage.h 

ExecuteStage.o: MemoryStage.h Tools.h Instructions.h ConditionCodes.h RegisterFile.h PipeRegField.h PipeReg.h M.h E.h W.h Stage.h ExecuteStage.h Status.h Debug.h

MemoryStage.o: Instructions.h Memory.h RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h E.h W.h Stage.h MemoryStage.h Status.h Debug.h

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h W.h M.h Instructions.h Stage.h FetchStage.h Status.h Debug.h

Loader.o: Loader.h Memory.h

Memory.o: Memory.h Tools.h

Tools.o: Tools.h

RegisterFile.o: RegisterFile.h Tools.h

ConditionCodes.o: ConditionCodes.h Tools.h

clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	./yess

