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


#include "semanticAnalyzer.h"
#include "token.h"


// =====================================================================================================================
SemanticAnalyzer::SemanticAnalyzer(Scanner& scanner, SymTab& symbolTable, GenIR& ir)
	:
    m_scanner(scanner),
    m_symbolTable(symbolTable),
    m_ir(ir)
{}


// =====================================================================================================================
// Syntax analysis main program
void SemanticAnalyzer::Analyse()
{
	Move();
	Program();
}

// =====================================================================================================================
// Move next character in
void SemanticAnalyzer::Move()
{
	m_look = m_scanner.Tokenize();
    // test
	printf("%s\n",m_look->toString().c_str());
}

// =====================================================================================================================
// Match, check and Move
bool SemanticAnalyzer::Match(Tag need)
{
	if(m_look->tag == need){
		Move();
		return true;
	}
	else
		return false;	
}

// =====================================================================================================================
void SemanticAnalyzer::PrintSyntaxError(SyntaxError code, Token* token)
{
    // grammar error string
    static const char *syntaxErrorTable[]=
    {
        "Type",
        "identifier",
        "array length",
        "constant",
        ",",
        ";",
        "=",
        ":",
        "while",
        "(",
        ")",
        "[",
        "]",
        "{",
        "}"
    };
    if(code % 2 == 0)//lost
        printf("%s<line: %d> Syntax error : lost %s before %s .\n", m_scanner.GetFile().c_str(), m_scanner.GetLine(),
                syntaxErrorTable[code / 2], token->toString().c_str());
    else//wrong
        printf("%s<line: %d> Syntax error :  Match %s wrongly in %s .\n", m_scanner.GetFile().c_str(), m_scanner.GetLine(),
                syntaxErrorTable[code / 2], token->toString().c_str());
}

#define SYNTAXERROR(code,t) PrintSyntaxError(code,t)

// =====================================================================================================================
void SemanticAnalyzer::Recovery(bool condition, SyntaxError lost, SyntaxError wrong)
{
	if(condition)
        /* In Follow*/
		SYNTAXERROR(lost, m_look);
	else{
		SYNTAXERROR(wrong, m_look);
		Move();
	}
}

#define _(T) ||m_look->tag==T
#define F(C) m_look->tag==C
//类型
#define TYPE_FIRST F(KW_INT)_(KW_CHAR)_(KW_VOID)
//表达式
#define EXPR_FIRST F(LPAREN)_(NUM)_(CH)_(STR)_(ID)_(NOT)_(SUB)_(LEA)_(MUL)_(INC)_(DEC)
//左值运算
#define LVAL_OPR F(ASSIGN)_(OR)_(AND)_(GT)_(GE)_(LT)_(LE)_(EQU)_(NEQU)_(ADD)_(SUB)_(MUL)_(DIV)\
_(MOD)_(INC)_(DEC)
//右值运算
#define RVAL_OPR F(OR)_(AND)_(GT)_(GE)_(LT)_(LE)_(EQU)_(NEQU)_(ADD)_(SUB)_(MUL)_(DIV)\
_(MOD)
//语句
#define STATEMENT_FIRST (EXPR_FIRST)_(SEMICON)_(KW_WHILE)_(KW_FOR)_(KW_DO)_(KW_IF)\
_(KW_SWITCH)_(KW_RETURN)_(KW_BREAK)_(KW_CONTINUE)

// =====================================================================================================================
//	<program>			->	<segment><program>|^
void SemanticAnalyzer::Program()
{
	if(m_look->tag == END){
		
		return ;
	}
	else{
		Segment();
		Program();
	}		
}

// =====================================================================================================================
//	<segment>			->	rsv_extern <Type><def>|<Type><def>
void SemanticAnalyzer::Segment()
{
	bool ext = Match(KW_EXTERN);
	Tag tag = Type();
	Def(ext, tag);
}

// =====================================================================================================================
bool SemanticAnalyzer::IsType()
{
    // from KW_INT to KW_VOID
    return (m_look->tag >= KW_INT) && (m_look->tag <= KW_VOID);
}

// =====================================================================================================================
bool SemanticAnalyzer::IsExpression()
{
    return (m_look->tag == LPAREN) || (m_look->tag == NUM) || (m_look->tag == CH) || (m_look->tag == STR) ||
        (m_look->tag == IDENTIFIER) || (m_look->tag == NOT) || (m_look->tag == SUB) || (m_look->tag == LEA) ||
        (m_look->tag == MUL) || (m_look->tag == INC) || (m_look->tag == DEC);
}


