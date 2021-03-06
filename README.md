# Configurator
Hierarchical C++ struct human readable serializer/deserializer.  Originally made for configuration file reading/writing.

Compiles with C++11 compatible compilers. Tested with gcc 4.7-9.0 and Visual Studio 2012.

### Example usage
``` cpp
struct Config1 : public Configurator {
  int exampleIntValue;
  float exampleFloat;
  string exampleString;
  vector<int> exampleVector;
  map<string, int> exampleMap;
  int i,j,k;
  float x,y,z;

  CFG_HEADER(Config1)
  CFG_ENTRY_DEF(exampleIntValue, 12)  // entry with explicit default value
  CFG_ENTRY(exampleFloat)             // entry with auto default value for type
  CFG_ENTRY_DEF(exampleString, "initial value")
  CFG_ENTRY(exampleVector)
  CFG_ENTRY(exampleMap)
  CFG_MULTIENTRY6(i,j,k,x,y,z) // multiple entries with auto default value
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
  config2.writeToFile("file.txt");

  Config2 config2b;
  config2b.readFile("file.txt");
  cout << config2b.toString() << endl;
}
```

#### Corresponding serialization
```
exampleSub={
  exampleIntValue=1
  exampleFloat=0
  exampleString=a string
  exampleVector=[1,2,3]
  exampleMap=[]
}
anotherInt=2
```

#### Useful Configurator methods
``` cpp
class Configurator{
public:
  /// read and parse file / stream / string
  void readFile(const std::string& filename);
  void readStream(std::istream& stream);
  void readString(const std::string& str);
  void readString(const char* str, size_t size);
  void readString(const char* str);

  /// write contents of struct to file / stream / string
  void writeToFile(const std::string& filename);
  void writeToStream(std::ostream& stream,int indent=0);
  void writeToString(std::string& str);
  size_t writeToString(char* str, size_t maxSize); //returns bytes used
  std::string toString();

  /// equality
  bool operator==(Configurator& other);
  bool operator!=(Configurator& other);

  /// set varname = val
  void set(const std::string& varName, const std::string& val);
  /// set varname based on contents of stream
  void set(const std::string& varName, std::istream& stream);
  /// return name of current struct
  virtual std::string getStructName()=0;
  ///virtualized destructor for proper inheritance
  virtual ~Configurator(){}
};
```
#### Supported types
* All primitives
* Most std containers: string, vector, set, map, array, pair
* Nested supported types: vector of vector, vector of outfitted struct, etc...
* Any type with a operator>>() and a compatible operator<<()
