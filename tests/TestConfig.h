#pragma once

#include "../Configurator/configurator.h"

struct SubConfig2:public codepi::Configurator{
  int k;

  CFG_HEADER(SubConfig2)
  CFG_ENTRY_DEF(k,9)
  CFG_TAIL
};

struct SubConfig1:public SubConfig2{
  int i,j;
  
  CFG_HEADER(SubConfig1)
  CFG_ENTRY_DEF(i,7)
  CFG_ENTRY(j)
  CFG_PARENT(SubConfig2)
  CFG_TAIL
};

struct TestConfig:public codepi::Configurator{

  int jjj;
  std::string n;
  std::vector<int> k;
  std::array<int,10> arr;
  std::set<int> intSet;
  std::pair<int, std::string> pair;
  std::pair<std::string, float> pair2;
  std::map<std::string, int> map;
  SubConfig1 s;
  std::vector<SubConfig1> t;
  std::vector<std::string> strList;
  SubConfig1 u;
  bool b;
  codepi::Optional<int> opt1, opt2, opt3;
  codepi::Optional< std::vector<int> > optvec;

  CFG_HEADER(TestConfig)
  CFG_ENTRY_DEF(jjj,12)
  CFG_ENTRY_DEF(n,"hello")
  CFG_MULTIENTRY10(k,arr,intSet,pair,pair2,map,s,t,strList,u)
  CFG_MULTIENTRY5(b,opt1,opt2,opt3,optvec);
  CFG_TAIL
};
