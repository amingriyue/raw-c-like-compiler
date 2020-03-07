#include "genIr.h"
#include <sstream>
#include "symbol.h"


//打印语义错误
#define SEMERROR(code) printf("error\n")//Error::semError(code)

int GenIR::lbNum=0;

/*
	初始化
*/
GenIR::GenIR(SymTab&tab):symtab(tab)
{
	symtab.SetIr(this);//构建符号表与代码生成器的一一关系
	lbNum=0;
	push(NULL,NULL);//初始化作用域
}

/*
	获取唯一名字的标签
*/
string GenIR::GenLb()
{
	lbNum++;
	string lb=".L";//为了和汇编保持一致！
	stringstream ss;
	ss<<lbNum;
	return lb+ss.str();
}

/*
	数组索引语句
*/
Var* GenIR::GenArray(Var*array,Var*index)
{
	if(!array || !index)return NULL;
	if(array->isVoid()||index->isVoid()){
		SEMERROR(EXPR_IS_VOID);//void函数返回值不能出现在表达式中
		return NULL;
	}
	if(array->isBase() || !index->isBase()){
		SEMERROR(ARR_TYPE_ERR);
		return index;
	}
	return GenPtr(GenAdd(array,index));
}

/*
	实际参数传递
*/
void GenIR::GenPara(Var*arg)
{
	/*
		注释部分为删除代码，参数入栈不能通过拷贝，只能push！
	*/
	if(arg->IsRef())arg=GenAssign(arg);
	//无条件复制参数！！！传值，不传引用！！！
	//Var*newVar=new Var(symtab.GetScopePath(),arg);//创建参数变量
	//symtab.AddVar(newVar);//添加无效变量，占领栈帧！！
	InterInst*argInst=new InterInst(OP_ARG,arg);//push arg!!!
	//argInst->offset=newVar->getOffset();//将变量的地址与arg指令地址共享！！！没有优化地址也能用
	//argInst->path=symtab.GetScopePath();//记录路径！！！为了寄存器分配时计算地址
	symtab.AddInst(argInst);
}

/*
	函数调用语句
*/
Var* GenIR::GenCall(Fun*function,vector<Var*>& args)
{
	if(!function)return NULL;//只要function有效，则参数类型一定检查通过
	for(int i=args.size()-1;i>=0;i--){//逆向传递实际参数
		GenPara(args[i]);
	}
	if(function->getType()==KW_VOID){
		//中间代码fun()
		symtab.AddInst(new InterInst(OP_PROC,function));
		return Var::GetVoid();//返回void特殊变量
	}
	else{		
		Var*ret=new Var(symtab.GetScopePath(),function->getType(),false);
		//中间代码ret=fun()
		symtab.AddInst(new InterInst(OP_CALL,function,ret));
		symtab.AddVar(ret);//将返回值声明延迟到函数调用之后！！！
		return ret;
	}
}

/*
	双目运算语句
*/
Var* GenIR::GenTwoOp(Var*lval,Tag opt,Var*rval)
{
	if(!lval || !rval)return NULL;
	if(lval->isVoid()||rval->isVoid()){
		SEMERROR(EXPR_IS_VOID);//void函数返回值不能出现在表达式中
		return NULL;
	}
	//赋值单独处理
	if(opt==ASSIGN)return GenAssign(lval,rval);//赋值
	//先处理(*p)变量
	if(lval->IsRef())lval=GenAssign(lval);
	if(rval->IsRef())rval=GenAssign(rval);
	if(opt==OR)return GenOr(lval,rval);//或
	if(opt==AND)return GenAnd(lval,rval);//与
	if(opt==EQU)return GenEqu(lval,rval);//等于
	if(opt==NEQU)return GenNequ(lval,rval);//不等于
	if(opt==ADD)return GenAdd(lval,rval);//加
	if(opt==SUB)return GenSub(lval,rval);//减	
	if(!lval->isBase() || !rval->isBase())
	{
		SEMERROR(EXPR_NOT_BASE);//不是基本类型
		return lval;
	}	
	if(opt==GT)return GenGt(lval,rval);//大于
	if(opt==GE)return GenGe(lval,rval);//大于等于
	if(opt==LT)return GenLt(lval,rval);//小于
	if(opt==LE)return GenLe(lval,rval);//小于等于
	if(opt==MUL)return GenMul(lval,rval);//乘
	if(opt==DIV)return GenDiv(lval,rval);//除
	if(opt==MOD)return GenMod(lval,rval);//取模
	return lval;
}

