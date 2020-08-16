// Copyright (C) 2011 Paul Ilardi (http://github.com/CodePi)
// 
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, unconditionally.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
// DEALINGS IN THE SOFTWARE.
// 
// Tested with gcc 4.1, gcc 4.3, vs2010.

#include "configurator.h"
#include <fstream>
#include <string.h>

#ifdef __GNUC__
#include <strings.h>
#endif

#define STR_DELIM ",#}]\t\r\n"

using namespace std;

namespace codepi {

//////////////////////////////////////////////////////////////////////
// StreambufWrapper: Helper class for wrapping a char* without copying
// This allows use of a c-style array as a streambuf
// The streambuf can then be used in a istream or ostream

class StreambufWrapper : public streambuf {
public:
  StreambufWrapper(char* s, std::size_t n) {
    setg(s, s, s + n);  // set up as input buffer
    setp(s, s + n);     // set up as output buffer
  }
  size_t getSizeUsed(){
    return pptr() - pbase();
  }
};

/////////////////////////////////////////////////////
// stripSpaces: Helper function

static string stripSpaces(const string& in){
  size_t a=in.find_first_not_of(" \t\r\n"); //find first non-space
  size_t b=in.find_last_not_of(" \t\r\n"); //find last non-space
  if(a==string::npos) return ""; //all white spaces
  return in.substr(a,b-a+1); //get rid of leading or trailing spaces
}

/////////////////////////////////////////////////////
// Configurator methods

void Configurator::readFile(const string& filename){
  // open file as stream and parse
  ifstream ifs(filename.c_str());
  if(!ifs){
    throwError("Configurator ("+getStructName()+") error, file not found: "+filename);
  }
  readStream(ifs);
}

void Configurator::readStream(istream& stream){
  string key;
  //find first non-white space, skipping '{'
  while(isspace(stream.peek())||stream.peek()=='{') stream.ignore();
  while(stream.good()){
    // push to next element, removing comments
    while(isspace(stream.peek())||stream.peek()=='#') {
      string dumpStr;
      if(stream.peek()=='#') getline(stream,dumpStr);
      else stream.ignore();
    }
    if(stream.peek()=='}') { //end of struct, break out
      stream.ignore();
      break;
    }
    getline(stream,key,'='); //read until '='
    if(stream) { //found key
      key=stripSpaces(key);
      set(key,stream); // set key based on contents of stream
    }

    //go to next non-whitespace
    while(isspace(stream.peek())) stream.ignore();
  }
}

std::istream& operator>>(std::istream& is, Configurator& cfg){
    cfg.readStream(is);
    return is;
}

void Configurator::readString(const string& str){
  readString(str.c_str(), str.size());
}

void Configurator::readString(const char* str, size_t size){
  // use str as custom buffer in istream without copying
  StreambufWrapper sb((char*)str, size);  
  istream is(&sb);                        
  readStream(is);
}

void Configurator::readString(const char* str){
  readString(str, strlen(str));
}

void Configurator::writeToFile(const std::string& filename){
  // write struct to file
  ofstream os(filename.c_str());
  writeToStream(os);
}

void Configurator::writeToStream(ostream& os,int indent){
  // write struct to stream
  if(indent>0) os<<"{\n"; //print braces on nested structs only
  cfgMultiFunction(CFG_WRITE_ALL, NULL, NULL, NULL, &os, indent, NULL); 
  if(indent>0) os<<Configurator::cfgIndentBy(indent-1)<<"}";
}

std::ostream& operator<<(std::ostream& os, Configurator& cfg) {
  cfg.writeToStream(os);
  return os;
}

void Configurator::writeToString(string& str){
  stringstream ss;
  writeToStream(ss);
  str = ss.str();
}

size_t Configurator::writeToString(char* str, size_t maxSize){
  // use str as custom buffer in ostream without copying
  StreambufWrapper sb(str, maxSize);
  ostream os(&sb);
  writeToStream(os);
  size_t size = sb.getSizeUsed();
  if(size<maxSize) str[size]='\0'; // terminate with \0 if space
  return size;
}

string Configurator::toString(){
  string str;
  writeToString(str);
  return str;
}

bool Configurator::operator==(Configurator& other){
  return !cfgCompareHelper(*this, other); // cfgCompareHelper return 0 if same
}

bool Configurator::operator!=(Configurator& other){
  return cfgCompareHelper(*this, other); // cfgCompareHelper return 0 if same
}

void Configurator::set(const std::string& varName, const std::string& val){
  // set varname = val
  stringstream ss(val);
  set(varName,ss);
}

void Configurator::set(const std::string& varName, std::istream& stream){
  if(varName=="include"){ // e.g. "include = filename"
    string filename;
    cfgSetFromStream(stream,filename); //get filename
    readFile(filename);  //parse contents of file (recurse)
  }else{ // set varName from contents of stream
    // Check for '.' separated format, e.g. "a.b.c=1"
    // If so, strip off baseVar (a) from subVar (b.c)
    size_t pos=varName.find_first_of('.');
    string baseVar = stripSpaces(varName.substr(0,pos));  //first var: e.g. "a"
    string subVar;
    if(pos!=string::npos) subVar = varName.substr(pos+1); //subsequent vars: e.g. "b.c"

    // set value of variable by parsing stream
    int rc=cfgMultiFunction(CFG_SET,&baseVar,&subVar,&stream,NULL,0,NULL);
    if(rc==0) throwError("Configurator ("+getStructName()+") error, key not recognized: "+varName);
    if(rc>1) throwError("Configurator ("+getStructName()+") error, multiple keys with the same name not allowed: "+varName);
    if(!stream) throwError("Configurator ("+getStructName()+") error, parse error after: "+varName);
  }
}

std::string Configurator::cfgIndentBy(int i){
  // return i*2 spaces, for printing
  std::string str;
  for(int j=0;j<i;j++) str+="  ";
  return str;
}

static char readuntil(istream& stream, string& str, const string& delimit){
  // similar to readline, but with multiple delimiters
  char c;
  str="";
  while(stream.get(c)){
    if(delimit.find(c)!=string::npos) break; //found delimit
    if(c=='\\') { //check for escape character
      char cNext = stream.peek(); //peek at next char
      //if delimiter, force grab, otherwise keep '\'
      if(delimit.find(cNext)!=string::npos) stream.get(c); 
    }
    str+=c;
  }
  if(stream) return c; //return last delimiter
  
  if(str.size()>0) stream.clear(); // read successful, clear fail
  return 0; //return 0 if end of stream
}

void Configurator::cfgSetFromStream(istream& ss, string& str, const std::string& subVar){
  // special handing of string (default handling only reads one word)
  if(!subVar.empty()) { //subVar should be empty
    ss.setstate(ios::failbit); //set fail bit to trigger error handling
    return;
  }
  char c=readuntil(ss,str,STR_DELIM); //find end of string
  if(c) ss.putback(c); // put the delimiter back on, so it can be handled later
  str = stripSpaces(str); 
  if(str == "''" || str == "\"\"") str = ""; // "" and '' indicate empty string
}

void Configurator::cfgSetFromStream(std::istream& ss, Configurator& cfg, const std::string& subVar){
  // read struct from stream
  if(!subVar.empty()) cfg.set(subVar,ss);  //handle a.b.c=1 format, recursively
  else                cfg.readStream(ss);  //handle standard format (a=1)
} 

static bool streql(const string&str1, const string&str2){
  // compare strings case insensitive
  #ifdef __GNUC__
    return strcasecmp(str1.c_str(),str2.c_str())==0;
  #else
    return _stricmp(str1.c_str(),str2.c_str())==0;
  #endif
}

void Configurator::cfgSetFromStream(istream& ss, bool& b, const std::string& subVar){
  if(!subVar.empty()) { //subVar should be empty
    ss.setstate(ios::failbit); //set fail bit to trigger error handling
    return;
  }
  string str;
  cfgSetFromStream(ss,str);
  str=stripSpaces(str);
  if(streql(str,"true") || streql(str,"t") || streql(str,"1")){
    b=true;
  }else if(streql(str,"false") || streql(str,"f") || streql(str,"0")){
    b=false;
  }else ss.setstate(ios::failbit); //set fail bit to trigger error handling
}

void Configurator::cfgWriteToStreamHelper(std::ostream& stream, Configurator& cfg, int indent){
  // write struct to stream
  cfg.writeToStream(stream,indent+1);
}

void Configurator::cfgWriteToStreamHelper(std::ostream& stream, bool& b, int indent){
  if(b) stream<<"true";
  else  stream<<"false";
}

void Configurator::cfgWriteToStreamHelper(std::ostream& stream, std::string& str, int indent){
  if(str.empty()) stream<<"''";  //empty string indicated by ''
  else {
    stringstream tmpSs(str);
    string tmpStr;
    while(1){   // write str to stream, prepending delimiters with '\'
      char c=readuntil(tmpSs,tmpStr,STR_DELIM);
      stream<<tmpStr;
      if(c) stream<<"\\"<<c; //prepend delimiter
      else  break;
    }
  }
}

int Configurator::cfgCompareHelper(Configurator& a, Configurator& b){
  return a.cfgMultiFunction(CFG_COMPARE, NULL,NULL,NULL,NULL,0,&b);
}

} //end namespace codepi