// =====================================================================================================================
bool SemanticAnalyzer::IsLeftValueOperation()
{
    return (m_look->tag == ASSIGN) || (m_look->tag == OR) || (m_look->tag == AND) || (m_look->tag == GT) ||
           (m_look->tag == GE) || (m_look->tag == LT) || (m_look->tag == LE) || (m_look->tag == EQU) ||
           (m_look->tag == NEQU) || (m_look->tag == ADD) || (m_look->tag == SUB) || (m_look->tag == MUL) ||
           (m_look->tag == DIV);
}

// =====================================================================================================================
bool SemanticAnalyzer::IsRightValueOperation()
{
    return (m_look->tag == OR) || (m_look->tag == AND) || (m_look->tag == GT) || (m_look->tag == GE) ||
           (m_look->tag == LT) || (m_look->tag == LE) || (m_look->tag == EQU) || (m_look->tag == NEQU) ||
           (m_look->tag == ADD) || (m_look->tag == SUB) || (m_look->tag == MUL) || (m_look->tag == DIV);
}

// =====================================================================================================================
bool SemanticAnalyzer::IsStatement()
{
    return IsExpression() || (m_look->tag == SEMICON) || (m_look->tag == KW_WHILE) || (m_look->tag == KW_FOR) ||
           (m_look->tag == KW_DO) || (m_look->tag == KW_IF) || (m_look->tag == KW_SWITCH) ||
           (m_look->tag == KW_RETURN) || (m_look->tag == KW_BREAK) || (m_look->tag == KW_CONTINUE);

}

// =====================================================================================================================
//	<Type>				->	rsv_int|rsv_char|rsv_bool|rsv_void
Tag SemanticAnalyzer::Type()
{
	Tag tag = KW_INT;
	if(IsType()){
		tag = m_look->tag;
		Move();
	}
	else
    {
        bool condition = (m_look->tag == IDENTIFIER) || (m_look->tag == MUL);
		Recovery(condition, TYPE_LOST, TYPE_WRONG);
    }

	return tag;
}

// =====================================================================================================================
//	<defdata>			->	identifier <varrdef>|mul identifier <init>
Var* SemanticAnalyzer::Defdata(bool ext, Tag tag)
{
	string name="";
	if(m_look->tag == IDENTIFIER)
    {
		name = (((Identifier*)m_look)->name);
		Move();
		return Varrdef(ext, tag, false, name);
	}
	else if(Match(MUL))
    {
		if(m_look->tag == IDENTIFIER)
        {
			name = (((Identifier*)m_look)->name);
			Move();
		}
		else
        {
            bool condition = (m_look->tag == SEMICON) || (m_look->tag == ASSIGN) || (m_look->tag == COMMA);
			Recovery(condition, ID_LOST, ID_WRONG);
        }

		return Init(ext, tag, true, name);
	}
	else
    {
        bool condition = (m_look->tag == SEMICON) || (m_look->tag == ASSIGN) ||
                         (m_look->tag == COMMA) || (m_look->tag == LBRACK);
		Recovery(condition, ID_LOST, ID_WRONG);
		return Varrdef(ext, tag, false, name);
	}
}

// =====================================================================================================================
//	<DefList>			->	comma <defdata> <DefList>|semicon
void SemanticAnalyzer::DefList(bool ext, Tag tag)
{
	if(Match(COMMA)){
        // ,
		m_symbolTable.AddVar(Defdata(ext, tag));
		DefList(ext, tag);
	}
	else if(Match(SEMICON))
    {
        // ;
		return;
    }
	else
    {
		if(m_look->tag == MUL)
        {
			Recovery(true, COMMA_LOST, COMMA_WRONG);
			m_symbolTable.AddVar(Defdata(ext, tag));
			DefList(ext, tag);
		}
		else
        {
            bool condition = IsType() || IsStatement() || m_look->tag == KW_EXTERN || m_look->tag == RBRACE;
			Recovery(condition, SEMICON_LOST, SEMICON_WRONG);
        }
	}
}

