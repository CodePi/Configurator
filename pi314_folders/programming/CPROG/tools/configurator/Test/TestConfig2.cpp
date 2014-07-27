#include "../Configurator/configurator.h"
#include <iostream>

using namespace std;

struct Struct{
  int a,b,c;
};

struct StructConfig :public Struct, public Configurator {
  StructConfig& operator= (const Struct& o){
    a=o.a;
    b=o.b;
    c=o.c;
    return *this;
  }

  CFG_HEADER(StructConfig)
  CFG_ENTRY1(a)
  CFG_ENTRY1(b)
  CFG_ENTRY1(c)
  CFG_TAIL
};

int main(){
  Struct s1;
  s1.a = 1;
  s1.b = 2;
  s1.c = 3;

  StructConfig sc1;
  sc1 = s1;
  
  sc1.writeToStream(cout);
}
