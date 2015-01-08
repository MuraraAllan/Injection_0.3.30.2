/* ������������� ������� */

%{
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "myvar.h"
#include "myparser.h"

#define YYSTYPE TVariable
#define YYPARSE_PARAM This
#define YYLEX_PARAM This
#define yyerror(a) _yyerror(This,a)

// ��������� �������� ������� �����
// ��� �������� ����� ��� ��������
#define YYINITDEPTH 15

#ifdef THIS
#undef THIS
#endif

#define THIS ((TParser*)This)

void _yyerror(void *, char*);
//int yylex (YYSTYPE *lval, void *This);
int yyparse (void *);
void SkipToEndif (void *This);
void SkipToElseOrEndif (void *This);
void SkipToWend (void *This);
void SkipToNext (void *This);

%}

/* BISON Declarations */

// ������� reentarant
%pure_parser

// ����������� ����������� token

// ������ ����� ��� ������ ��������� ���������
%token NUM

// ����� "VAR"
%token VAR

// ����� "SUB"
%token SUB

// ����� "ENDSUB"
%token END_SUB

// ����� "IF"
%token IF

// ����� "THEN"
%token THEN

// ����� "ELSE"
%token ELSE

// ����� "ENDIF"
%token END_IF

// ����� "GOTO"
%token GOTO

// ����� "RETURN"
%token RETURN

// ����� "DIM"
%token DIM

// ����� "REPEAT"
%token REPEAT

// ����� "UNTIL"
%token UNTIL

// ����� "WHILE"
%token WHILE

// ����� "WEND"
%token WEND

// ����� "FOR"
%token FOR

// ����� "TO"
%token TO

// ����� "STEP"
%token STEP

// ����� "NEXT"
%token NEXT

// ����� "SAFECALL"
%token SAFECALL

// ����� "FATALERROR"
%token FATALERROR

// ����� "RETURN ERROR"
%token RETURNERROR

// �������� ����������/�������
%token IDENTIFIER

// �������� ���������� � ������� ��������
%token OR
%token AND

%left OR
%left AND
%left NOT
%left '<' '>' '='
%left '-' '+' 
%left '*' '/' 
%left NEG

/* Grammar follows */
%%
input:    /* empty string */
        | input line
;

// ������ ���: �����, �����, ����� � ������� ��� ������ �������
line:     '\n'
        | label '\n'
        | label commands '\n'
        | commands '\n'
;

// � ������ ����� - �������� ��. $1 ��������������� � yylex � TVariable(��� �����)
label:    IDENTIFIER ':'        { THIS->AddLabel($1);
                                  if(THIS->Error)
                                    YYABORT;
                                }
;

commands:
// ���������� �����������
          VAR list_of_variables

// �������
        | DIM list_of_arrays

// For .. To .. Step ... Next
// ����� ����� ��� ���� ������ � ����� � for_closure
// ��� ��� �������� ����� ��������������� ��� � �������� ��������.
// ���������� ����� ���������� ��������� � for_closure
        | for_closure TO exp    { if(!THIS->IsPreprocessing)
                                  {
                                      THIS->SetStepForLastFor(1.0);
                                      THIS->SetEndForLastFor($3); 
                                      THIS->SetForPos();
                                  }
                                  THIS->CycleDepth++;
                                }
        | for_closure TO exp STEP exp
                                { if(!THIS->IsPreprocessing)
                                  {
                                      THIS->SetStepForLastFor($5);
                                      THIS->SetEndForLastFor($3);
                                      THIS->SetForPos();
                                  }
                                  THIS->CycleDepth++;
                                }

// �������� ������� ������ �� FOR:
// ��� ����� ������������ ��� IsPreprocessing
        | NEXT                  { if(!THIS->IsPreprocessing)
                                  {
                                    if(!THIS->LastForIsEqual())  // ���� �� ����� �� �����
                                      THIS->GoToLastFor();       // �� ������� �� ������� ����� FOR
                                    else
                                    {
                                      THIS->CycleDepth--;        // ����� ������ ���� FOR
                                      THIS->PopFor();
                                    }
                                  } else
                                      THIS->CycleDepth--;
                                }
// While ... Wend
        | WHILE exp             { if(THIS->IsPreprocessing)
                                  {
                                    THIS->CycleDepth++;
                                  }
                                  else if(!$2.IsTrue()) 	// ���� ������� �����,
                                  {
                                    THIS->CycleDepth++;     // ��������� ������� ����������� ������ (��� �����)
                                    SkipToWend(This);       // ���������� �� WEND
                                    THIS->CycleDepth--;
                                  }
                                  else
                                  {
                                    THIS->PushWhile();      // �� � ���� ������� - ��������� ������� ����� WHILE
                                    THIS->CycleDepth++;
                                  }
                                }

        | WEND                  { if(!THIS->IsPreprocessing) // ����� ������: ������� �� ������� 
                                    THIS->PopWhile();        // ����� ��������������� WHILE - � ��� ����������
                                  THIS->CycleDepth--;
                                }