// =====================================================================================================================
//	<varrdef>			->	lbrack num rbrack | <init>
Var* SemanticAnalyzer::Varrdef(bool ext, Tag tag, bool ptr, string name)
{
	if(Match(LBRACK)){
		int len = 0;
		if(m_look->tag == NUM)
        {
			len = ((Number*)m_look)->val;
			Move();
		}
		else
        {
			Recovery(m_look->tag == RBRACK, NUM_LOST, NUM_WRONG);
        }
		if(!Match(RBRACK))
			Recovery((m_look->tag == COMMA) || (m_look->tag == SEMICON), RBRACK_LOST, RBRACK_WRONG);

		return new Var(m_symbolTable.GetScopePath(), ext, tag, name, len);
	}
	else
		return Init(ext, tag, ptr, name);
}

// =====================================================================================================================
//	<init>				->	assign <Expr>|^
Var* SemanticAnalyzer::Init(bool ext, Tag tag, bool ptr, string name)
{
	Var* initVal = NULL;
	if(Match(ASSIGN))
    {
		initVal = Expr();
	}
	return new Var(m_symbolTable.GetScopePath(), ext, tag, ptr, name, initVal);
}

// =====================================================================================================================
//	<def>					->	mul id <init><DefList>|ident <IdTail>
void SemanticAnalyzer::Def(bool ext, Tag tag)
{
	string name = "";
    // if it is pointer
	if(Match(MUL))
    {
		if(m_look->tag == IDENTIFIER)
        {
			name = (((Identifier*)m_look)->name);
			Move();
		}
		else
        {
            bool condition = (m_look->tag == SEMICON) || (m_look->tag == COMMA) || (m_look->tag == ASSIGN);
			Recovery(condition, ID_LOST, ID_WRONG);
        }
		m_symbolTable.AddVar(Init(ext, tag, true, name));
		DefList(ext, tag);
	}
	else
    {
		if(m_look->tag == IDENTIFIER){
            // vars/array/function
			name=(((Identifier*)m_look)->name);
			Move();
		}
		else
        {
            bool condition = (m_look->tag == SEMICON) || (m_look->tag == COMMA) || (m_look->tag == ASSIGN) ||
                             (m_look->tag == LPAREN) || (m_look->tag == LBRACK);
			Recovery(condition, ID_LOST, ID_WRONG);
        }
		IdTail(ext, tag, false, name);
	}
}

// =====================================================================================================================
//	<IdTail>			->	<varrdef><DefList>|lparen <para> rparen <FunTail>
void SemanticAnalyzer::IdTail(bool ext, Tag tag, bool ptr, string name)
{
	if(Match(LPAREN)){
		m_symbolTable.Enter();
		vector<Var*> paraList;
		Para(paraList);
		if(!Match(RPAREN))
        {
            bool condition = (m_look->tag == LBRACK) || (m_look->tag == SEMICON);
			Recovery(condition, RPAREN_LOST, RPAREN_WRONG);
        }
		Fun* pFun = new Fun(ext, tag, name, paraList);
		FunTail(pFun);
		m_symbolTable.Leave();
	}
	else{
		m_symbolTable.AddVar(Varrdef(ext, tag, false, name));
		DefList(ext, tag);
	}
}

// =====================================================================================================================
//	<ParaDataTail>->	lbrack rbrack|lbrack num rbrack|^
Var* SemanticAnalyzer::ParaDataTail(Tag tag, string name)
{
	if(Match(LBRACK))
    {
		int len = 1;
		if(m_look->tag == NUM){
			len = ((Number*)m_look)->val;
			Move();
		}
		if(!Match(RBRACK))
        {
			Recovery((m_look->tag == COMMA) || (m_look->tag == RPAREN), RBRACK_LOST, RBRACK_WRONG);
        }

		return new Var(m_symbolTable.GetScopePath(), false, tag, name, len);
	}
	return new Var(m_symbolTable.GetScopePath(), false, tag, false, name);
}


