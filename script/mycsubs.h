////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2001 mamaich
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

// 1.0
//---------------------------------------------------------------------------
#ifndef mycsubsH
#define mycsubsH

#include <pshpack4.h>

// ���� ������������ ���� ������ ���������� � ����� DLL, ����������
// ���������� �������/�������� ��� �������
// ��������������� mycsubs.cpp �������� � ������ � DLL ������!!!
// �� ������ ���������� � DLL/EXE � ��������

// This file is used for writing functions/classes accessible from script
// on non-C++ language. It is also used to move these functions to DLL
// compiled with other C-dialect. Exporting classes would cause problems.


//---------------------------------------------------------------------------

struct LibraryFunctions;
// ��� ���������� ������� ���������� �� ���������� �������� ��������� ������
typedef struct __ParserVariable {} ParserVariable;
typedef struct __ParserObject {} ParserObject;

// ��� �������, ����������� ����� �������� ������. ���������� NULL = Ok,
// � ������ ������ - ��������� �� ������ � �������
typedef const char* __cdecl ExternalCFunction(LibraryFunctions *Table,
    ParserVariable *Result, ParserVariable *Params[], int ParamCount, ParserObject *Parser);

#ifndef __has_c_func_table
#define __has_c_func_table
// ��������� �����, ���� �� ����� ���� �������� myparser.h
struct CFuncTable
{
    const char *Name;
    ExternalCFunction *Function;
    int ParamCount;    // -1 - �� ��������� �� ���-��
};
struct CPropTable
{
    const char *Name;
    ExternalCFunction *Get;
    ExternalCFunction *Set;
};
#endif

// ��� ���������� ���������� � �������, ����������� ������
// ��� �������������� ��� ������ � ParserVariable

typedef int __cdecl __RtlFi(const ParserVariable*);
typedef const char* __cdecl __RtlFs(const ParserVariable*);
typedef double __cdecl __RtlFd(const ParserVariable*);
typedef void* __cdecl __RtlFv(const ParserVariable*);
typedef void __cdecl __RtlFe(const ParserVariable*, int);
typedef void __cdecl __RtlFss(const ParserVariable*, const char*);
typedef void __cdecl __RtlFsd(const ParserVariable*, double);
typedef void __cdecl __RtlFsv(const ParserVariable*, const void *);
typedef ParserVariable* __cdecl __RtlFsp(const ParserVariable*, int);
typedef ParserVariable* __cdecl __RtlFp();
typedef void __cdecl __RtlFpd(const ParserVariable*);
typedef ParserVariable* __cdecl __RtlFpp(const ParserVariable*);

typedef bool __cdecl __RtlPex(ParserObject *Parser, ParserVariable *Result,
    const char *Func, ParserVariable *Params[], int ParamCount);
typedef void __cdecl __RtlPv(ParserObject *Parser);
typedef void __cdecl __RtlPcp(ParserObject *Parser, const char* Name, const ParserVariable *Val);
typedef ParserVariable* __cdecl __RtlPcg(ParserObject *Parser, const char* Name);

typedef void __cdecl __RtlPpf(ParserObject *Parser, const struct CFuncTable *Table);
typedef void __cdecl __RtlPcf(ParserObject *Parser, const char *Class,const struct CFuncTable *Table);
typedef void __cdecl __RtlPpp(ParserObject *Parser, const char * Class, const struct CPropTable * Prop);

// ������ �������� ��������������� ����� �� myvar.h
enum ParserVarType {T_Error, T_Class, T_Number, T_String, T_Array};

struct LibraryFunctions
{
    int Size;   // ������ ��������� ��� �������� ������
// ���������� ��� ���������� ParserVariable, ���������� ����������
    __RtlFi *GetType;
// ���������� ��� ����������
    __RtlFe *SetType;

// ������� ��������� �� const char* ��������������� ������ ParserVariable
    __RtlFs *GetString;
// ������� double �������������� ...
    __RtlFd *GetNumber;
// �������� ���� ������
    __RtlFv *GetUserData;

// ���������� ������ � ���������� � ������ �������� (��� ���. �� ��������!!!)
    __RtlFss *SetString;
// ���������� �������� ����� ...
    __RtlFsd *SetNumber;
// ���������� ���� ������
    __RtlFsv *SetUserData;

// ��������� ParserVariable, � ������������ � ������ ������
    __RtlFss *CreateClass;

// �������� �������� ��-�� �������
    __RtlFsp *TakeArrayValue;

// ������� ����� ������ ParserVariable
    __RtlFp  *NewVar;
// ������� ��������� ������
    __RtlFpd  *DelVar;
// ������� ����� ������ ParserVariable ��������� ������ �����������
// ��������� ����� ������������� ����� ������� DelVar
    __RtlFpp  *DupVar;


// ����� ������� �� ������� ��� ������� ������� ������� � ��������� ����������
// ���������� true==Ok, false - Result �������� �������� ������
    __RtlPex *Execute;

// ��������� ������������� ������ �������� �������. ������
    __RtlPv *Terminate;

// ����� ��������������� ������� �� TParser
    __RtlPcp *SetGlobalVariable;
    __RtlPpf *SetFunctions;
    __RtlPcf *SetClass;
    __RtlPpp *SetProperties;

// �������� �������� ��. ����������. ������������ ���������
// ����� ������� DelVar
    __RtlPcg *GetGlobalVariable;

// ������� TVariable � ������ �������
    __RtlFss *Error;
};

#include <poppack.h>

LibraryFunctions *GetLibraryFunctions();

#endif
