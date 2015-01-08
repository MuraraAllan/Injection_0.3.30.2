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
#ifndef MY_PARSER_H
#define MY_PARSER_H

#include "myvar.h"
#include "myutil.h"
#include "mycsubs.h"

#include <pshpack4.h>

#ifndef __has_c_func_table
#define __has_c_func_table
// ��-�� ����������� ������ ��� ��������� mycsubs.h �������� ����� �����������
// �������� ����. ��� ��������� mycsubs.h ���� �������� � ��� �����������
// The copy of structures from mycsubs.h, because one file should never include
// both mycsubs.h and myparser.h and I don't want to move them to separate header
struct CFuncTable
{
    const char *Name;
    ExternalCFunction *Function;
    int ParamCount;    	// -1 - �� ��������� �� ���-��
};						// -1 - variable number of args

struct CPropTable
{
    const char *Name;
    ExternalCFunction *Get;
    ExternalCFunction *Set;
};
#endif

#include <poppack.h>

namespace Sasha
{

// ������ ��������� � ����� = 4 �������
// TAB size = 4

//////////////////////////////////////////////////////////////////////////////
//
//                                    ������
//                                    CLASSES
//
//////////////////////////////////////////////////////////////////////////////

// ��� �������, ���������� �� �������. ������� ������ ���� ��������� ����������
// ���������� ����������, � ���� �� ������/������ - �����. Error
// Typedef of a function callable from script. Function should check types of
// parameters itself and return "Error" if they are invalid.
class TParser;
typedef TVariable __cdecl ExternalFunction(TVariable *Params[], int ParamCount, TParser *Parser);

//
// �����, ����������� ���������� �������.
// Script parser class
//

class TParser
{
//
// ���������� ������
// Internal structures
//

// ���������� �� �������� ������ ���������� ��������� � ������ ������ � �������
// ����� ����������, ������� � ����� ���������� ����� ���� ����������� (����.)
// � ��������� aaa ����� ���� ����� aaa � ���������� aaa.
// Struct which contains info about one variable currently accessible from script
    struct Variable
    {
        TString Name;       // ��� ���.
        					// var name
// ���� ���������� �������� � ���������, �� ��� �����:
// "���_���������::���_����������-". ��� ���������� - ������ "::���"
//                               ^- ������ ��� ��������
// Local vars are named "func_name::var_name-". "-" is added every time function 
// is called resursevely. Global vars are "::name"
        TVariable Var;      // ��������
        					// value
        Variable (const TString &N) : Name(N) {Var.Type=TVariable::T_Unknown;}
        Variable (const Variable &V) : Name(V.Name), Var(V.Var) {}
    };

// ������ ���� ����������.
// List of all active variables
    TList <Variable> Variables;

// ���������� ��� �������� ���� �� ����� ������� ������������ � �������
// Struct describing one script function
    struct Function
    {
        TString Name;   // ��� �-� ��� ��� ���������� � �������
        				// func name
        int Position;   // �������� � Script � ������ �� ������ ���� �-�
        				// offset to function start in bytes in script
        TList<TString> Arguments;   // �������� ���� ����������
        							// argument names
        Function() {}
        Function(const Function &f) : Name(f.Name),
            Position(f.Position), Arguments(f.Arguments) {}
    };

// ������ ���� ������� � �������� �� ����������
// List of all functions & their params
    TList <Function> Functions;

// ���� � �����
// info about label 
    struct Label
    {
        TString Name;   // ���
        				// label name. 
        int Position;   // � ��� �������� ����� ���������� ScriptPos
                        // ����� goto �� ��� �����
                        // ScriptPos would take this value after GoTo this label
        int Depth;      // ����������� ����� � ����. ������������ ��� ���������
                        // goto ������ �� ����� � �������������� goto ������
                        // �����. �������� � �������:
                        // Depth of this label inside loops. Used to jump 
                        // outside loops. Example:
// sub aaa()
//   L1:        ; Depth==0
//   for a=1 to 10
//     L2:      ; Depth==1
//     for b=1 to 10
//       L3:    ; Depth==2
//       while c<10
//         L4:  ; Depth==3
//       wend
//     next
//   next
//
        Label() {}
        Label(const Label &L) : Name(L.Name), Position(L.Position), Depth(L.Depth) {}
    };

// ������ ����� � �� ���������
// List of all labels
    TList <Label> Labels;

// ���������� ��������� ������ �������� ���������� ��� �������� �� �������
// (��� �� ������� ��� ��������� � ���������)
// Internal. Temporary list of parameters passed to function.
    TList <TVariable> Arguments;

// ������ �������� Arguments ��� ��������
// Function arguments stack. Used on recursion.
    TStack <TList <TVariable> > ArgStk;

// ���� � ������� ������ ������������ �� �������
// �� ���� ����� ����� ���������� ��������� ����� ��������, ���� �����
// ��� �������� � � ����
// Info about class/functions accessible from script. There can be several such
// structs describing one class, though they could be joined into one.
    struct Class
    {
        TString Name;   // ��� ������ ��� "" ���� ������ ������ ���������� �-�
        				// Class name or "" if contains lict of functions
        				// (i.e. global functions)
        struct Metod    // ���� �� ����� ������
        				// describes one method/function
        {
            TString Name;               // ��� ������
            							// method/func name
            ExternalFunction *Func;     // ���� �� ���� ���������� �����
            ExternalCFunction *CFunc;   // �� ���� ���������
            							// one of these ptrs points to the 
            							// function. The second is 0

            int ParamCount;             // ������ ���������� ����������
            // ��� �������� � ���������. ��� ����� "this". ����==-1, ��
            // �� �����������
            // Exact number of parameters (without "this"). If -1 - not checked

            Metod() : Func(0), CFunc(0), ParamCount(0), Name("") {}
            Metod(const Metod&m) : Func(m.Func), CFunc(m.CFunc),
                Name(m.Name), ParamCount(m.ParamCount) {}
        };
        struct Property // ���� �� ����� property
        				// info about one property
        {
            TString Name;   // ��� (����� ��������� � ������)
                            // ������� 
                            // property name. The class may have both function 
                            // and a property with equal names
            ExternalFunction *Get;  // �-� ��� ��������� �������� property (��� 0)
            						// Get property function. Or 0
            ExternalFunction *Set;  // �-� ��� ��������� �������� (��� 0 ���� ������ ��� ������)
            						// Set function. Or 0
            ExternalCFunction *CGet;    // �� �� �����, ������ � ������ � �-���������
            ExternalCFunction *CSet;	// the same but if property is defined in C file (not C++)
            // � ������ �++ ������� (Get||Set)!=0 && CGet==CSet==0
            // ��� �-�������� Get==Set==0, (CGet||CSet)!=0
            Property() : Get(0), Set(0), CGet(0), CSet(0), Name("") {}
            Property(const Property &P) : Get(P.Get), Set(P.Set),
                CGet(P.CGet), CSet(P.CSet), Name(P.Name) {}
        };
        TList <Metod> Metods;
        TList <Property> Properties;
        Class() {}
        Class(const Class &c) : Name(c.Name), Metods(c.Metods), Properties(c.Properties) {}
    };

// ���������� �������� �������/�������/propert-��
// Internal list of classes, funcions, properties
    TList <Class> Classes;

// ������� ��� ������ While..Wend � Repeat..Until
// ���������� ������� ���� ����� ����������� ScriptPos ����� ���������� Wend/Until
// Stack of loops. Stores ScriptPos of the first line after While/Repeat
    TStack <int> Whiles;
    TStack <int> RUntils;

// ���� � ����� FOR
// FOR loop
    struct For
    {
        TVariable Var;      // ��� ���������� �� ������� ���� ����
        					// the loop variable
        TVariable EndValue; // �� �������� ��������
        					// its end value
        TVariable Step;     // ���
        					// step
        int Position;       // ���� ��������������� ScriptPos ��� ����. ��������
        					// ScriptPos of next iteration
        For() {}
        For(const For &f) : Var(f.Var), EndValue(f.EndValue), Step(f.Step),
            Position(f.Position) {}
    };