// =====================================================================================================================
//	<ParaData>		->	mul ident|ident <ParaDataTail>
Var* SemanticAnalyzer::ParaData(Tag tag)
{
	string name = "";
	if(Match(MUL)){
		if(m_look->tag == IDENTIFIER)
        {
			name = ((Identifier*)m_look)->name;
			Move();
		}
		else
        {
			Recovery((m_look->tag = COMMA) || (m_look->tag == RPAREN), ID_LOST, ID_WRONG);
        }
		return new Var(m_symbolTable.GetScopePath(), false, tag, true, name);
	}
	else if(m_look->tag == IDENTIFIER)
    {
		name = ((Identifier*)m_look)->name;
		Move();
		return ParaDataTail(tag, name);
	}
	else{
		Recovery((m_look->tag == COMMA) || (m_look->tag == RPAREN) || (m_look->tag == LBRACK), ID_LOST, ID_WRONG);
		return new Var(m_symbolTable.GetScopePath(), false, tag, false, name);
	}
}

// =====================================================================================================================
//	<para>				->	<Type><ParaData><ParaList>|^
void SemanticAnalyzer::Para(vector<Var*>& list)
{
	if(m_look->tag == RPAREN)
		return;
	Tag tag = Type();
	Var* v = ParaData(tag);
	m_symbolTable.AddVar(v);//保存参数到符号表
	list.push_back(v);
	ParaList(list);
}

// =====================================================================================================================
//	<ParaList>		->	comma<Type><ParaData><ParaList>|^
void SemanticAnalyzer::ParaList(vector<Var*>& list)
{
	if(Match(COMMA))
    {
		Tag tag = Type();
		Var* v = ParaData(tag);
		m_symbolTable.AddVar(v);
		list.push_back(v);
		ParaList(list);
	}
}

// =====================================================================================================================
//	<FunTail>			->	<Block>|semicon
void SemanticAnalyzer::FunTail(Fun* f)
{
	if(Match(SEMICON)){
		m_symbolTable.DecFun(f);
	}
	else{
		m_symbolTable.DefFun(f);
		Block();
		m_symbolTable.EndDefFun();
	}
}

// =====================================================================================================================
//	<Block>				->	lbrac<SubProgram>rbrac
void SemanticAnalyzer::Block()
{
	if(!Match(LBRACE))
		Recovery(IsType() || IsStatement() || (m_look->tag == RBRACE), LBRACE_LOST, LBRACE_WRONG);
	SubProgram();
	if(!Match(RBRACE))
		Recovery(IsType() || IsStatement() || (m_look->tag == KW_EXTERN) || (m_look->tag == KW_ELSE) ||
                 (m_look->tag == KW_CASE) || (m_look->tag == KW_DEFAULT),
			     RBRACE_LOST, RBRACE_WRONG);
}

// =====================================================================================================================
//	<SubProgram>	->	<LocalDef><SubProgram>|<Statements><SubProgram>|^
void SemanticAnalyzer::SubProgram()
{
	if(IsType()){
		LocalDef();
		SubProgram();
	}
	else if(IsStatement()){
		Statement();
		SubProgram();
	}
}

// =====================================================================================================================
// <LocalDef>		->	<Type><defdata><DefList>
void SemanticAnalyzer::LocalDef()
{
	Tag tag = Type();
	m_symbolTable.AddVar(Defdata(false, tag));
	DefList(false, tag);
}

// =====================================================================================================================
//	<Statement>		->	<AltExpr>semicon
//										|<WhileStatement>|<ForStatement>|<DoWhileStatement>
//										|<IfStatement>|<SwitchStatement>
//										|rsv_break semicon
//										|rsv_continue semicon
//										|rsv_return<AltExpr>semicon
void SemanticAnalyzer::Statement()
{
	switch(m_look->tag)
	{
	case KW_WHILE:
        WhileStatement();
        break;
	case KW_FOR:
        ForStatement();
        break;
	case KW_DO:
        DoWhileStatement();
        break;
	case KW_IF:
        IfStatement();
        break;
	case KW_SWITCH:
        SwitchStatement();
        break;
	case KW_BREAK:
	    m_ir.GenBreak();
		Move();
		if(!Match(SEMICON))
			Recovery(IsType() || IsStatement() || (m_look->tag == RBRACE), SEMICON_LOST, SEMICON_WRONG);
		break;
	case KW_CONTINUE:
		m_ir.GenContinue();
		Move();
		if(!Match(SEMICON))
			Recovery(IsType() || IsStatement() || (m_look->tag == RBRACE), SEMICON_LOST, SEMICON_WRONG);
		break;
	case KW_RETURN:
		Move();
		m_ir.GenReturn(AltExpr());
		if(!Match(SEMICON))
			Recovery(IsType() || IsStatement() || (m_look->tag == RBRACE), SEMICON_LOST, SEMICON_WRONG);
		break;
	default:
		AltExpr();
		if(!Match(SEMICON))
			Recovery(IsType() || IsStatement() || (m_look->tag == RBRACE), SEMICON_LOST, SEMICON_WRONG);
	}
}

