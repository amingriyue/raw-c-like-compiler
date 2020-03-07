#pragma once 
#include <vector>
#include "common.h"
#include "symbolTable.h"
#include "interCode.h"

/*
	中间代码生成器
*/
class GenIR
{
	static int lbNum;//标签号码，用于产生唯一的标签
	
	SymTab &symtab;//符号表
	
	//break continue辅助标签列表
	vector< InterInst* > heads;
	vector< InterInst* > tails;
	void push(InterInst*head,InterInst*tail);//添加一个作用域
	void pop();//删除一个作用域
	
	
	//函数调用
	void GenPara(Var*arg);//参数传递语句
	
	//双目运算
	Var* GenAssign(Var*lval,Var*rval);//赋值语句
	
	Var* GenOr(Var*lval,Var*rval);//或运算语句
	Var* GenAnd(Var*lval,Var*rval);//与运算语句
	Var* GenGt(Var*lval,Var*rval);//大于语句
	Var* GenGe(Var*lval,Var*rval);//大于等于语句
	Var* GenLt(Var*lval,Var*rval);//小于语句
	Var* GenLe(Var*lval,Var*rval);//小于等于语句
	Var* GenEqu(Var*lval,Var*rval);//等于语句
	Var* GenNequ(Var*lval,Var*rval);//不等于语句
	Var* GenAdd(Var*lval,Var*rval);//加法语句
	Var* GenSub(Var*lval,Var*rval);//减法语句
	Var* GenMul(Var*lval,Var*rval);//乘法语句
	Var* GenDiv(Var*lval,Var*rval);//除法语句
	Var* GenMod(Var*lval,Var*rval);//取模
	
	//单目运算
	Var* GenNot(Var*val);//非
	Var* GenMinus(Var*val);//负
	Var* GenIncL(Var*val);//左自加语句
	Var* GenDecL(Var*val);//右自减语句
	Var* GenLea(Var*val);//取址语句
	Var* GenPtr(Var*val);//指针取值语句	
	Var* GenIncR(Var*val);//右自加语句
	Var* GenDecR(Var*val);//右自减
public:

	GenIR(SymTab &tab);//重置内部数据

	Var* GenAssign(Var*val);//变量拷贝赋值，用于指针左值引用和变量复制
	
	//产生符号和语句
	Var* GenArray(Var*array,Var*index);//数组索引语句
	Var* GenCall(Fun*function,vector<Var*> & args);//函数调用语句
	Var* GenTwoOp(Var*lval,Tag opt,Var*rval);//双目运算语句	
	Var* GenOneOpLeft(Tag opt,Var*val);//左单目运算语句	
	Var* GenOneOpRight(Var*val,Tag opt);//右单目运算语句
	
	//产生复合语句
	void GenWhileHead(InterInst*& _while,InterInst*& _exit);//while循环头部
	void GenWhileCond(Var*cond,InterInst* _exit);//while条件
	void GenWhileTail(InterInst*& _while,InterInst*& _exit);//while尾部
	void GenDoWhileHead(InterInst*& _do,InterInst*& _exit);//do-while头部
	void GenDoWhileTail(Var*cond,InterInst* _do,InterInst* _exit);//do-while尾部
	void GenForHead(InterInst*& _for,InterInst*& _exit);//for循环头部
	void GenForCondBegin(Var*cond,InterInst*& _step,InterInst*& _block,InterInst* _exit);//for条件开始
	void GenForCondEnd(InterInst* _for,InterInst* _block);//for循环条件结束部分
	void GenForTail(InterInst*& _step,InterInst*& _exit);//for循环尾部
	void GenIfHead(Var*cond,InterInst*& _else);//if头部
	void GenIfTail(InterInst*& _else);//if尾部
	void GenElseHead(InterInst* _else,InterInst*& _exit);//else头部
	void GenElseTail(InterInst*& _exit);//else尾部
	void GenSwitchHead(InterInst*& _exit);//switch头部
	void GenSwitchTail(InterInst* _exit);//switch尾部
	void GenCaseHead(Var*cond,Var*lb,InterInst*& _case_exit);//case头部
	void GenCaseTail(InterInst* _case_exit);//case尾部	
	
	//产生特殊语句
	void GenBreak();//产生break语句
	void GenContinue();//产生continue语句
	void GenReturn(Var*ret);//产生return语句
	bool GenVarInit(Var*var);//产生变量初始化语句
	void GenFunHead(Fun*function);//产生函数入口语句
	void GenFunTail(Fun*function);//产生函数出口语句
	
	//全局函数
	static string GenLb();//产生唯一的标签	
	static bool typeCheck(Var*lval,Var*rval);//检查类型是否可以转换
};

