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


#include "token.h"
#include <sstream>
#include "error.h"

// Lexcial name
const char * tokenName[]=
{
	"error",//错误，异常，结束标记等
	"文件结尾",//文件结束
	"标识符",//标识符
	"int","char","void",//数据类型
	"extern",//extern
	"数字","字符","字符串",//字面量
	"!","&",//单目运算 ! - & *
	"+","-","*","/","%",//算术运算符
	"++","--",
	">",">=","<","<=","==","!=",//比较运算符
	"&&","||",//逻辑运算
	"(",")",//()
	"[","]",//[]
	"{","}",//{}
	",",":",";",//逗号,冒号,分号
	"=",//赋值
	"if","else",//if-else
	"switch","case","default",//swicth-case-deault
	"while","do","for",//循环
	"break","continue","return"//break,continue,return
};

Token::Token (Tag t):tag(t)
{}

string Token::toString()
{
	return tokenName[tag];
}

Token::~Token ()
{}


Identifier::Identifier (string n):Token(IDENTIFIER),name(n)
{}

string Identifier::toString()
{
	return Token::toString()+name;
}


String::String (string s):Token(STR),str(s)
{}

string String::toString()
{
	return string("[")+Token::toString()+"]:"+str;
}


Number::Number (int v):Token(NUM),val(v)
{}

string Number::toString()
{
	stringstream ss;
	ss<<val;
	return string("[")+Token::toString()+"]:"+ss.str();
}

Char::Char (char c):Token(CH),ch(c)
{}

string Char::toString()
{
	stringstream ss;
	ss<<ch;
	return string("[")+Token::toString()+"]:"+ss.str();
}