// =====================================================================================================================
//	<WhileStatement>		->	rsv_while lparen<AltExpr>rparen<Block>
//	<Block>				->	<Block>|<Statement>
void SemanticAnalyzer::WhileStatement()
{
	m_symbolTable.Enter();
	InterInst* _while,*_exit;
	m_ir.GenWhileHead(_while, _exit);	
	Match(KW_WHILE);
	if(!Match(LPAREN))
		Recovery(IsExpression() || (m_look->tag == RPAREN), LPAREN_LOST, LPAREN_WRONG);	

	Var*cond=AltExpr();	
	m_ir.GenWhileCond(cond, _exit);//while条件	
	if(!Match(RPAREN))
		Recovery((m_look->tag == LBRACE), RPAREN_LOST, RPAREN_WRONG);
	if(m_look->tag == LBRACE)
    {
        Block();
    }
	else
    {
        Statement();
    }
	m_ir.GenWhileTail(_while, _exit);
	m_symbolTable.Leave();	
}

// =====================================================================================================================
//	<DoWhileStatement> -> 	rsv_do <Block> rsv_while lparen<AltExpr>rparen semicon
//	<Block>				->	<Block>|<Statement>
void SemanticAnalyzer::DoWhileStatement()
{
	m_symbolTable.Enter();
	InterInst* _do, * _exit;
	m_ir.GenDoWhileHead(_do, _exit);
	Match(KW_DO);	
	if(m_look->tag == LBRACE)
    {
        Block();
    }
	else
    {
        Statement();
    }
	if(!Match(KW_WHILE))
    {
		Recovery(m_look->tag == LPAREN, WHILE_LOST, WHILE_WRONG);
    }
	if(!Match(LPAREN))
    {
		Recovery(IsExpression() || (m_look->tag == RPAREN), LPAREN_LOST, LPAREN_WRONG);
    }
	m_symbolTable.Leave();
	Var* cond = AltExpr();
	if(!Match(RPAREN))
    {
		Recovery(m_look->tag == SEMICON, RPAREN_LOST, RPAREN_WRONG);
    }
	if(!Match(SEMICON))
    {
		Recovery(IsType() || IsStatement() || (m_look->tag == RBRACE), SEMICON_LOST, SEMICON_WRONG);
    }
	m_ir.GenDoWhileTail(cond,_do,_exit);
}

// =====================================================================================================================
//	<ForStatement> 		-> 	rsv_for lparen <ForInit> semicon <AltExpr> semicon <AltExpr> rparen <Block>
//	<Block>				->	<Block>|<Statement>	
void SemanticAnalyzer::ForStatement()
{
	m_symbolTable.Enter();
	InterInst *_for, *_exit, *_step, *_Block;
	Match(KW_FOR);
	if(!Match(LPAREN))
    {
		Recovery(IsType() || IsExpression() || (m_look->tag == SEMICON), LPAREN_LOST, LPAREN_WRONG);
    }
	ForInit();
	m_ir.GenForHead(_for, _exit);
	Var* cond = AltExpr();	
	m_ir.GenForCondBegin(cond, _step, _Block, _exit);
	if(!Match(SEMICON))
    {
		Recovery(IsExpression(), SEMICON_LOST, SEMICON_WRONG);
    }
	AltExpr();
	if(!Match(RPAREN))
    {
		Recovery((m_look->tag == LBRACE),RPAREN_LOST,RPAREN_WRONG);
    }
	m_ir.GenForCondEnd(_for, _Block);
	if(m_look->tag == LBRACE)
    {
        Block();
    }
	else
    {
        Statement();
    }
	m_ir.GenForTail(_step, _exit);
	m_symbolTable.Leave();
}

// =====================================================================================================================
//	<ForInit> 		->  <LocalDef> | <AltExpr>
void SemanticAnalyzer::ForInit()
{
	if(IsType())
    {
		LocalDef();
    }
	else{
		AltExpr();
		if(!Match(SEMICON))
        {
			Recovery(IsExpression(), SEMICON_LOST, SEMICON_WRONG);
        }
	}
}

