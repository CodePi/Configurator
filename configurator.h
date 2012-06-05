// Copyright (C) 2011 Paul Ilardi (configurator@pilardi.com)
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
// Tested with gcc 4.1, gcc 4.3, vs2005, vs2010.
// NOTE: On Windows, recommend compiling with /we4002 compile flag.  
//       This will catch "incorrect number of args in macro" errors.

#pragma once

#include<string>
#include<vector>
#include<set>
#include<sstream>
#include<fstream>
#include<stdexcept>
#include<iomanip>
#include <iterator>

#include"type_traits_helper.h"

#ifdef _MSC_VER // if Visual Studio
#pragma warning( error : 4002 ) // treat macros with incorrect number of args as error
#endif

//////////////////////////////////////////////////////////////////
// Configurator - virtual base class

/// Virtual class for serializing and deserializing config files.
class Configurator{
public:
	/// read and parse file / stream / string
	void readFile(const std::string& filename);
	void readStream(std::istream& stream);
	void readString(const std::string& str);
	void readString(const char* str, int size);
	void readString(const char* str);

	/// write contents of struct to file / stream / string
	void writeToFile(const std::string& filename);
	void writeToStream(std::ostream& stream,int indent=0);
	void writeToString(std::string& str);
	int  writeToString(char* str, int maxSize); //returns bytes used
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

protected:
	enum MFType{CFG_INIT_ALL,CFG_SET,CFG_WRITE_ALL,CFG_COMPARE};

	/// Helper method that is called by all of the public methods above.
	///   This method is automatically generated in subclass using macros below
	///   Returns the number of variables matched
	virtual int cfgMultiFunction(MFType mfType, std::string* str, std::string* subVar,
		std::istream* streamIn, std::ostream* streamOut, int indent, 
		Configurator* other)=0;

	/// return i*2 spaces, for printing
	static std::string cfgIndentBy(int i);
	/// returns default value of type T
	template <typename T> static T cfgGetDefaultVal(const T&var){ return T();}
	/// overridable method called on parse error
	virtual void throwError(std::string error){ throw std::runtime_error(error); }

	//////////////////////////////////////////////////////////////////
	// cfgSetFromStream(stream, val, subVar)
	// Used internally by cfgMultiFunction
	// Sets value of val based on contents of stream
	// subVar is for '.' separated nested structs, e.g. "a.b=5"
	// Five definitions below: Configurator, string, vector, bool, and other

	/// cfgSetFromStream for strings.  
	/// by default, operator>> will only read one word at a time
	/// this one will read until a delimiter: ,#}]\t\r\n
	static void cfgSetFromStream(std::istream& ss, std::string& str, const std::string& subVar="");

	/// cfgSetFromStream for Configurator descendants
	static void cfgSetFromStream(std::istream& ss, Configurator& cfg, const std::string& subVar="");

	/// cfgSetFromStream for bool (allows (t,true,1,f,false,0))
	static void cfgSetFromStream(std::istream& ss, bool& b, const std::string& subVar="");

	/// cfgSetFromStream for std::pair
	template <typename T1, typename T2>
	static void cfgSetFromStream(std::istream& is, std::pair<T1,T2>& pair, const std::string& subVar=""){
		cfgSetFromStream(is, pair.first, subVar);
		// push to next element, removing comments
		while(isspace(is.peek())||is.peek()==','||is.peek()=='#') {
			std::string dumpStr;
			if(is.peek()=='#') getline(is,dumpStr);
			else is.ignore();
		}
		cfgSetFromStream(is, pair.second, subVar);
	}

	/// cfgSetFromStream helper for any iterators
	/// Parses container from stream of format: "[1 2 3 4 5]" (commas required for strings, optional for others)
	template <typename Inserter>
	static void cfgInsertFromStream(std::istream& is, Inserter inserter, const std::string& subVar="");

	/// cfgSetFromStream for vectors
	/// Parses vector from stream of format: "[1 2 3 4 5]" (commas required for strings, optional for others)
	template <typename T>
	static void cfgSetFromStream(std::istream& is, std::vector<T>& vec, const std::string& subVar=""){
		vec.clear();
		cfgInsertFromStream(is, std::back_inserter(vec), subVar);
	}