    TStack <For> Fors;

//
// ���������� �������
// Internal functions
//

// ����������: ������������� ������ ������ �� ������� ����
// sets the runtime error code 
    void SetError(int errCode, const char *Str=0, const char *Param=0);

//
// ���������� ����������
// Internal vars
//

// ��� ������� ������������� �������
// Current running function
    TString FunctionName;

public:
// �.�. ������ ������� ������ � namespace (������ � GCC ��� ��������)
// ���������� �������� ���������� ��� yylex ��� public � ������ friend
// Made this stuff public because C++Builder4's support for namespaces & friends
// is not so perfect as in GCC. So all these vars should be considered private

// ��������� �� ����� �������. ���������� ����������� � SetScript.
// Script text. Allocated in SetScript.
    char *Script;
// ������ ����� �������
// Script size in bytes
    int ScriptSize;
// ������� ������� �������������� yylex � *Script. ������������� � ������ �� ������
// Script.
// Current script position in bytes from "Script" beginning
    int ScriptPos;

// ����� true - yyparse ��������� ������������� ������ ��������� � ������������
// ������������ ������ ��� ������ ������� (�.�. ��� ����������)
// Flag used in yyparse to stop parsing SUB on return
    bool FinishedSub;

// ������ ������������ �������� ��������� ��� ������� �� ������, ���� �������
// ������ �� �������
// Function result or "Error"
    TVariable Result;

// ������ ��������������� �������� ��������������.
// ����� true - ������ ���������� ������ (���� ���������� ������������� ������)
// Set this to true to force parser to terminate script when it hangs
    bool Terminated;

// ������� yylex ������� ���� '\n' (��� ��������� ������ � ��������� �����)     
// Forces yylex to think that next char is '\n'. Used in loops/labels
    bool ReturnCR;

// ������� ������� ����������� ������ (� ��� �������� ��������������� Depth
// ��� �����)
// Current depth for labels
    int CycleDepth;

private:

// ������� � �������, ��� ���������� ������� ���� ���������� ������. (���� ������
// �������� ������� ������ ������� ������)
// Error position in script
    int ErrPos;

// ����� ��� �������� ��������� �� ������
// Buffer with error string
    char *TmpErrorString;

// ������� ���������� ����������, ������������ �������
// ������ ��� �� �� ��������. ����� �� ����� PreProcess ��� ��������������� ����� ����������
// � ���������� �������
// List of function parameters used during PreProcess
    TList<TString> Parameters;

public:
// ������, ��������������� ����� ������ (��. myparser.cpp)
// Error codes & strings
    static const char *ErrorStrings[];