// =====================================================================================================================
//	<IfStatement>			->	rsv_if lparen<Expr>rparen<Block><ElseStatement>
void SemanticAnalyzer::IfStatement()
{
	m_symbolTable.Enter();
	InterInst* _else, *_exit;
	Match(KW_IF);
	if(!Match(LPAREN))
    {
		Recovery(IsExpression(), LPAREN_LOST, LPAREN_WRONG);
    }
	Var* cond = Expr();
	m_ir.GenIfHead(cond, _else);
	if(!Match(RPAREN))
    {
		Recovery(m_look->tag == LBRACE, RPAREN_LOST, RPAREN_WRONG);
    }
	if(m_look->tag == LBRACE)
    {
        Block();
    }
	else
    {
        Statement();
    }
	m_symbolTable.Leave();	
	
	m_ir.GenElseHead(_else, _exit);
	if(m_look->tag == KW_ELSE){
		ElseStatement();
	}
	m_ir.GenElseTail(_exit);
}

// =====================================================================================================================
//	<ElseStatement>		-> 	rsv_else<Block>|^
void SemanticAnalyzer::ElseStatement()
{
	if(Match(KW_ELSE)){
		m_symbolTable.Enter();
		if(m_look->tag == LBRACE)
        {
            Block();
        }
		else
        {
            Statement();
        }
		m_symbolTable.Leave();
	}
}

// =====================================================================================================================
//	<SwitchStatement>	-> 	rsv_switch lparen <Expr> rparen lbrac <casestat> rbrac
void SemanticAnalyzer::SwitchStatement()
{
	m_symbolTable.Enter();
	InterInst* _exit;
	m_ir.GenSwitchHead(_exit);
	Match(KW_SWITCH);
	if(!Match(LPAREN))
    {
		Recovery(IsExpression(), LPAREN_LOST, LPAREN_WRONG);
    }
	Var* cond = Expr();
	if(cond->IsRef())
    {
        cond = m_ir.GenAssign(cond);//switch(*p),switch(a[0])
    }
	if(!Match(RPAREN))
    {
		Recovery(m_look->tag == LBRACE, RPAREN_LOST, RPAREN_WRONG);
    }
	if(!Match(LBRACE))
    {
		Recovery((m_look->tag == KW_CASE) || (m_look->tag == KW_DEFAULT), LBRACE_LOST, LBRACE_WRONG);
    }
	CaseStatement(cond);
	if(!Match(RBRACE))
    {
		Recovery(IsType() || IsStatement(), RBRACE_LOST, RBRACE_WRONG);
    }
	m_ir.GenSwitchTail(_exit);
	m_symbolTable.Leave();
}

// =====================================================================================================================
//	<casestat> 		-> 	rsv_case <CaseLabel> colon <SubProgram><casestat>
//										|rsv_default colon <SubProgram>
void SemanticAnalyzer::CaseStatement(Var*cond)
{
	if(Match(KW_CASE)){
		InterInst*  _case_exit;
		Var* lb= CaseLabel();
		m_ir.GenCaseHead(cond, lb, _case_exit);
		if(!Match(COLON))
        {
			Recovery(IsType() || IsStatement(), COLON_LOST, COLON_WRONG);
        }
		m_symbolTable.Enter();
		SubProgram();
		m_symbolTable.Leave();
		m_ir.GenCaseTail(_case_exit);//case尾部
		CaseStatement(cond);
	}
	else if(Match(KW_DEFAULT))
    {
		if(!Match(COLON))
        {
			Recovery(IsType() || IsStatement(), COLON_LOST, COLON_WRONG);
        }
		m_symbolTable.Enter();
		SubProgram();
		m_symbolTable.Leave();
	}
}

// =====================================================================================================================
//	<CaseLabel>		->	<Literal>
Var* SemanticAnalyzer::CaseLabel()
{
	return Literal();
}

// =====================================================================================================================
//	<AltExpr>			->	<Expr>|^
Var* SemanticAnalyzer::AltExpr()
{
	if(IsExpression())
		return Expr();
	return Var::GetVoid();
}

// =====================================================================================================================
//	<Expr> 				-> 	<AssExpr>
Var* SemanticAnalyzer::Expr()
{
	return AssExpr();
}

