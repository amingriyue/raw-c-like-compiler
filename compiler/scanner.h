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

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <map>

#include "common.h"
#include "token.h"

using namespace std;
namespace Compiler
{

#define BUFLEN 100

// =====================================================================================================================
class Scanner
{
public:
    Scanner(string srcFile);
    ~Scanner() { Destroy(); }

    void Init();
    void Destroy();

    char ScanFile(FILE *pSrcFile);
    Tag GetTag(string name);
    Token* Tokenize();
    string GetFile() { return m_srcFile; }
    int GetLine() { return m_line; }
    int GetColumn() { return m_column; }
    FILE* GetOutHandle() { return m_pOutHandle; }
    FILE* GetIrHandle() { return m_pIrHandle; }

private:
    string                      m_srcFile;
    FILE*                       m_pSrcHandle;
    FILE*                       m_pOutHandle;
    FILE*                       m_pIrHandle;

    int                         m_bufferLength;
    int                         m_readPosition;
    char                        m_pBuffer[BUFLEN];
    int                         m_line;
    int                         m_column;
    char                        m_lastElement;
    char                        m_cursor; // point to last reading

    std::map<string, Tag>       m_keywords;
};
} // Compiler