/*
	赋值语句
*/
Var* GenIR::GenAssign(Var*lval,Var*rval)
{
	//被赋值对象必须是左值
	if(!lval->getLeft()){
		SEMERROR(EXPR_NOT_LEFT_VAL);//左值错误
		return rval;
	}
	//类型检查
	if(!typeCheck(lval,rval)){
		SEMERROR(ASSIGN_TYPE_ERR);//赋值类型不匹配
		return rval;
	}
	//考虑右值(*p)
	if(rval->IsRef()){
		if(!lval->IsRef()){
			//中间代码lval=*(rval->ptr)
			symtab.AddInst(new InterInst(OP_GET,lval,rval->getPointer()));
			return lval;
		}
		else{
			//中间代码*(lval->ptr)=*(rval->ptr),先处理右值
			rval=GenAssign(rval);
		}
	}
	//赋值运算
	if(lval->IsRef()){
		//中间代码*(lval->ptr)=rval
		symtab.AddInst(new InterInst(OP_SET,rval,lval->getPointer()));
	}
	else{
		//中间代码lval=rval
		symtab.AddInst(new InterInst(OP_AS,lval,rval));
	}	
	return lval;
}

/*
	拷贝赋值语句,处理取*p值的情况
*/
Var* GenIR::GenAssign(Var*val)
{
	Var*tmp=new Var(symtab.GetScopePath(),val);//拷贝变量信息
	symtab.AddVar(tmp);
	if(val->IsRef()){
		//中间代码tmp=*(val->ptr)
		symtab.AddInst(new InterInst(OP_GET,tmp,val->getPointer()));
	}
	else
		symtab.AddInst(new InterInst(OP_AS,tmp,val));//中间代码tmp=val
	return tmp;
}

/*
	或运算语句
*/
Var* GenIR::GenOr(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_OR,tmp,lval,rval));//中间代码tmp=lval||rval
	return tmp;
}

/*
	与运算语句
*/
Var* GenIR::GenAnd(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_AND,tmp,lval,rval));//中间代码tmp=lval&&rval
	return tmp;
}

/*
	大于语句
*/
Var* GenIR::GenGt(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_GT,tmp,lval,rval));//中间代码tmp=lval>rval
	return tmp;
}

/*
	大于等于语句
*/
Var* GenIR::GenGe(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_GE,tmp,lval,rval));//中间代码tmp=lval>=rval
	return tmp;
}

/*
	小于语句
*/
Var* GenIR::GenLt(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_LT,tmp,lval,rval));//中间代码tmp=lval<rval
	return tmp;
}

/*
	小于等于语句
*/
Var* GenIR::GenLe(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_LE,tmp,lval,rval));//中间代码tmp=lval<=rval
	return tmp;
}

/*
	等于语句
*/
Var* GenIR::GenEqu(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_EQU,tmp,lval,rval));//中间代码tmp=lval==rval
	return tmp;
}

/*
	不等于语句
*/
Var* GenIR::GenNequ(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_NE,tmp,lval,rval));//中间代码tmp=lval!=rval
	return tmp;
}