    enum ErrorCodes {
        P_NoError,      // ����
        				// none
        P_Syntax,       // ����. ������
        				// syntax error
        P_VarDefined,   // ���������� ��� ����������
        				// var with the same name is already defined
        P_VarUndefined, // ���������� �� ����������
        				// undefined var
        P_InternalError,// ���������� ������
        				// internal parser bug
        P_BadResult,    // �������� ��������� ��� ������������ �������� ��� ������� ����
        				// invalid operation for this type
        P_LabelDefined, // ����� ��� ���������� � ������ ����� ���������
        				// label with the same name already defined
        P_BadLabelPlace,// ����������� ����� ��� ���������
        				// trying to define label outside function
        P_FunctionNotFound, // ������� �� �������
        					// function not found
        P_BadArgsCount, // ������������ ���-�� ���������� ��� ������ �-�
        				// invalid number of params to a function in script
        P_InvalidClass, // ������������� ������������ ���������� ��� ���. ������
        				// Trying to call class member function on a non-class var
        P_BadArray,     // ������������� ���������� � ���. �������
        				// trying to use normal var as an array
        P_RuntimeError, // ������� �-� ������� ������
        				// external function returned error
        P_GotoError,    // ������� �������� ������ �����
        				// trying to goto inside loop
        P_LabelNotFound,// ����� ����
        				// no such label
        P_InvalidParameters // ������. ���-�� ���������� ��� ������� �-�
        					// invalid number of params to external func
    };

// ��� ������ ������������� ������, 0 == ��� ������
// error code
    int Error;

// �������� ��������� ������ ��� SAFECALL.
// last error value on SAFECALL
    TVariable FatalError;

// �������� ������ � ��� ���� ��� ��� ����� ����������
// (������ ���� ��������� ���������, ���� ��������� �� TmpErrorString) 
// Full error description
    const char *ErrString;

//
// �����������
// Constructor
//

