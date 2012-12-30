/* Hexagonal vector (Eisenstein or Euler integers) and array of bytes subscripted by hexagonal vector
 * The largest hvec used in this program has a norm of about
 * than 95²*12, or 108300.
 */
#ifndef HVEC_H
#define HVEC_H

#include <cmath>
#include <cstdlib>
#include <map>
using namespace std;

#define PAGERAD 5
#define PAGESIZE (PAGERAD*(PAGERAD+1)*3+1)
#define sqr(a) ((a)*(a))

class hvec
{private:
 int x,y; // x is the real part, y is at 120°
 static int numx,numy,denx,deny,quox,quoy,remx,remy;
 void divmod(hvec b);
 public:
 hvec()
 {x=y=0;}
 hvec(int xa)
 {x=xa;
  y=0;
  }
 hvec(int xa,int ya)
 {x=xa;
  y=ya;
  }
 hvec operator+(hvec b);
 hvec operator-();
 hvec operator-(hvec b);
 hvec operator*(hvec b);
 hvec& operator*=(hvec b);
 hvec& operator+=(hvec b);
 hvec operator/(hvec b);
 hvec operator%(hvec b);
 bool operator==(hvec b);
 bool operator!=(hvec b);
 friend bool operator<(const hvec a,const hvec b); // only for the map
 unsigned long norm();
 int pageinx();
 int letterinx();
 int getx()
 {return x;
  }
 int gety()
 {return y;
  }
 void inc(int n);
 bool cont(int n);
 };

hvec start(int n);
extern const hvec LETTERMOD,PAGEMOD;
extern int debughvec;

template <typename T> class harray
{map<hvec,T *> index;
 public:
 T& operator[](hvec i);
 void clear();
 };

template <typename T> T& harray<T>::operator[](hvec i)
{hvec q,r;
 q=i/PAGEMOD;
 r=i%PAGEMOD;
 if (!index[q])
    index[q]=(T*)calloc(PAGESIZE,sizeof(T));
 return index[q][r.pageinx()];
 }

template <typename T> void harray<T>::clear()
{typename map<hvec,T *>::iterator i;
 for (i=index.start();i!=index.end();i++)
     {free(i->second);
      i->second=NULL;
      }
 }

extern harray<char> hletters,hbits;
#endif