// ���� REPEAT ... UNTIL
        | REPEAT                { THIS->PushRepeat(); THIS->CycleDepth++;  }


        | UNTIL exp             { if(!$2.IsTrue()&&!THIS->IsPreprocessing)
                                  {
                                    THIS->GoToLastRepeat(); // ���� ������� ����� �� ������� �� REPEAT
                                  }
                                  else
                                  {
                                    THIS->PopRepeat();      // ���� ������� - ������ � ��������� REPEAT� 
                                    THIS->CycleDepth--;
                                  }
                                }
// �������
        | IF exp THEN           { if(!THIS->IsPreprocessing)
                                    if(!($2.IsTrue()))
                                      SkipToElseOrEndif(This);
                                   // ���� ������� ����� - ������� �� ELSE, � ���� ELSE ����, �� �� ENDIF
                                   // �� � ���� ������� - ������ �� ������
                                }

// ����� ELSE ��� ���������� yylex ����������� ������ � ������ ���� ������� � IF ���� �������
// (�� ���� ������ ������� ��� ��������� ���� � ������� SkipToElseOrEndif)
        | ELSE                  { if(!THIS->IsPreprocessing)
                                    SkipToEndif(This);  // ���������� "������" �����
                                }

// � ��������, ENDIF ����� ������ ��� ������� SkipToEndif/SkipToElseOrEndif
        | END_IF

// Goto
        | GOTO IDENTIFIER       { THIS->GoTo($2); }

// ����� ������� (��������� ����)
        | safe_function_call    // ����� �������, ������� �������� ������ ������
        | function_call         // ����� �������. ���� ��� ������ ������ - ������ �����������

// ���������� �������
        | SUB IDENTIFIER        { THIS->BeginCountingParameters($2);}
                                // ��� ������ ����������� ����� "SUB ���_��."
                                // ������ ������� �� ��������� (��� ���������� �-�)
            '(' sub_parameters ')'
                                { THIS->AddSub($2);  // ���� ��� ����������� �����
                                  if(THIS->Error)    // ���������� ������� sub_parameters
                                    YYABORT;
                                  THIS->BeginSub($2);// ������� �������, ��� ����� � ���������
                                  if(THIS->Error)
                                    YYABORT;
                                }

        | END_SUB               { THIS->EndSub();    // ����� �� ���������
                                  if(THIS->FinishedSub)
                                    YYACCEPT;        // ��������� yyparse, ���� �� ��� ������ �� TParser::Execute
                                }

// ������� �� ��������� (��������� ������������� �������� ���� � ���_stmt)
        | RETURN ret_stmt       { if(!THIS->IsPreprocessing)
                                    YYACCEPT;  // ��������� yyparse
                                }

        | RETURNERROR err_stmt  { if(!THIS->IsPreprocessing)
                                    YYACCEPT;  // ��������� yyparse
                                }

// ���������� �������� ����������
        | IDENTIFIER '=' exp    { THIS->SetVar ($1, $3);
                                  if(THIS->Error)
                                    YYABORT;
                                }
// ... �������
        | IDENTIFIER '[' exp ']' '=' exp
                                { THIS->SetArray ($1, $3, $6);
                                  if(THIS->Error)
                                    YYABORT;
                                }
;

// ������� FOR
for_closure: FOR for_exp        { if(!THIS->IsPreprocessing)
                                    THIS->PushFor();    } // ��� �� ���������� ������� � ������� ������ FOR
;

// �� � ��� �� ������� ����������...
for_exp:  IDENTIFIER '=' exp    { THIS->SetVar ($1, $3);
                                  if(THIS->Error)
                                    YYABORT;
                                  THIS->SetLastForVar($1);
                                }
        | VAR IDENTIFIER '=' exp
                                { THIS->AddVar ($2);
                                  if(THIS->Error)
                                    YYABORT;
                                  THIS->SetVar ($2, $4);
                                  if(THIS->Error)
                                    YYABORT;
                                  THIS->SetLastForVar($2);
                                }
;

// ��� ��������������� ������������ �������� 
ret_stmt: /* empty */
        | exp                   { $$ = THIS->Result = $1; 
                                  if($1.ErrorCode()&&!THIS->IsPreprocessing)
                                    THIS->SetRuntimeError($1);
                                }
;

// � ��� ������������ ������ �� ������
err_stmt: /* empty */
        | exp                   {
                                    if($1.Type==TVariable::T_String)
                                        THIS->Result = ::Error($1.Data.AsString.c_str());
                                    else
                                        THIS->Result = ::Error($1.Data.AsNumber);
                                }
;

// ������ ���������� ��������� ��� ����������
sub_parameters:
          /* empty */
        | parameter_list
;

// ��� ������
call_parameters:
          /* empty */
        | call_parameter_list
;

// �������� ���������� � ����������� �� ���� ��� ����������
parameter_list:
// ���������� �������� ���������� ������� ��� ����������
          IDENTIFIER            { THIS->AddParameter($1);
                                  if(THIS->Error)
                                    YYABORT;
                                }
        | parameter_list ','
          IDENTIFIER            { THIS->AddParameter($3);
                                  if(THIS->Error)
                                    YYABORT;
                                }