    TParser();
    ~TParser();

////////////////////////////////////////////////////////////////////////
//
// ������� ��� ������������� ����� ������
// Functions which should be used to control script execution
//
////////////////////////////////////////////////////////////////////////

// ���������� ������� � �������:
// Order of function calls to run script:
//  1. �������� ����� � ������
//  1. Load script somehow
//  2. ������� TParser
//  2. Create TParser instance
//  3. TParser.SetScript (��������� �� ����. ����, ������)
//  3. TParser.SetScript (ptr to loaded script, script size)
//  4. ���� ����: GetEnvironmentFrom(������)
//  5. Use GetEnvironmentFrom(parser) to set global var information from other instance
//     SetFunctions() - ���������� ������� �������
//                    - set external functions callable from script
//     SetClass() - ���������� ������� �����
//				  - set external classes
//     ....
//     SetGlobalVariable() - ���������� ��������� (����� ���� ��������)
//                         - add global variables and class instances
//     PreProcess() - ������ � ����� �������!!! PreProcess �������������
//      ���������� ����������. ��� ���� ����� ������������ ����� ������� �������
//                  - should be called after all these funcstions
//  5. TParser.Execute (����������� �������)
//                     (execute function from script)
//   ......
//  6. TParser.Execute (����������� �������)
//  ����� �������� Execute �������� ���-� �����������.
//  TParser instance remembers values of script global vars between calls to
//  Execute


// ���������� ����������� ���. ������� ����� ����������� ������
//  Script - � ��������� �������� ��� �����.
//  ScriptSize - � ������.
// Sets the script text. Functions makes a copy of a passed buffer.
    bool SetScript(const char *Script, int ScriptSize);

// ��������� ������ � ����������� ��� � ����������.
// �����. false == ������
// Preprocess script. Returns false on syntax error.
    bool PreProcess();

// ���������. �����. ��������� ������������ �������� ��� � ������ ���� ������ �� �������
// ���: T_String, �������� "Result is undefined". � ������ ������: T_Error, AsNumber
// ��� AsString - ��� ��� ����� ������
// �������� ���������� � �������� �������, ������������ ���������
// Runs specified function. Returns function result.
    TVariable Execute(const char *Func);

// ��������� � ��������� ���������� �������
// �������� ���������� � �������� ������� � ������� �����������.
// Runs the given function passing it parameters
    TVariable Execute(const char *Func, TVariable *Params[], int ParamCount);

// �������� ������������� ������. ����� ������ ���� �-� Execute()
// ����������� � �����-������ ������� (������ Parse error).
// Force script to terminate. May be used to stop hang scripts. This would
// simulate syntax error on next executing command.
    void Terminate() {Terminated=true;}

// ������� ����� ������ � �������. ������ �� 1
// Return the line with error. First line == 1
    int GetErrorLine();

// ���������� �������� ���� ���������� ����������, �������, ������� �������
// ������� ������� �� ������� �������. �������������� ���� ������� ������� � �.�.
// ���������
// Copy global variables from the other script instance.
    void GetEnvironmentFrom(const TParser &P);

// �������� � ���������� �������� ���������� ����������. ����� �������
// ��������������� �������� �������� True, False, PI...
// Sets the value of a predefined global var. TRUE and FALSE constants
// are set in this way
    void SetGlobalVariable(const char *Name, const TVariable &v);

// �������� �������� ���������� ���. (��������� �� ������������).
// ���� �� ������� �������� ��� T_Error 
// Return the global var value. If no such var - return type==T_Error
    TVariable GetGlobalVariable(const char *Name);

// �������� �������
// ���������� ����� ������� ����� ��������. � ������ ������ - ������
// ���� � ������� �������, ����������� �� ����� � ������ ������.
// ��� ����� ���������� ���������� TVariable � ����� T_Class, �
// Data.AsString="���_������", Data.AsNumber � Data.UserData - ���������
// ������ �������, ������������ ��� ������������� �������.
// ������� ��� �����, ��� �������� �����������
// ����������� � ����������, �.�. ��� ���������� ������� ��
// �������, ��������:
// VAR Variable = TMySuperClass() - ���������� ���������� ���� TMySuperClass
//     Variable.Create()  - ������� �����������
//     Variable.Destroy() - ����� �����������
// ����������� ������ ���������� TVariable � Type==T_Class
// � Data.AsString == ��� ������.
// ������� ���������� ������ ���������� �� TVariable. ��������� ������� -
// ���, ����� �������� ������������ ����� (����� this). ������������
// �� �� ����������!!!
// ��������� ������� ������� ������ ��������� ��� ���� ��������� = 0
//
    struct FuncTable
    {
        char *Name;
        ExternalFunction *Function;
        int ParamCount;    // -1 - �� ��������� �� ���-��
    };

// ���������� ������ � �������� ������� ��� ������ �� ����������������
// ���������.
// ��������� ���������� ������ �� �����. �������.
// Set the external functions
    void SetFunctions(const struct FuncTable *Table) {SetClass("",Table);}

// ���������� ������ � �������� ������� ������ ��� ������ �� ����������������
// ���������. ��� ������� ������ - ���� ����� SetClass
// Addes external class methods
    void SetClass(const char *ClassName, const struct FuncTable *Metods);

// �������� ���������. ������ ������� ���������� ��� ������ ��������, ������ -
// ��� ��� ������. ������ ��� ������ - ����� ��, ��� ��� ������ ������� ������.
// ��������� ��-� ������� ��� �������� � SetProperties - 0,0,0
// ������� Get ��������� ������������ ���������� �����,
// ������� Set ��������� ������ ���������� ��������������� ��������,
//  ������ - �����.
    struct PropTable
    {
        char *Name;
        ExternalFunction *Get;
        ExternalFunction *Set;
    };

// �������� ������� ��������� ��� ������� ������.
// Adds properties to external class
    void SetProperties(const char * Class, const struct PropTable * Prop);

// ������ �����, ����� �� �����������, ��� ����� ������� ������� ��� �������.
// ���� ClassName=="" - ������ ���������� �������
// Removes external class (both methods and properties)
    void RemoveClass(const char *ClassName);

// ���������� ��� ��������� #ParamNo ��� ������� Function, ������������
// � �������. ���������� 0, ���� ParamNo ������� ������� ��� ������� �� �������
// Returns the name of ParamNo paremeter to Function. Or zero 
    const char *GetParameterName(const char *Function, int ParamNo);

// ������� ��� ��������� ������� ������� ������� �� ��
// �� ������������ �������. ��� �� ��� ����� ������ � DLL ������� ��
// ������ ����� ������ - �� mycsubs.h
// Functions to set external class definition from DLL written on non-C++ language
    void SetCFunctions(const struct CFuncTable *Table) {SetCClass("",Table);}
    void SetCClass(const char *ClassName, const struct CFuncTable *Metods);
    void SetCProperties(const char * Class, const struct CPropTable * Prop);

// ���������� �������, ������� ����� ���������� ����� ��������� ������ ������
// �������.
// ������� ���������� TRUE==���������� ����������, FALSE - ��������
// ��������� ��������� Parser, ������� �� �����, � ������ �������
// ����� ����������� ���������.
//  ��� ���������� ������� - SetDebuggerFunction(0);
// Set the function called after each line of script is executed
    typedef bool DebuggerFunction(TParser &P, int Line);
    void SetDebuggerFunction(DebuggerFunction *F) {DbgFun=F;}

// �������� ������ ������
    void ClearError();

//////////////////////////////////////////////////////////////////////
//
// ���������� �������
// Internal functions used in yyparse(). Should be considered private
//
//////////////////////////////////////////////////////////////////////

public: // had to make this public to access from yyparse generated with BISON

// void DbgFun(TParser *THIS,int CurrentLine);
// �������, ������� ���������� ����� (��� ����� - �� �����) ���������� ������ CurrentLine
    DebuggerFunction *DbgFun;

// ���������� ����� ������ �� �������� ������� � Script[] 
// Returns the line number from position in script
    int GetLineFromPos(int Pos);

// ������, �����������, yylex ��� ���� ��������������� ������.
// ����� �� true - ���������� ���������� ������������.
    bool IsPreprocessing;

// �������, ������������ �� ����, ���������������� �� script.y    

// �� ������ � �����������

// ���������� �������� ��������� � ������ ���������� � Dest
// ������������� ������ - ���� Dest �� ������ ��� ���������� (T_Variable) ���
// ��� Source �� �������� ����������
    void SetVar (TVariable &Dest, const TVariable &Source);

// �������� �������� ����������
    TVariable GetVar (TVariable &Name);

// ���������� ����� ����������. � ������ ��������� ���������� � �������.
    void AddVar (TVariable &New);

// ���������� ���� � �������������� ������. ���������� �� ^����^ �-� ��� ������ 
    void SetParserError(char *Err);

// ������� �������������� ��� ���������� ���������� � �������    

// �������� ������� ���������� ���������� ��������� ��� �� ����������
    TString TmpFunction;    // ������������ �� ����� ������� ������� ������ FunctionName
    void BeginCountingParameters(TVariable &v);

// ��������� ����� �������� � �������
    void AddParameter(TVariable &New);

// ���������� �������. ��� �� EndCountingParameters()
    void AddSub (TVariable &New);

// ��������� � ������ ������� ��� ��������� ��������� ����������
// ������������� FunctionName
    void BeginSub (TVariable &Sub);

// ��������� ������� ����������. ������� ��������� ����������
    void EndSub();

// ��������� �����, ��������� ��� ���������. �� ��������, ���� ����� ���������
// ��������� ���. (�������� ���� ����� ����� ������ �����)
    void AddLabel(TVariable & Var);

// ���������� ������ � ���������    

// �������� ������    
    void AddArray(TVariable & Var,TVariable &Size);

// ������� ������
    void DeleteArray(TVariable &Var);

// �������� �������� ��-�� �������
    TVariable GetArray(TVariable & Var,TVariable &Pos);

// ���������� �������� ��-�� �������
    void SetArray(TVariable & Var,TVariable &Pos,TVariable & Val);

// ����� ��������� �� ������� (��������� ��� �������)    

// ������ ����������� ���������� ������ ���������
    void BeginCollectingParameters();
    void EndCollectingParameters();

// ��������� �������� ���������
    void StoreParameter(TVariable &Var);

// ���������� ����� ����������
    TVariable CallSub(TVariable &Sub);

// �����

// ������� �� �����
    void GoTo(const TVariable & V);

// ������� ���������� ����� ����������� �������    

// ���������������� RunTime �������
    void InitFunctions();

// ������
    void InitClasses();

// ����������    
    void InitGlobals();

// ��������� ������    

// ��������� ������� ������� ��� ������ Repeat ... Until()
    void PushRepeat();

// ������ �� ����� ��������� ����������� ������� Repeat ��� �������� �� ���
    void PopRepeat();

// ������� �� Repeat
    void GoToLastRepeat();

// ��������� ������� While ��� �������� �� ���
    void PushWhile();
    int LastWhilePos;  // ������� ����� ������ WHILE (������������ � yylex) 

// ��������� �� ��������� ������� While � ������ �� �� �����
    void PopWhile();

// ��������� ������� ����� TO � ���������� LastForVar
    TVariable LastForVar;
    void PushFor();

// ���������� LastForVar
    void SetLastForVar(const TVariable &Var);

// ���������� ��� ��� ���������� FOR
    void SetStepForLastFor(const TVariable &Var);

// ���������� TO ��� ���������� FOR
    void SetEndForLastFor(const TVariable &Var);

// true == ���� ����� �� FOR (���������� ��� NEXT)
    bool LastForIsEqual();

// ������� �� TO �� ���������� FOR (���������� ��� NEXT)
    void GoToLastFor();

// ������ ��������� FOR
    void PopFor();

// ��������� ������� ���������� FOR
    void SetForPos();

// ���������� ���������� ��������� �� ������ �� �������� ����������    
    void SetRuntimeError(TVariable &V);

// ������ ��-�� ��������� ��������:    
//    friend int ::yylex (TVariable *lval, void *This);
};


}; // namespace
#endif