// =====================================================================================================================
//	<AssExpr>			->	<OrExpr><asstail>
Var* SemanticAnalyzer::AssExpr()
{
	Var* lval = OrExpr();
	return AssTail(lval);
}

// =====================================================================================================================
//	<asstail>			->	assign<AssExpr>|^
Var* SemanticAnalyzer::AssTail(Var* lval)
{
	if(Match(ASSIGN))
    {
		Var* rval = AssExpr();
		Var* result = m_ir.GenTwoOp(lval, ASSIGN, rval);
		return AssTail(result);
	}
	return lval;
}

// =====================================================================================================================
//	<OrExpr> 			-> 	<AndExpr><ortail>
Var* SemanticAnalyzer::OrExpr()
{
	Var* lval = AndExpr();
	return OrTail(lval);
}

// =====================================================================================================================
//	<ortail> 			-> 	or <AndExpr> <ortail>|^
Var* SemanticAnalyzer::OrTail(Var* lval)
{
	if(Match(OR)){
		Var* rval = AndExpr();
		Var* result = m_ir.GenTwoOp(lval, OR, rval);
		return OrTail(result);
	}
	return lval;
}

// =====================================================================================================================
//	<AndExpr> 		-> 	<CmpExpr><AndTail>	
Var* SemanticAnalyzer::AndExpr()
{
	Var* lval = CmpExpr();
	return AndTail(lval);
}

// =====================================================================================================================
//	<AndTail> 		-> 	and <CmpExpr> <AndTail>|^
Var* SemanticAnalyzer::AndTail(Var*lval)
{
	if(Match(AND))
    {
		Var* rval = CmpExpr();
		Var* result = m_ir.GenTwoOp(lval, AND, rval);
		return AndTail(result);
	}
	return lval;
}

// =====================================================================================================================
//	<CmpExpr>			->	<AloExpr><CmpTail>
Var* SemanticAnalyzer::CmpExpr()
{
	Var* lval = AloExpr();
	return CmpTail(lval);
}

// =====================================================================================================================
//	<CmpTail>			->	<CmpS><AloExpr><CmpTail>|^
Var* SemanticAnalyzer::CmpTail(Var*lval)
{
	if((m_look->tag == GT) || (m_look->tag == GE) || (m_look->tag == LT) || (m_look->tag == LE) || (m_look->tag == EQU) ||
       (m_look->tag == NEQU))
    {
		Tag opt = CmpS();
		Var* rval = AloExpr();
		Var* result = m_ir.GenTwoOp(lval, opt, rval);
		return CmpTail(result);
	}
	return lval;
}

// =====================================================================================================================
//	<CmpS>				->	gt|ge|ls|le|equ|nequ
Tag SemanticAnalyzer::CmpS()
{
	Tag opt = m_look->tag;
	Move();
	return opt;
}

// =====================================================================================================================
//	<AloExpr>			->	<Item><AloTail>
Var* SemanticAnalyzer::AloExpr()
{
	Var* lval = Item();
	return AloTail(lval);
}

// =====================================================================================================================
//	<AloTail>			->	<AddS><Item><AloTail>|^
Var* SemanticAnalyzer::AloTail(Var* lval)
{
	if((m_look->tag == ADD) || (m_look->tag == SUB))
    {
		Tag opt = AddS();
		Var* rval = Item();
		Var* result = m_ir.GenTwoOp(lval, opt, rval);
		return AloTail(result);
	}
	return lval;
}

// =====================================================================================================================
/*
	<AddS>				->	add|sub
*/
Tag SemanticAnalyzer::AddS()
{
	Tag opt = m_look->tag;
	Move();
	return opt;
}

// =====================================================================================================================
//	<Item>				->	<Factor><ItemTail>
Var* SemanticAnalyzer::Item()
{
	Var* lval = Factor();
	return ItemTail(lval);
}

// =====================================================================================================================
//	<ItemTail>		->	<MulS><Factor><ItemTail>|^
Var* SemanticAnalyzer::ItemTail(Var*lval)
{
	if((m_look->tag == MUL) || (m_look->tag == DIV) || (m_look->tag == MOD)){
		Tag opt = MulS();
		Var* rval = Factor();
		Var* result = m_ir.GenTwoOp(lval, opt, rval);//双目运算
		return ItemTail(result);
	}
	return lval;
}