	/// cfgSetFromStream for sets
	/// Parses sets from stream of format: "[1 2 3 4 5]" (commas required for strings, optional for others)
	template <typename T>
	static void cfgSetFromStream(std::istream& is, std::set<T>& set, const std::string& subVar=""){
		set.clear();
		cfgInsertFromStream(is, std::inserter(set, set.end()), subVar);
	}

	/// cfgSetFromStream for all other types
	/// the enable_if is required to prevent it from matching on Configurator descendants
	/// note: this is defined inline because vs2005 has trouble compiling otherwise
	template <typename T>
	static typename TTHelper::enable_if<!TTHelper::is_configurator<T>::value,void>::type
		cfgSetFromStream(std::istream& ss, T& val, const std::string& subVar=""){
			if(!subVar.empty()) { //subVar should be empty
				ss.setstate(std::ios::failbit); //set fail bit to trigger error handling
				return;
			}
			ss>>std::setbase(0)>>val;
	}

	//////////////////////////////////////////////////////////////////
	// cfgWriteToStreamHelper(stream, val, indent)
	// Used internally by cfgMultiFunction
	// Writes the contents of val to the stream
	// Three definitions below: Configurator, vector, and other

	/// cfgWriteToStreamHelper for descendants of Configurator
	static void cfgWriteToStreamHelper(std::ostream& stream, Configurator& cfg, int indent);

	/// cfgWriteToStreamHelper for bool (writes true/false)
	static void cfgWriteToStreamHelper(std::ostream& stream, bool& b, int indent);

	/// cfgWriteToStreamHelper for string 
	static void cfgWriteToStreamHelper(std::ostream& stream, std::string& str, int indent);

	/// cfgWriteToStreamHelper for std::pair
	template <typename T1, typename T2>
	static void cfgWriteToStreamHelper(std::ostream& stream, std::pair<T1,T2>& pair, int indent){
		cfgWriteToStreamHelper(stream, pair.first, indent);
		stream << ",";
		cfgWriteToStreamHelper(stream, pair.second, indent);
	}

	/// cfgWriteToStreamHelper for anything with iterators
	/// Prints container to stream in format: "[1,2,3,4,5]"
	template <typename Iterator>
	static void cfgWriteToStreamHelper(std::ostream& stream, Iterator start, 
		Iterator end, int indent);

	/// cfgWriteToStreamHelper for vectors
	/// Prints vector to stream in format: "[1,2,3,4,5]"
	template <typename T>
	static void cfgWriteToStreamHelper(std::ostream& stream, std::vector<T>& vec, int indent){
		cfgWriteToStreamHelper(stream, vec.begin(), vec.end(), indent);
	}

	/// cfgWriteToStreamHelper for sets
	/// Prints set to stream in format: "[1,2,3,4,5]"
	template <typename T>
	static void cfgWriteToStreamHelper(std::ostream& stream, std::set<T>& set, int indent){
		cfgWriteToStreamHelper(stream, set.begin(), set.end(), indent);
	}

	/// cfgWriteToStreamHelper for all other types
	/// the enable_if is required to prevent it from matching on Configurator descendants
	/// note: this is defined inline because vs2005 has trouble compiling otherwise
	template <typename T>
	static typename TTHelper::enable_if<!TTHelper::is_configurator<T>::value,void>::type
		cfgWriteToStreamHelper(std::ostream& stream, T& val, int indent){
			stream<<val;
	}

	/////////////////////////////////////////////////////////////////////////////
	// cfgCompareHelper(a, b)
	// returns 0 if same, >0 if different

	/// cfgCompareHelper for Configurator
	static int cfgCompareHelper(Configurator& a, Configurator& b);

	template <typename T1, typename T2>
	static int cfgCompareHelper(std::pair<T1,T2>& a, std::pair<T1,T2>& b){
		return cfgCompareHelper(a.first, b.first) + cfgCompareHelper(a.second, b.second);
	}

	template <typename Iterator>
	static int cfgCompareHelper(Iterator start1, Iterator end1, Iterator start2, Iterator end2){
		int retVal = 0;
		Iterator a = start1;
		Iterator b = start2;
		while(a!=end1 && b!=end2){
			retVal+=cfgCompareHelper(*a,*b);
			a++; b++;
		}
		if(a!=end1 || b!=end2) return 1; // containers not same size

		return retVal;
	}
	
	/// cfgCompareHelper for any type with defined operator==
	template <typename T>
	static typename TTHelper::enable_if<!TTHelper::is_configurator<T>::value,int>::type
		cfgCompareHelper(T& a, T& b){
			return !(a==b);
	}
	
