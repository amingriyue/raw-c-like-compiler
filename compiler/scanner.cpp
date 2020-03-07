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

#include "scanner.h"

namespace Compiler
{

// =====================================================================================================================
Scanner::Scanner(string srcFile)
    :
    m_srcFile(srcFile),
    m_bufferLength(0),
    m_readPosition(-1),
    m_line(1),
    m_column(0),
    m_lastElement(-1),
    m_cursor(' ')
{
	//add keyword mapping here ~
	m_keywords["int"]       = KW_INT;
	m_keywords["char"]      = KW_CHAR;
	m_keywords["void"]      = KW_VOID;
	m_keywords["extern"]    = KW_EXTERN;
	m_keywords["if"]        = KW_IF;
	m_keywords["else"]      = KW_ELSE;
	m_keywords["switch"]    = KW_SWITCH;
	m_keywords["case"]      = KW_CASE;
	m_keywords["default"]   = KW_DEFAULT;
	m_keywords["while"]     = KW_WHILE;
	m_keywords["do"]        = KW_DO;
	m_keywords["for"]       = KW_FOR;
	m_keywords["break"]     = KW_BREAK;
	m_keywords["continue"]  = KW_CONTINUE;
	m_keywords["return"]    = KW_RETURN;
}

// =====================================================================================================================
void Scanner::Init()
{
    printf("%s\n", m_srcFile.c_str());
    m_pSrcHandle = fopen(m_srcFile.c_str(), "r");

    int fileNameStart = m_srcFile.rfind("/");
    int fileNameEnd = m_srcFile.rfind(".");

    if (fileNameStart == -1)
    {
        fileNameStart = 0;
    }

    string asmFile = "../out/" + m_srcFile.substr(fileNameStart, fileNameEnd - fileNameStart) + ".s";
    string irFile = "../out/" + m_srcFile.substr(fileNameStart, fileNameEnd - fileNameStart) + ".ir";
    m_pOutHandle = fopen(asmFile.c_str(), "w");
    m_pIrHandle = fopen(irFile.c_str(), "w");
}

// =====================================================================================================================
void Scanner::Destroy()
{
    printf("%s\n", m_srcFile.c_str());
    fclose(m_pSrcHandle);
    fclose(m_pOutHandle);
    fclose(m_pIrHandle);
}

// =====================================================================================================================
char Scanner::ScanFile(FILE *pSrcFile)
{
    if (!pSrcFile)
    {
        return -1;
    }

    if (m_readPosition == m_bufferLength - 1)
    {
        m_bufferLength = fread(m_pBuffer, 1, BUFLEN, pSrcFile);
        if (m_bufferLength== 0)
        {
            m_bufferLength = 1;
            m_pBuffer[0] = -1;
        }
        m_readPosition = -1;
    }

    m_readPosition++;

    char element = m_pBuffer[m_readPosition];

    if (element == -1)
    {
        fclose(pSrcFile);
        pSrcFile = NULL;
    }
    else if (element != '\n')
    {
        m_column++;
    }

    if (m_lastElement == '\n')
    {
        m_line++;
        m_column = 0;
    }

    m_lastElement = element;

    return element;
}

// =====================================================================================================================
Tag Scanner::GetTag(string name)
{
    return m_keywords.find(name) != m_keywords.end() ? m_keywords[name] : IDENTIFIER;
}

// =====================================================================================================================
// Match DFA, parse lexical tokens
Token* Scanner::Tokenize()
{
    // -1 is a invalid token
	for(; m_cursor != -1;){
		Token* token = NULL;
        // ignore bank space
		while(m_cursor == ' ' || m_cursor == '\n' || m_cursor == '\t')
        {
            m_cursor = ScanFile(m_pSrcHandle);
        }
		// Identifier and keyword, which begins with non-digit, but can be followed by digit.
		if(m_cursor >= 'a' && m_cursor <= 'z' || m_cursor>='A' && m_cursor <= 'Z' || m_cursor == '_')
        {
			string name="";
			do{
				name.push_back(m_cursor);
				m_cursor = ScanFile(m_pSrcHandle);
			}while(m_cursor >= 'a' && m_cursor <= 'z' || m_cursor >= 'A' && m_cursor <= 'Z' || m_cursor == '_' ||
                    m_cursor >= '0' && m_cursor <= '9');

            if (m_keywords.find(name) == m_keywords.end())
            {
				token = new Identifier(name);
            }
			else
            {
                // Keyword
			    Tag tag = m_keywords[name];
				token = new Token(tag);
            }
		}
		// string token
		else if(m_cursor == '"')
        {
			string str = "";
			while((m_cursor = ScanFile(m_pSrcHandle)) != '"')
            {
                // escape
				if(m_cursor == '\\')
                {
                    m_cursor = ScanFile(m_pSrcHandle);
                    switch (m_cursor) {
                        case 'n':
                            str.push_back('\n');
                            break;
                        case '\\':
                            str.push_back('\\');
                            break;
                        case 't':
                            str.push_back('\t');
                            break;
                        case '"':
                            str.push_back('"');
                            break;
                        case '0':
                            str.push_back('\0');
                            break;
                        case '\n':
                            break;
					    case -1:
						    token = new Token(ERROR);
						    break;
                        default:
                            str.push_back(m_cursor);
                    }
				}
				else if(m_cursor == '\n' || m_cursor == -1)
                {
                    // end of file
                    printf("%s<line: %d, column: %d> lexical error : end of file.\n", m_srcFile.c_str(), m_line, m_column);
					token =new Token(ERROR);
					break;
				}
				else
					str.push_back(m_cursor);
			}
			// string
			if(!token)
            {
                token = new String(str);
            }
		}
		// digit
		else if(m_cursor >= '0' && m_cursor <= '9'){
			int val = 0;
            // decimal, begin with non zero
			if(m_cursor != '0'){
				do{
					val = val * 10 + m_cursor - '0';
					m_cursor = ScanFile(m_pSrcHandle);
				}while(m_cursor >= '0' && m_cursor <= '9');
			}
			else{
				m_cursor = ScanFile(m_pSrcHandle);
                // hexadecimal
				if(m_cursor == 'x'){
				    m_cursor = ScanFile(m_pSrcHandle);
					if(m_cursor >= '0' && m_cursor <= '9' || m_cursor >= 'A' && m_cursor <= 'F' ||
                       m_cursor >= 'a' && m_cursor <= 'f'){
						do{
							val = val * 16 + m_cursor;
							if(m_cursor >= '0' && m_cursor <= '9')
                                val -= '0';
							else if(m_cursor >= 'A' && m_cursor <= 'F')
                                val += 10 - 'A';
							else if(m_cursor >= 'a' && m_cursor <= 'f')
                                val += 10 - 'a';							
				            m_cursor = ScanFile(m_pSrcHandle);
						}while(m_cursor >= '0' && m_cursor <= '9' || m_cursor >= 'A' && m_cursor <= 'F' ||
                               m_cursor >= 'a' && m_cursor <= 'f');
					}
					else{
						// there isn't m_cursor after 0x
                        printf("%s<line: %d, column: %d> lexical error : no data after 0x.\n", m_srcFile.c_str(), m_line, m_column);
						token = new Token(ERROR);
					}
				}
                // binary
				else if(m_cursor == 'b'){
				    m_cursor = ScanFile(m_pSrcHandle);
					if(m_cursor >='0' && m_cursor <= '1'){
						do{
							val = val * 2 + m_cursor - '0';
				            m_cursor = ScanFile(m_pSrcHandle);
						}while(m_cursor >='0' && m_cursor <= '1');
					}
					else{
                        // there isnt m_cursor after 0b
                        printf("%s<line: %d, column: %d> lexical error : no data after 0b.\n", m_srcFile.c_str(), m_line, m_column);
						token = new Token(ERROR);
					}
				}
                // octal
				else if( m_cursor >= '0' && m_cursor <= '7'){
					do{
						val = val * 8 + m_cursor - '0';
				        m_cursor = ScanFile(m_pSrcHandle);
					}while(m_cursor >= '0' && m_cursor <= '7');
				}
			}
            // final digit
			if(!token)
            {
                token = new Number(val);
            }
		}
		// char
		else if(m_cursor == '\''){
			char c;

			m_cursor = ScanFile(m_pSrcHandle);
            // escape
			if(m_cursor == '\\')
            {
			    m_cursor = ScanFile(m_pSrcHandle);

                switch (m_cursor)
                {
				    case 'n':
                        c = '\n';
                        break;
                    case '\\':
                        c = '\\';
                        break;
				    case 't':
                        c = '\t';
                        break;
				    case '0':
                        c = '\0';
                        break;
				    case '\'':
                        c = '\'';
                        break;
				    case -1:
                    case '\n':
                        printf("%s<line: %d, column: %d> lexical error : no right quotation.\n", m_srcFile.c_str(), m_line, m_column);
					    token = new Token(ERROR);
                        break;
                    default:
				        c = m_cursor;
                }
			}
			else if(m_cursor == '\n' || m_cursor == -1){
                printf("%s<line: %d, column: %d> lexical error : no right quotation.\n", m_srcFile.c_str(), m_line, m_column);
				token = new Token(ERROR);
			}
			else if(m_cursor == '\''){
                printf("%s<line: %d, column: %d> lexical error : no data.\n", m_srcFile.c_str(), m_line, m_column);
				token = new Token(ERROR);
			    m_cursor = ScanFile(m_pSrcHandle);
			}
			else
            {
                c = m_cursor;
            }
			if(!token)
            {
			    m_cursor = ScanFile(m_pSrcHandle);
				if(m_cursor == '\'')
                {
					token = new Char(c);
				}
				else
                {
                    printf("%s<line: %d, column: %d> lexical error : no right quotation.\n", m_srcFile.c_str(), m_line, m_column);
					token = new Token(ERROR);
				}
			}
		}
		else
        {
            // bounds
			switch(m_cursor)
			{
				case '#':
					while(m_cursor != '\n' && m_cursor != -1)
			            m_cursor = ScanFile(m_pSrcHandle);
					token = new Token(ERROR);
					break;
				case '+':
			        m_cursor = ScanFile(m_pSrcHandle);
                    if (m_cursor != '+')
                    {
					    token = new Token(ADD);
                    }
                    else
                    {
			            m_cursor = ScanFile(m_pSrcHandle);
					    token = new Token(INC);
                    }
                    break;
				case '-':
			        m_cursor = ScanFile(m_pSrcHandle);
                    if (m_cursor != '-')
                    {
					    token = new Token(SUB);
                    }
                    else
                    {
			            m_cursor = ScanFile(m_pSrcHandle);
					    token = new Token(DEC);
                    }
                    break;
				case '*':
					token = new Token(MUL);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case '/':
			        m_cursor = ScanFile(m_pSrcHandle);
                    // signal line comment
					if(m_cursor == '/'){
						while(m_cursor != '\n' && m_cursor != -1)
			                m_cursor = ScanFile(m_pSrcHandle);
						token = new Token(ERROR);
					}
                    // multiple line comment
					else if(m_cursor == '*')
                    {
						while((m_cursor = ScanFile(m_pSrcHandle)) != -1){
							if(m_cursor =='*')
                            {
			                    m_cursor = ScanFile(m_pSrcHandle);
								if(m_cursor == '/')
                                    break;
							}
						}
						if(m_cursor == -1)
                            printf("%s<line: %d, column: %d> lexical error : no end.\n", m_srcFile.c_str(), m_line, m_column);
						token = new Token(ERROR);
					}
					else
                    {
						token = new Token(DIV);
                    }
					break;
				case '%':
					token = new Token(MOD);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case '>':
			        m_cursor = ScanFile(m_pSrcHandle);
                    if (m_cursor != '=')
                    {
					    token = new Token(GT);
                    }
                    else
                    {
			            m_cursor = ScanFile(m_pSrcHandle);
					    token = new Token(GE);
                    }
                    break;
				case '<':
			        m_cursor = ScanFile(m_pSrcHandle);
                    if (m_cursor != '=')
                    {
					    token = new Token(LT);
                    }
                    else
                    {
			            m_cursor = ScanFile(m_pSrcHandle);
					    token = new Token(LE);
                    }
                    break;
				case '=':
			        m_cursor = ScanFile(m_pSrcHandle);
                    if (m_cursor != '=')
                    {
					    token = new Token(ASSIGN);
                    }
                    else
                    {
			            m_cursor = ScanFile(m_pSrcHandle);
					    token = new Token(EQU);
                    }
                    break;
				case '&':
			        m_cursor = ScanFile(m_pSrcHandle);
                    if (m_cursor != '&')
                    {
					    token = new Token(LEA);
                    }
                    else
                    {
			            m_cursor = ScanFile(m_pSrcHandle);
					    token = new Token(AND);
                    }
                    break;
				case '|':
			        m_cursor = ScanFile(m_pSrcHandle);
                    if (m_cursor != '|')
                    {
                        printf("%s<line: %d, column: %d> lexical error : no pair for OR.\n", m_srcFile.c_str(), m_line, m_column);
					    token = new Token(ERROR);
                    }
                    else
                    {
			            m_cursor = ScanFile(m_pSrcHandle);
					    token = new Token(OR);
                    }
					break;
				case '!':
			        m_cursor = ScanFile(m_pSrcHandle);
                    if (m_cursor != '=')
                    {
					    token = new Token(NOT);
                    }
                    else
                    {
			            m_cursor = ScanFile(m_pSrcHandle);
					    token = new Token(NEQU);
                    }
                    break;
				case ',':
					token = new Token(COMMA);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case ':':
					token = new Token(COLON);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case ';':
					token = new Token(SEMICON);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case '(':
					token = new Token(LPAREN);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case ')':
					token = new Token(RPAREN);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case '[':
					token = new Token(LBRACK);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case ']':
					token = new Token(RBRACK);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case '{':
					token = new Token(LBRACE);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case '}':
					token = new Token(RBRACE);
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				case -1:
			        m_cursor = ScanFile(m_pSrcHandle);
                    break;
				default:
					token =new Token(ERROR);
                    printf("%s<line: %d, column: %d> lexical error : no exist.\n", m_srcFile.c_str(), m_line, m_column);
			        m_cursor = ScanFile(m_pSrcHandle);
			}
		}
		if(token && token->tag != ERROR)
			return token;
		else
			continue;
	}

	return new Token(END);
}

} // Compiler
