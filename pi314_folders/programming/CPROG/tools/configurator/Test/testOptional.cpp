#include "../Configurator/Optional.h"
#include <vector>
#include <stdio.h>

using namespace std;

vector<int> rvalue(int**p){
  vector<int> v = {1,2,3};
  *p = v.data();
  return v;

}

const char* pf(bool test){
  if(test==true) return "pass";
  else return "fail";
}

int main(){

  // test1
  vector<int> v={1,2,3};
  int* orig = v.data();
  Optional<vector<int>> ov(move(v));
  int* curr = ov->data();
  printf("construct by move contents:\t%x\t%x\t%s\n", orig, curr, pf(orig==curr));

  // test2
  Optional<vector<int>> ov2(rvalue(&orig));
  curr = ov2->data();
  printf("construct by rvalue:\t\t%x\t%x\t%s\n", orig, curr, pf(orig==curr));

  // test3
  vector<int> v3={1,2,3};
  orig = v3.data();
  Optional<vector<int>> ov3(v3);
  curr = ov3->data();
  printf("construct by copy:\t\t%x\t%x\t%s\n", orig, curr, pf(orig!=curr));
  
  // test4
  orig = ov3->data();
  Optional<vector<int>> ov4(ov3);
  curr = ov4->data();
  printf("construct by copy wrapper:\t%x\t%x\t%s\n", orig, curr, pf(orig!=curr));

  // test5
  Optional<vector<int>> ov5(Optional<vector<int>>(rvalue(&orig)));
  curr = ov5->data();
  printf("construct by double rvalue:\t%x\t%x\t%s\n", orig, curr, pf(orig==curr));
  
  // test6
  Optional<vector<int>> ov6;
  ov6 = rvalue(&orig);
  curr = ov6->data();
  printf("assignment rvalue:\t\t%x\t%x\t%s\n", orig, curr, pf(orig==curr));

  // test7 
  Optional<vector<int>> ov7;
  ov7 = Optional<vector<int>>(rvalue(&orig));
  curr = ov7->data();
  printf("assignment double rvalue:\t%x\t%x\t%s\n", orig, curr, pf(orig==curr));
  
  // test8
  const vector<int> v8 = {1,2,3};
  Optional<vector<int>> ov8(v8);
  orig = (int*)v8.data();
  curr = ov8->data();
  printf("construct by const:\t\t%x\t%x\t%s\n", orig, curr, pf(orig!=curr));

}
