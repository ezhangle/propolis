#include <iostream>
#include <cstdio>
#include "encoding.h"

/*  0 Test pattern, do not decode. Not all test patterns use metadata.
 *  1 Unicode packing mode 1
 *  2 Unicode packing mode 2
 *  3 unassigned, possibly octal
 *  4 unassigned, possibly hex
 *  5 letters @ to _ (i.e. no encoding)
 *  6 base-64 code?
 *  7 ASCII
 *  8 bytes
 *  9 reserved for multiple encodings
 * 10 decimal, with 14 punctuation marks
 * 11 decimal in different scripts
 * 12-31 unassigned
 * 
 * The two Unicode packings differ only in the encoding of half-fronted Unicode
 * points, so run the text (in UTF-8) through the half-fronting algorithm
 * to get a wide string before encoding.
 * 
 * The various encoding functions return encoded text if the text can be so encoded,
 * else a null string, along with an integer denoting the encoding.
 */

using namespace std;
const string chr24("0123456789 #()*+,-./<=>@");
const string digits("0123456789");

wstring fromutf8(string utf8)
{
  wstring wide;
  string seq;
  wchar_t wch;
  int ch,clen,wclen,i;
  bool err=false;
  while (utf8.length())
  {
    ch=utf8[0]&0xff;
    clen=(ch>0x7f)+(ch>0xbf)+(ch>0xdf)+(ch>0xef)+(ch>0xf7)+(ch>0xfb)+(ch>0xfd);
    if (clen<2)
      clen=1-clen;
    switch (clen)
    {
      case 0: // sequence starts with a continuation byte
      case 7: // sequence starts with 0xfe or 0xff
        wide.clear();
	utf8.clear();
	break;
      case 1:
	wch=utf8[0];
	wide+=wch;
	utf8.erase(0,1);
	break;
      default:
	seq=utf8.substr(0,clen);
	utf8.erase(0,clen);
	seq[0]&=(1<<(8-clen))-1;
	for (wch=i=0;i<clen;i++)
	{
	  wch=(wch<<6)|(seq[i]&0x7f);
	  if (i>0 && (seq[i]&0xc0)!=0x80)
	    err=true;
	}
	wide+=wch;
    }
    wclen=(wch>-1)+(wch>0x7f)+(wch>0x7ff)+(wch>0xffff)+(wch>0x1fffff)+(wch>0x3ffffff);
    if (err || wclen!=clen)
    {
      wide.clear();
      utf8.clear();
    }
  }
  return wide;
}

string toutf8(wstring wide)
{
  string utf8,seq;
  wchar_t wch;
  char ch,clen,wclen,i;
  while (wide.length())
  {
    wch=wide[0];
    wclen=(wch>-1)+(wch>0x7f)+(wch>0x7ff)+(wch>0xffff)+(wch>0x1fffff)+(wch>0x3ffffff);
    switch (wclen)
    {
      case 0: // character is negative
        wide.clear();
	utf8.clear();
	break;
      case 1:
	utf8+=wch;
	wide.erase(0,1);
	break;
      default:
	seq="";
	wide.erase(0,1);
	for (i=0;i<wclen;i++)
	{
	  ch=(wch&0x3f)+0x80;
	  if (i==wclen-1)
	    ch=wch|-(1<<(8-wclen));
	  seq=ch+seq;
	  wch>>=6;
	}
	utf8+=seq;
    }
  }
  return utf8;
}

encoded encode32(string text)
{
  encoded code;
  while (text.length())
  {
    if (text[0]>='@' && text[0]<='_')
    {
      code.codestring+=text[0];
      text.erase(0,1);
    }
    else
      code.codestring=text="";
  }
  code.encoding=5;
  return code;
}

string decode32(string text)
{
  return text;
}

encoded encodedecimal(string text)
{
  encoded code;
  string dig3,let2;
  size_t charcode;
  while (text.length())
  {
    dig3=text.substr(0,3);
    if (dig3.length()==3 && dig3.find_first_not_of(digits)==string::npos)
      charcode=100*dig3[0]+10*dig3[1]+dig3[2]-111*'0';
    else
    {
      charcode=chr24.find(dig3[0]);
      if (charcode!=string::npos)
	charcode+=1000;
    }
    if (charcode==string::npos)
      code.codestring=text="";
    else
    {
      let2=charcode/32+'@';
      let2+=charcode%32+'@';
      code.codestring+=let2;
      text.erase(0,(charcode<1000)?3:1);
    }
  }
  code.encoding=10;
  return code;
}

