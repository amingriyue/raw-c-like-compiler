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

extern const char * tokenName[];

// Lexcial word
class Token
{
public:
	Tag tag;
	Token (Tag t);
	virtual string toString();
	virtual ~Token ();
};


// Identifier
class Identifier:public Token
{
public:
	string name;
	Identifier (string n);
	virtual string toString();
};


// String
class String:public Token
{
public:
	string str;
	String (string s);
	virtual string toString();
};


// Logical number
class Number:public Token
{
public:
	int val;
	Number (int v);
	virtual string toString();
};


// Charactor
class Char:public Token
{
public:
	char ch;
	Char (char c);
	virtual string toString();
};