// =====================================================================================================================
//	<MulS>				->	mul|div|mod
Tag SemanticAnalyzer::MulS()
{
	Tag opt = m_look->tag;
	Move();
	return opt;
}

// =====================================================================================================================
//	<Factor> 			-> 	<LeftOp><Factor>|<val>
Var* SemanticAnalyzer::Factor()
{
	if((m_look->tag == NOT) || (m_look->tag == SUB) || (m_look->tag == LEA) || (m_look->tag == MUL) ||
       (m_look->tag == INC) || (m_look->tag == DEC))
    {
		Tag opt = LeftOp();
		Var* v= Factor();
		return m_ir.GenOneOpLeft(opt, v);
	}
	else
		return Val();
}

// =====================================================================================================================
//	<LeftOp> 				-> 	not|sub|lea|mul|incr|decr
Tag SemanticAnalyzer::LeftOp()
{
	Tag opt = m_look->tag;
	Move();
	return opt;
}

// =====================================================================================================================
//	<Val>					->	<Elem><RightOp>
Var* SemanticAnalyzer::Val()
{
	Var* v = Elem();
	if((m_look->tag == INC) || (m_look->tag == DEC))
    {
		Tag opt = RightOp();
		v = m_ir.GenOneOpRight(v, opt);
	}
	return v;
}

// =====================================================================================================================
//	<RightOp>					->	incr|decr|^
Tag SemanticAnalyzer::RightOp()
{
	Tag opt = m_look->tag;
	Move();
	return opt;
}

// =====================================================================================================================
//	<Elem>				->	ident<IdExpr>|lparen<Expr>rparen|<Literal>
Var* SemanticAnalyzer::Elem()
{
	Var* v = NULL;
	if(m_look->tag == IDENTIFIER)
    {
		string name = ((Identifier*)m_look)->name;
		Move();
		v = IdExpr(name);
	}
	else if(Match(LPAREN)){
		v = Expr();
		if(!Match(RPAREN)){
			Recovery(IsLeftValueOperation(), RPAREN_LOST, RPAREN_WRONG);
		}
	}
	else
    {
		v = Literal();
    }
	return v;
}

// =====================================================================================================================
//	<Literal>			->	number|string|chara
Var* SemanticAnalyzer::Literal()
{
	Var *v = NULL;
	if((m_look->tag == NUM) || (m_look->tag == STR) || (m_look->tag == CH)){
		v = new Var(m_look);
		if(m_look->tag == STR)
        {
			m_symbolTable.AddStr(v);
        }
		else
        {
			m_symbolTable.AddVar(v);
        }
		Move();
	}
	else
    {
		Recovery(IsRightValueOperation(), LITERAL_LOST, LITERAL_WRONG);
    }

	return v;
}

// =====================================================================================================================
//	<IdExpr>			->	lbrack <Expr> rbrack|lparen<RealArg>rparen|^
Var* SemanticAnalyzer::IdExpr(string name)
{
	Var* v = NULL;
	if(Match(LBRACK)){
		Var* index = Expr();
		if(!Match(RBRACK))
        {
			Recovery(IsLeftValueOperation(), LBRACK_LOST, LBRACK_WRONG);
        }
		Var* array = m_symbolTable.GetVar(name);
		v = m_ir.GenArray(array, index);
	}
	else if(Match(LPAREN))
    {
		vector<Var*> args;
		RealArg(args);
		if(!Match(RPAREN))
        {
			Recovery(IsRightValueOperation(), RPAREN_LOST, RPAREN_WRONG);
        }
		Fun* function = m_symbolTable.GetFun(name, args);
		v = m_ir.GenCall(function, args);
	}
	else
    {
	    v = m_symbolTable.GetVar(name);
    }

	return v;
}

// =====================================================================================================================
//	<RealArg>			->	<arg><ArgList>|^
void SemanticAnalyzer::RealArg(vector<Var*>& args)
{
	if(IsExpression())
    {		
		args.push_back(Arg());
		ArgList(args);
	}
}

// =====================================================================================================================
//	<ArgList>			->	comma<arg><ArgList>|^
void SemanticAnalyzer::ArgList(vector<Var*>& args)
{
	if(Match(COMMA))
    {
		args.push_back(Arg());
		ArgList(args);
	}
}

// =====================================================================================================================
//	<arg> 				-> 	<Expr>
Var* SemanticAnalyzer::Arg()
{
	return Expr();
}