;

// �������� ���������� � ����������� �� �������� ��� ������ �-�
call_parameter_list:
// ���������� �������� ���������� ������� ��� ����������
          exp                   { THIS->StoreParameter($1);
                                  if(THIS->Error)
                                    YYABORT;
                                }
        | call_parameter_list ','
          exp                   { THIS->StoreParameter($3);
                                  if(THIS->Error)
                                    YYABORT;
                                }
;


// ��� ��� VAR - ���������� ������ ���������� � ���������� � ����������� �� ��������
list_of_variables:
          IDENTIFIER            { THIS->AddVar ($1);
                                  if(THIS->Error)
                                    YYABORT;
                                }
        | list_of_variables ',' IDENTIFIER
                                { THIS->AddVar ($3);
                                  if(THIS->Error)
                                    YYABORT;
                                }
        | assignment_identifier
        | list_of_variables ',' assignment_identifier
;

// ��� DIM
list_of_arrays:
          IDENTIFIER '[' exp ']'{ THIS->AddArray ($1, $3);
                                  if(THIS->Error)
                                    YYABORT;
                                }
        | list_of_arrays ',' IDENTIFIER '[' exp ']'
                                { THIS->AddArray ($3, $5);
                                  if(THIS->Error)
                                    YYABORT;
                                }
;

// ���������� �������� ���������� ��� �� ����������
assignment_identifier:
          IDENTIFIER '=' exp    { THIS->AddVar ($1);
                                  if(THIS->Error)
                                    YYABORT;
                                  THIS->SetVar ($1, $3);
                                  if(THIS->Error)
                                    YYABORT;
                                }       
;

// ����� �������
function_call:
          IDENTIFIER '('        { THIS->BeginCollectingParameters();}
                                // ��� ������ ����������� '(', ���������� ���� 
                                // ����������
           call_parameters ')'  { $$ = THIS->CallSub($1); // ��� ������� ')' ���� ����� ���������
                                  THIS->EndCollectingParameters();
                                  if(THIS->Error)
                                    YYABORT;
                                  if($$.ErrorCode())
                                    THIS->SetRuntimeError($$);
                                }
;

// Safe ����� ������� (��� ������� �����, ������ ��� �����. �� ������)
safe_function_call: SAFECALL
          IDENTIFIER '('        { THIS->BeginCollectingParameters();}
           call_parameters ')'  { TVariable tmp = THIS->CallSub($2);
                                  THIS->EndCollectingParameters();
                                  if(THIS->Error)
                                    YYABORT;
                                  if(tmp.ErrorCode())   // ��� ������������ ��������
                                  {                     // ������ (���� ����)
                                    THIS->FatalError=tmp;
                                    THIS->FatalError.Type=tmp.Data.AsString.Length()?TVariable::T_String :
                                        TVariable::T_Number;
                                    $$=THIS->FatalError;
                                  } else
                                  {
                                    THIS->FatalError=0.0;
                                    $$=tmp;
                                  }
                                }
;

// � ��� ������ ��������������� ���������. �� ������ ������������ �� ���� �� ������
exp:      NUM                   { $$ = $1;         }
        | IDENTIFIER            { $$ = THIS->GetVar($1);
                                  if(THIS->Error)
                                    YYABORT;
                                }
        | IDENTIFIER '[' exp ']'{ $$ = THIS->GetArray($1,$3);
                                  if(THIS->Error)
                                    YYABORT;
                                }
        | function_call         { $$ = $1;         }
        | safe_function_call    { $$ = $1;         }
        | FATALERROR            { $$ = THIS->FatalError;    }
        | exp '+' exp           { $$ = $1 + $3;    }
        | exp '-' exp           { $$ = $1 - $3;    }
        | exp '*' exp           { $$ = $1 * $3;    }
        | exp '/' exp           { $$ = $1 / $3;    }
        | exp AND exp           { $$ = $1.IsTrue() && $3.IsTrue(); }
        | exp OR  exp           { $$ = $1.IsTrue() || $3.IsTrue(); }
        | '-' exp  %prec NEG    { $$ = -$2;        }
        | NOT exp  %prec NOT    { $$ = !$2.IsTrue(); }
        | '(' exp ')'           { $$ = $2;         }
        | exp '=' '=' exp       { $$ = $1 == $4;   }
        | exp '<' '=' exp       { $$ = ($1 < $4) || ($1 == $4);   }
        | exp '>' '=' exp       { $$ = !($1 < $4); }
        | exp '<' exp           { $$ = $1 < $3;    }
        | exp '<' '>' exp       { $$ = !($1 == $4);}
        | exp '>' exp           { $$ = !($1 < $3) && !($1 == $3); }
;
%%


#include <stdio.h>

void _yyerror (void *This, char *s)  /* Called by yyparse on error */
{
    THIS->SetParserError(s);
}

