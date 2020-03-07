EXE=compiler
CC=g++
OBJ=main.o scanner.o token.o semanticAnalyzer.o symbol.o symbolTable.o \
    genIr.o interCode.o
CPPFLAGS += -g
$(EXE):$(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) 
	rm $(OBJ) *~ -f
clean:
	rm $(EXE) $(OBJ) *~ -f
