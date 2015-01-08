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

#ifndef myfuncsH
#define myfuncsH
//---------------------------------------------------------------------------

// ��� strncpy, ������ � ��������� ���� ������������� 0 ��� ������������
inline char *strncpyz(char *dest,const char *source,int maxlen)
{
  strncpy(dest,source,maxlen);
  dest[maxlen-1]=0;
  return dest;
}

// ����� � ����� s �������� �� ������� ���� width+1 ����� d � �������
// ������� width � ������������� �� ������ ����, ���� left==true
char *PrintNumber(char *s,double d,int width,bool left=false);

// ���������� � ����� s2 ����� d ������� width � ������������ ��
// ������ ����, ���� left==true � ������ ���. ������ decimals ����� ','
// ���������� �� ������ �� 3 �����
char *PrintMoney(char *s2,double d,int width,bool left=false,int decimals=2);

// �������� ����� �� ����������� PrintMoney ������
double GetMoney(const char s[]);

// �������������� ���� � ������ s
char *PrintDate(char *s,int d,int m,int y,int width);

// ��������� ���� �� ������
int GetDate(const char s[],int &d,int &m,int &y);

double EncodeTime(int Hour, int Min, int Sec, int MSec);
void DecodeTime(double Time,int &Hour, int &Min, int &Sec, int &MSec);
double EncodeDate(int Year,int  Month,int Day);
void DecodeDate(double Date, int &Year, int &Month, int &Day);
bool IsLeapYear(int Year);


#endif