	template <typename T>
	static int cfgCompareHelper(std::vector<T>& a, std::vector<T>& b){
		return cfgCompareHelper(a.begin(), a.end(), b.begin(), b.end());
	}

	template <typename T>
	static int cfgCompareHelper(std::set<T>& a, std::set<T>& b){
		return cfgCompareHelper(a.begin(), a.end(), b.begin(), b.end());
	}

	/// Used by TTHelper::is_configurator to identify decendents of Configurator
	typedef void is_configurator;

};

//////////////////////////////////////////////////////////////////
// Implementation of templated static methods from Configurator

// setFromString for vectors
// Parses vector from stream of format: "[1 2 3 4 5]" (commas required for strings, optional for others)
template <typename Inserter>
void Configurator::cfgInsertFromStream(std::istream& is, Inserter inserter, const std::string& subVar){
	if(!subVar.empty()) { //subVar should be empty
		is.setstate(std::ios::failbit); //set fail bit to trigger error handling
		return;
	}
	std::string line;
	// find first element
	while(isspace(is.peek())||is.peek()=='[') is.ignore();
	while(is.good()){
		// check for each of vector
		if(is.peek()==']'){
			is.ignore();
			break;
		}

		// read element and add to vector
		Inserter::container_type::value_type val;
		cfgSetFromStream(is,val);
		if(is) {
			*inserter = val;
			inserter++;
		}

		// push to next element, removing comments
		while(isspace(is.peek())||is.peek()==','||is.peek()=='#') {
			std::string dumpStr;
			if(is.peek()=='#') getline(is,dumpStr);
			else is.ignore();
		}
	}
}

// cfgWriteToStreamHelper for vectors
// Prints container to stream in format: "[1,2,3,4,5]"
template <typename Iterator>
void Configurator::cfgWriteToStreamHelper(std::ostream& stream, Iterator start, 
		Iterator end, int indent){
	stream<<"[";
	for(Iterator i=start; i!=end; i++){
		if(i!=start) stream<<",";
		cfgWriteToStreamHelper(stream,*i,indent);
	}
	stream<<"]";
}

//////////////////////////////////////////////////////////////////
// Macros to automatically generate the cfgMultiFunction method in
// descendant classes.

// automatically generates subclass constructor and begins cfgMultiFunction method
#define CFG_HEADER(structName) \
	structName() { cfgMultiFunction(CFG_INIT_ALL,NULL,NULL,NULL,NULL,0,NULL); } \
	std::string getStructName() { return #structName; } \
	int cfgMultiFunction(MFType mfType, std::string* str, std::string* subVar, \
		std::istream* streamIn, std::ostream* streamOut,int indent,Configurator*other){ \
		int retVal=0; \
		structName* otherPtr; \
		if(mfType==CFG_COMPARE) {otherPtr = dynamic_cast<structName*>(other); \
			if(!otherPtr) return 1; /*dynamic cast failed, types different*/ }

// continues cfgMultiFunction method, called for each member variable in struct 
#define CFG_ENTRY2(varName, defaultVal) \
	if(mfType==CFG_INIT_ALL) {varName = defaultVal;retVal++;} \
	else if(mfType==CFG_SET && #varName==*str) { cfgSetFromStream(*streamIn,varName,*subVar);retVal++;}\
	else if(mfType==CFG_WRITE_ALL) {*streamOut<<cfgIndentBy(indent)<<#varName<<"="; \
		cfgWriteToStreamHelper(*streamOut,varName,indent); \
		*streamOut<<std::endl;retVal++; \
		if(streamOut->fail()) \
		throwError("Configurator ("+getStructName()+") error, can't write variable: "+#varName);} \
	else if(mfType==CFG_COMPARE) { \
		retVal+=cfgCompareHelper(this->varName,otherPtr->varName); }	

// alternative to CFG_ENTRY2 used when default defaultVal is sufficient
#define CFG_ENTRY1(varName) CFG_ENTRY2(varName, cfgGetDefaultVal(varName))

// calls cfgMultiFunction method of parent
// allows for inheritance
#define CFG_PARENT(parentName) \
	int rc=parentName::cfgMultiFunction(mfType,str,subVar,streamIn,streamOut,indent,other); \
	retVal+=rc;
	

// closes out cfgMultiFunction method
#define CFG_TAIL return retVal; }