/*
	加法语句
*/
Var* GenIR::GenAdd(Var*lval,Var*rval)
{
	Var*tmp=NULL;
	//指针和数组只能和基本类型相加
	if((lval->getArray()||lval->getPtr())&&rval->isBase()){
		tmp=new Var(symtab.GetScopePath(),lval);
		rval=GenMul(rval,Var::getStep(lval));
	}
	else if(rval->isBase()&&(rval->getArray()||rval->getPtr())){
		tmp=new Var(symtab.GetScopePath(),rval);
		lval=GenMul(lval,Var::getStep(rval));
	}
	else if(lval->isBase() && rval->isBase()){//基本类型
		tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	}
	else{
		SEMERROR(EXPR_NOT_BASE);//加法类型不兼容
		return lval;
	}
	//加法命令
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_ADD,tmp,lval,rval));//中间代码tmp=lval+rval
	return tmp;
}

/*
	减法语句
*/
Var* GenIR::GenSub(Var*lval,Var*rval)
{
	Var*tmp=NULL;
	if(!rval->isBase())
	{
		SEMERROR(EXPR_NOT_BASE);//类型不兼容,减数不是基本类型
		return lval;
	}
	//指针和数组
	if((lval->getArray()||lval->getPtr())){
		tmp=new Var(symtab.GetScopePath(),lval);
		rval=GenMul(rval,Var::getStep(lval));
	}
	else{//基本类型
		tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	}
	//减法命令
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_SUB,tmp,lval,rval));//中间代码tmp=lval-rval
	return tmp;
}

/*
	乘法语句
*/
Var* GenIR::GenMul(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_MUL,tmp,lval,rval));//中间代码tmp=lval*rval
	return tmp;
}

/*
	除法语句
*/
Var* GenIR::GenDiv(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_DIV,tmp,lval,rval));//中间代码tmp=lval/rval
	return tmp;
}

/*
	模语句
*/
Var* GenIR::GenMod(Var*lval,Var*rval)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//基本类型
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_MOD,tmp,lval,rval));//中间代码tmp=lval%rval
	return tmp;
}

/*
	左单目运算语句
*/
Var* GenIR::GenOneOpLeft(Tag opt,Var*val)
{
	if(!val)return NULL;
	if(val->isVoid()){
		SEMERROR(EXPR_IS_VOID);//void函数返回值不能出现在表达式中
		return NULL;
	}
	//&x *p 运算单独处理
	if(opt==LEA)return GenLea(val);//取址语句
	if(opt==MUL)return GenPtr(val);//指针取值语句
	//++ --
	if(opt==INC)return GenIncL(val);//左自加语句
	if(opt==DEC)return GenDecL(val);//左自减语句
	//not minus ++ --
	if(val->IsRef())val=GenAssign(val);//处理(*p)
	if(opt==NOT)return GenNot(val);//not语句
	if(opt==SUB)return GenMinus(val);//取负语句
	return val;
}

/*
	取反
*/
Var* GenIR::GenNot(Var*val)
{
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//生成整数
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_NOT,tmp,val));//中间代码tmp=-val
	return tmp;
}

/*
	取负
*/
Var* GenIR::GenMinus(Var*val)
{
	if(!val->isBase()){
		SEMERROR(EXPR_NOT_BASE);//运算对象不是基本类型
		return val;
	}
	Var*tmp=new Var(symtab.GetScopePath(),KW_INT,false);//生成整数
	symtab.AddVar(tmp);
	symtab.AddInst(new InterInst(OP_NEG,tmp,val));//中间代码tmp=-val
	return tmp;
}

/*
	左自加
*/
Var* GenIR::GenIncL(Var*val)
{
	if(!val->getLeft()){
		SEMERROR(EXPR_NOT_LEFT_VAL);
		return val;
	}
	if(val->IsRef()){//++*p情况 => t1=*p t2=t1+1 *p=t2
		Var* t1=GenAssign(val);//t1=*p
		Var* t2=GenAdd(t1,Var::getStep(val));//t2=t1+1
		return GenAssign(val,t2);//*p=t2
	}
	symtab.AddInst(new InterInst(OP_ADD,val,val,Var::getStep(val)));//中间代码++val
	return val;
}

