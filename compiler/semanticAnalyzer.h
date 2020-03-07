/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2020 David Zhou <zhouchunming1986@126.com>. All Rights Reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 **********************************************************************************************************************/


#pragma once
#include "common.h"
#include "scanner.h"
#include "symbol.h"
#include "symbolTable.h"
#include "token.h"
#include "genIr.h"
#include "interCode.h"

using namespace Compiler;
class SemanticAnalyzer
{
private:
    // grammar begin
	void Program();
	void Segment();
	Tag Type();
	
	// clarification and define
	Var* Defdata(bool ext, Tag tag);
	void DefList(bool ext, Tag tag);
	Var* Varrdef(bool ext, Tag tag, bool ptr, string name);
	Var* Init(bool ext, Tag tag, bool ptr, string name);
	void Def(bool ext, Tag tag);
	void IdTail(bool ext, Tag tag, bool ptr, string name);
	
	// function
	Var* ParaDataTail(Tag t, string name);
	Var* ParaData(Tag t);
	void Para(vector<Var*>& list);
	void ParaList(vector<Var*>& list);
	void FunTail(Fun* f);
	void Block();
	void SubProgram();
	void LocalDef();
	
	// statement grammar
	void Statement();
	void WhileStatement();
	void DoWhileStatement();
	void ForStatement();
	void ForInit();
	void IfStatement();
	void ElseStatement();
	void SwitchStatement();
	void CaseStatement(Var* cond);
	Var* CaseLabel();
	
	// expression grammar
	Var* AltExpr();
	Var* Expr();
	Var* AssExpr();
	Var* AssTail(Var* lval);
	Var* OrExpr();
	Var* OrTail(Var* lval);
	Var* AndExpr();
	Var* AndTail(Var* lval);
	Var* CmpExpr();
	Var* CmpTail(Var* lval);
	Tag CmpS();
	Var* AloExpr();
	Var* AloTail(Var* lval);
	Tag AddS();
	Var* Item();
	Var* ItemTail(Var* lval);
	Tag MulS();
	Var* Factor();
	Tag LeftOp();
	Var* Val();
	Tag RightOp();
	Var* Elem();
	Var* Literal();
	Var* IdExpr(string name);
	void RealArg(vector<Var*>& args);
	void ArgList(vector<Var*>& args);
	Var* Arg();
	
	Scanner&    m_scanner;   // scanner to pass token to analyzer
	Token*      m_look;     // check character in advance
	
	// symbol table
    SymTab&    m_symbolTable;
	
	// intermediate language generator
    GenIR &         m_ir;
	
	//Syntax analyze
	void Move();
	bool Match(Tag t);
	void Recovery(bool cond, SyntaxError lost, SyntaxError wrong);
    void PrintSyntaxError(SyntaxError code, Token* token);
    bool IsType();
    bool IsExpression();
    bool IsLeftValueOperation();
    bool IsRightValueOperation();
    bool IsStatement();

public:
	SemanticAnalyzer(Scanner& scanner, SymTab& symbolTable, GenIR& ir);
	
	void Analyse();
};

