
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "scanner.h"
#include "semanticAnalyzer.h"
#include "symbolTable.h"
#include "genIr.h"

using namespace std;
using namespace Compiler;

int main(int argc,char*argv[])
{
    string srcFiles = argv[1];
    Scanner scanner(srcFiles);

    scanner.Init();

    SymTab symbolTable;
    GenIR  genIr(symbolTable);
    SemanticAnalyzer semanticAnalyzer(scanner, symbolTable, genIr);
    semanticAnalyzer.Analyse();

    symbolTable.genIr(scanner.GetIrHandle());
    symbolTable.genAsm(scanner.GetOutHandle());

	return 0;
}