/*
	左自减
*/
Var* GenIR::GenDecL(Var*val)
{
	if(!val->getLeft()){
		SEMERROR(EXPR_NOT_LEFT_VAL);
		return val;
	}
	if(val->IsRef()){//--*p情况 => t1=*p t2=t1-1 *p=t2
		Var* t1=GenAssign(val);//t1=*p
		Var* t2=GenSub(t1,Var::getStep(val));//t2=t1-1
		return GenAssign(val,t2);//*p=t2
	}
	symtab.AddInst(new InterInst(OP_SUB,val,val,Var::getStep(val)));//中间代码--val
	return val;
}

/*
	取址语句
*/
Var* GenIR::GenLea(Var*val)
{
	if(!val->getLeft()){
		SEMERROR(EXPR_NOT_LEFT_VAL);//不能取地址
		return val;
	}
	if(val->IsRef())//类似&*p运算
		return val->getPointer();//取出变量的指针,&*(val->ptr)等价于ptr
	else{//一般取地址运算
		Var* tmp=new Var(symtab.GetScopePath(),val->getType(),true);//产生局部变量tmp
		symtab.AddVar(tmp);//插入声明
		symtab.AddInst(new InterInst(OP_LEA,tmp,val));//中间代码tmp=&val
		return tmp;
	}
}

/*
	指针取值语句
*/
Var* GenIR::GenPtr(Var*val)
{
	if(val->isBase()){
		SEMERROR(EXPR_IS_BASE);//基本类型不能取值
		return val; 
	}
	Var*tmp=new Var(symtab.GetScopePath(),val->getType(),false);
	tmp->setLeft(true);//指针运算结果为左值
	tmp->setPointer(val);//设置指针变量
	symtab.AddVar(tmp);//产生表达式需要根据使用者判断，推迟！
	return tmp;
}

/*
	右单目运算语句
*/
Var* GenIR::GenOneOpRight(Var*val,Tag opt)
{
	if(!val)return NULL;
	if(val->isVoid()){
		SEMERROR(EXPR_IS_VOID);//void函数返回值不能出现在表达式中
		return NULL;
	}
	if(!val->getLeft()){
		SEMERROR(EXPR_NOT_LEFT_VAL);
		return val;
	}
	if(opt==INC)return GenIncR(val);//右自加语句
	if(opt==DEC)return GenDecR(val);//右自减语句
	return val;
}

/*
	右自加
*/
Var* GenIR::GenIncR(Var*val)
{
	Var*tmp=GenAssign(val);//拷贝
	symtab.AddInst(new InterInst(OP_ADD,val,val,Var::getStep(val)));//中间代码val++
	return tmp;
}

/*
	右自减
*/
Var* GenIR::GenDecR(Var*val)
{
	Var*tmp=GenAssign(val);//拷贝
	symtab.AddInst(new InterInst(OP_SUB,val,val,Var::getStep(val)));//val--
	return tmp;
}


/*
	检查类型是否可以转换
*/
bool GenIR::typeCheck(Var*lval,Var*rval)
{
	bool flag=false;
	if(!rval)return false;
	if(lval->isBase()&&rval->isBase())//都是基本类型
		flag=true;
	else if(!lval->isBase() && !rval->isBase())//都不是基本类型
		flag=rval->getType()==lval->getType();//只要求类型相同
	return flag;
}

/*
	产生while循环头部
*/
void GenIR::GenWhileHead(InterInst*& _while,InterInst*& _exit)
{

	// InterInst* _blank=new InterInst();//_blank标签
	// symtab.AddInst(new InterInst(OP_JMP,_blank));//goto _blank
	

	_while=new InterInst();//产生while标签
	symtab.AddInst(_while);//添加while标签

	// symtab.AddInst(_blank);//添加_blank标签

	_exit=new InterInst();//产生exit标签
	push(_while,_exit);//进入while
}