string decodedecimal(string text)
{
  string plain;
  int charcode;
  while (text.length())
  {
    if (text.length()==1)
      plain="";
    else
    {
      charcode=text[0]*32+text[1]-33*'@';
      if (charcode<1000)
      {
	plain+=charcode/100+'0';
	charcode%=100;
	plain+=charcode/10+'0';
	charcode%=10;
	plain+=charcode+'0';
      }
      else
	plain+=chr24[charcode-1000];
    }
    text.erase(0,2);
  }
  return plain;
}

encoded encodeascii(string text)
{
  encoded code;
  int charcode=0,nbits=0;
  while (text.length())
  {
    charcode<<=7;
    charcode+=text[0]&0x7f;
    if (text[0]&0x80)
    {
      text=code.codestring="";
      nbits=-7;
      charcode=0;
    }
    text.erase(0,1);
    nbits+=7;
    while (nbits>=5)
    {
      code.codestring+=(charcode>>(nbits-5))+'@';
      nbits-=5;
      charcode&=(1<<nbits)-1;
    }
  }
  if (nbits)
    code.codestring+=(charcode<<(5-nbits))+'@';
  code.encoding=7;
  return code;
}

string decodeascii(string text)
{
  string plain;
  int charcode=0,nbits=0;
  while (text.length())
  {
    charcode<<=5;
    charcode+=text[0]&0x1f;
    text.erase(0,1);
    nbits+=5;
    while (nbits>=7)
    {
      plain+=charcode>>(nbits-7);
      nbits-=7;
      charcode&=(1<<nbits)-1;
    }
  }
  return plain;
}

encoded encodebyte(string text)
{
  encoded code;
  int charcode=0,nbits=0;
  while (text.length())
  {
    charcode<<=8;
    charcode+=text[0]&0xff;
    text.erase(0,1);
    nbits+=8;
    while (nbits>=5)
    {
      code.codestring+=(charcode>>(nbits-5))+'@';
      nbits-=5;
      charcode&=(1<<nbits)-1;
    }
  }
  if (nbits)
    code.codestring+=(charcode<<(5-nbits))+'@';
  code.encoding=8;
  return code;
}

string decodebyte(string text)
{
  string plain;
  int charcode=0,nbits=0;
  while (text.length())
  {
    charcode<<=5;
    charcode+=text[0]&0x1f;
    text.erase(0,1);
    nbits+=5;
    while (nbits>=8)
    {
      plain+=charcode>>(nbits-8);
      nbits-=8;
      charcode&=(1<<nbits)-1;
    }
  }
  return plain;
}

void sort1(vector<encoded> &list)
{
  int i;
  i=list.size()-2;
  while (i>=0 && list[i].codestring.length()>list[i+1].codestring.length())
  {
    swap(list[i],list[i+1]);
    i--;
  }
}

vector<encoded> encodedlist(string text)
{
  vector<encoded> list;
  list.push_back(encode32(text));
  sort1(list);
  list.push_back(encodeascii(text));
  sort1(list);
  list.push_back(encodebyte(text));
  sort1(list);
  list.push_back(encodedecimal(text));
  sort1(list);
  return list;
}

string decode(encoded ciphertext)
{
  wstring uniplain;
  string plain;
  switch (ciphertext.encoding)
  {
    case 5:
      plain=decode32(ciphertext.codestring);
      break;
    case 7:
      plain=decodeascii(ciphertext.codestring);
      break;
    case 8:
      plain=decodebyte(ciphertext.codestring);
      break;
    case 10:
      plain=decodedecimal(ciphertext.codestring);
      break;
    default:
      cerr<<"unimplemented code "<<ciphertext.encoding<<endl;
  }
  return plain;
}

void dumpenc(vector<encoded> encodedlist)
{
  int i;
  for (i=0;i<encodedlist.size();i++)
    cout<<encodedlist[i].encoding<<" "<<encodedlist[i].codestring<<endl;
}

void testenc1(string text)
{
  vector<encoded> list;
  int i;
  cout<<"Original: "<<text<<endl;
  list=encodedlist(text);
  for (i=0;i<list.size();i++)
    cout<<list[i].encoding<<" "<<list[i].codestring<<" "<<decode(list[i])<<endl;
}

void testenc()
{
  wstring wide;
  int i;
  wide=fromutf8("Πρόπολις");
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("慮畫搠楥樠湵潧瘠污楳吠敨敳愠敲渠瑯䌠楨敮敳眠牯獤"); // "naku dei jungo valsi These are not Chinese words" converted from utf-16le
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("\300\201"); // overlong encoding
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("\302\302"); // invalid sequence
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("\345\202"); // truncated sequence
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  wide=fromutf8("örült az őrült");
  for (i=0;i<wide.length();i++)
    printf("%x ",wide[i]);
  cout<<endl<<toutf8(wide)<<endl;
  testenc1("PROPOLIS");
  testenc1("propolis");
  testenc1("Πρόπολις");
  testenc1("0588235294117647");
}
