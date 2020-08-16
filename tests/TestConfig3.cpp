#include "../Configurator/configurator.h"

#include <map>
#include <vector>
#include <string>

using namespace std;
using namespace codepi;

struct Config1 : public Configurator {
  int exampleIntValue;
  float exampleFloat;
  string exampleString;
  vector<int> exampleVector;
  map<string, int> exampleMap;

  CFG_HEADER(Config1)
  CFG_ENTRY_DEF(exampleIntValue, 12)
  CFG_ENTRY(exampleFloat)
  CFG_ENTRY_DEF(exampleString, "initial value")
  CFG_ENTRY(exampleVector)
  CFG_ENTRY(exampleMap)
  CFG_TAIL
};

struct Config2 : public Configurator {
  Config1 exampleSub;
  int anotherInt;

  CFG_HEADER(Config2)
  CFG_ENTRY(exampleSub)
  CFG_ENTRY(anotherInt)
  CFG_TAIL
};

int main(){
  Config2 config2;

  config2.exampleSub.exampleIntValue = 1;
  config2.exampleSub.exampleString = "a string";
  config2.exampleSub.exampleVector = { 1, 2, 3 };
  config2.anotherInt = 2;
  cout << config2.toString() << endl;
  config2.writeToFile("file3.txt");

  Config2 config2b;
  config2b.readFile("file3.txt");
  cout << config2b.toString() << endl;

}