/*
	产生while条件
*/
void GenIR::GenWhileCond(Var*cond,InterInst* _exit)
{
	if(cond){
		if(cond->isVoid())cond=Var::getTrue();//处理空表达式
		else if(cond->IsRef())cond=GenAssign(cond);//while(*p),while(a[0])
		symtab.AddInst(new InterInst(OP_JF,_exit,cond));
	}
}

/*
	产生while尾部
*/
void GenIR::GenWhileTail(InterInst*& _while,InterInst*& _exit)
{
	symtab.AddInst(new InterInst(OP_JMP,_while));//添加jmp指令
	symtab.AddInst(_exit);//添加exit标签
	pop();//离开while
}

/*
	产生do-while循环头部
*/
void GenIR::GenDoWhileHead(InterInst*& _do,InterInst*& _exit)
{
	_do=new InterInst();//产生do标签
	_exit=new InterInst();//产生exit标签
	symtab.AddInst(_do);
	push(_do,_exit);//进入do-while
}

/*
	产生do-while尾部
*/
void GenIR::GenDoWhileTail(Var*cond,InterInst* _do,InterInst* _exit)
{
	if(cond){
		if(cond->isVoid())cond=Var::getTrue();//处理空表达式
		else if(cond->IsRef())cond=GenAssign(cond);//while(*p),while(a[0])
		symtab.AddInst(new InterInst(OP_JT,_do,cond));
	}
	symtab.AddInst(_exit);
	pop();
}

/*
	产生for循环头部
*/
void GenIR::GenForHead(InterInst*& _for,InterInst*& _exit)
{
	_for=new InterInst();//产生for标签
	_exit=new InterInst();//产生exit标签
	symtab.AddInst(_for);
}

/*
	产生for条件开始部分
*/
void GenIR::GenForCondBegin(Var*cond,InterInst*& _step,InterInst*& _block,InterInst* _exit)
{
	_block=new InterInst();//产生block标签
	_step=new InterInst();//产生循环动作标签
	if(cond){
		if(cond->isVoid())cond=Var::getTrue();//处理空表达式
		else if(cond->IsRef())cond=GenAssign(cond);//for(*p),for(a[0])
		symtab.AddInst(new InterInst(OP_JF,_exit,cond));
		symtab.AddInst(new InterInst(OP_JMP,_block));//执行循环体
	}
	symtab.AddInst(_step);//添加循环动作标签
	push(_step,_exit);//进入for
}

/*
	产生for条件结束部分
*/
void GenIR::GenForCondEnd(InterInst* _for,InterInst* _block)
{
	symtab.AddInst(new InterInst(OP_JMP,_for));//继续循环
	symtab.AddInst(_block);//添加循环体标签
}

/*
	产生for尾部
*/
void GenIR::GenForTail(InterInst*& _step,InterInst*& _exit)
{
	symtab.AddInst(new InterInst(OP_JMP,_step));//跳转到循环动作
	symtab.AddInst(_exit);//添加_exit标签
	pop();//离开for
}

/*
	产生if头部
*/
void GenIR::GenIfHead(Var*cond,InterInst*& _else)
{
	_else=new InterInst();//产生else标签
	if(cond){
		if(cond->IsRef())cond=GenAssign(cond);//if(*p),if(a[0])
		symtab.AddInst(new InterInst(OP_JF,_else,cond));
	}
}

/*
	产生if尾部
*/
void GenIR::GenIfTail(InterInst*& _else)
{
	symtab.AddInst(_else);
}

/*
	产生else头部
*/
void GenIR::GenElseHead(InterInst* _else,InterInst*& _exit)
{
	_exit=new InterInst();//产生exit标签
	symtab.AddInst(new InterInst(OP_JMP,_exit));
	symtab.AddInst(_else);
}

/*
	产生else尾部
*/
void GenIR::GenElseTail(InterInst*& _exit)
{
	symtab.AddInst(_exit);
}

/*
	产生switch头部
*/
void GenIR::GenSwitchHead(InterInst*& _exit)
{
	_exit=new InterInst();//产生exit标签
	push(NULL,_exit);//进入switch，不允许continue，因此head=NULL
}

/*
	产生switch尾部
*/
void GenIR::GenSwitchTail(InterInst* _exit)
{
	symtab.AddInst(_exit);//添加exit标签
	pop();
}

/*
	产生case头部
*/
void GenIR::GenCaseHead(Var*cond,Var*lb,InterInst*& _case_exit)
{
	_case_exit=new InterInst();//产生case的exit标签
	if(lb)symtab.AddInst(new InterInst(OP_JNE,_case_exit,cond,lb));//if(cond!=lb)goto _case_exit
}

/*
	产生case尾部
*/
void GenIR::GenCaseTail(InterInst* _case_exit)
{
	symtab.AddInst(_case_exit);//添加case的exit标签
	// InterInst * _case_exit_append=new InterInst();//产生case的exit附加标签
	// symtab.AddInst(new InterInst(OP_JMP,_case_exit_append));//goto _case_exit_append
	// symtab.AddInst(_case_exit);//添加case的exit标签
	// symtab.AddInst(_case_exit_append);//添加case的exit附加标签
}

/*
	添加一个作用域
*/
void GenIR::push(InterInst*head,InterInst*tail)
{
	heads.push_back(head);
	tails.push_back(tail);
}

/*
	删除一个作用域
*/
void GenIR::pop()
{
	heads.pop_back();
	tails.pop_back();
}

/*
	产生break语句
*/
void GenIR::GenBreak()
{
	InterInst*tail=tails.back();//取出跳出标签
	if(tail)symtab.AddInst(new InterInst(OP_JMP,tail));//goto tail
	else SEMERROR(BREAK_ERR);//break不在循环或switch-case中
}

/*
	产生continue语句
*/
void GenIR::GenContinue()
{
	InterInst*head=heads.back();//取出跳出标签
	if(head)symtab.AddInst(new InterInst(OP_JMP,head));//goto head
	else SEMERROR(CONTINUE_ERR);//continue不在循环中
}

/*
	产生return语句
*/
void GenIR::GenReturn(Var*ret)
{
	if(!ret)return;
	Fun*fun=symtab.GetCurFun();
	if(ret->isVoid()&&fun->getType()!=KW_VOID||ret->isBase()&&fun->getType()==KW_VOID){//类型不兼容
		SEMERROR(RETURN_ERR);//return语句和函数返回值类型不匹配
		return;
	}
	InterInst* returnPoint=fun->getReturnPoint();//获取返回点
	if(ret->isVoid())symtab.AddInst(new InterInst(OP_RET,returnPoint));//return returnPoint
	else{
		if(ret->IsRef())ret=GenAssign(ret);//处理ret是*p情况
		symtab.AddInst(new InterInst(OP_RETV,returnPoint,ret));//return returnPoint ret
	}
}

/*
	产生变量初始化语句
*/
bool GenIR::GenVarInit(Var*var)
{
	if(var->getName()[0]=='<')return 0;
	symtab.AddInst(new InterInst(OP_DEC,var));//添加变量声明指令
	if(var->setInit())//初始化语句
		GenTwoOp(var,ASSIGN,var->getInitData());//产生赋值表达式语句 name=init->name
	return 1;
}

/*
	产生函数入口语句
*/
void GenIR::GenFunHead(Fun*function)
{
	function->enterScope();//进入函数作用域
	symtab.AddInst(new InterInst(OP_ENTRY,function));//添加函数入口指令
	function->setReturnPoint(new InterInst);//创建函数的返回点
}

/*
	产生函数出口语句
*/
void GenIR::GenFunTail(Fun*function)
{
	symtab.AddInst(function->getReturnPoint());//添加函数返回点，return的目的标号
	symtab.AddInst(new InterInst(OP_EXIT,function));//添加函数出口指令
	function->leaveScope();//退出函数作用域
}
