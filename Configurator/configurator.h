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

#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <array>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <iterator>
#include <type_traits>
#include <assert.h>

#include "Optional.h"

#ifdef _MSC_VER // if Visual Studio
#pragma warning( error : 4002 ) // treat macros with incorrect number of args as error
#endif

namespace codepi {

//////////////////////////////////////////////////////////////////
// Configurator - virtual base class

/// Virtual class for serializing and deserializing config files.
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
	template <typename T> static T cfgGetDefaultVal(const T&var){return T();}
	/// overridable method called on parse error
	virtual void throwError(std::string error){ throw std::runtime_error(error); }

	//////////////////////////////////////////////////////////////////
	// cfgSetFromStream(stream, val, subVar)
	// Used internally by cfgMultiFunction
	// Sets value of val based on contents of stream
	// subVar is for '.' separated nested structs, e.g. "a.b=5"
	// Overloaded for multiple types: string, configurator descendants, bool, 
	//   pair, various STL containers, and primatives

	/// cfgSetFromStream for strings.  
	/// by default, operator>> will only read one word at a time
	/// this instead will read until a delimiter: ,#}]\t\r\n
	static void cfgSetFromStream(std::istream& ss, std::string& str, const std::string& subVar="");

	/// cfgSetFromStream for Configurator descendants
	static void cfgSetFromStream(std::istream& ss, Configurator& cfg, const std::string& subVar="");

	/// cfgSetFromStream for bool (allows (t,true,1,f,false,0))
	static void cfgSetFromStream(std::istream& ss, bool& b, const std::string& subVar="");

	/// helper function for cfgSetFromStream for pairs
	/// workaround: a map's value_type is pair<const T1, T2> this casts off the const
	template <typename T>
	static T& remove_const(const T& val){
		return const_cast<T&>(val);
	}

	/// cfgSetFromStream for std::pair
	/// element pair separated by whitespace or comma
	/// note: string elements may contain spaces
	template <typename T1, typename T2>
	static void cfgSetFromStream(std::istream& is, std::pair<T1,T2>& pair, const std::string& subVar=""){
		cfgSetFromStream(is, remove_const(pair.first), subVar);
		// push to next element, removing comments
		while(isspace(is.peek())||is.peek()==','||is.peek()=='#') {
			std::string dumpStr;
			if(is.peek()=='#') getline(is,dumpStr);
			else is.ignore();
		}
		cfgSetFromStream(is, pair.second, subVar);
	}

	/// cfgSetFromStream helper for many STL containers
	/// Parses container from stream of format: "[1 2 3 4 5]" (commas required for strings, optional for others)
	template <typename Container>
	static void cfgContainerSetFromStream(std::istream& is, Container& container, const std::string& subVar="");

	/// cfgSetFromStream for vectors
	/// wrapper for cfgContainerSetFromStream
	template <typename T>
	static void cfgSetFromStream(std::istream& is, std::vector<T>& vec, const std::string& subVar=""){
		cfgContainerSetFromStream(is, vec, subVar);
	}

	/// cfgSetFromStream for stl array
	/// wrapper for cfgContainerSetFromStream
	template <typename T, size_t N>
	static void cfgSetFromStream(std::istream& is, std::array<T,N>& arr, const std::string& subVar=""){
		cfgContainerSetFromStream(is, arr, subVar);
	}

	/// cfgSetFromStream for sets
	/// wrapper for cfgContainerSetFromStream
	template <typename T>
	static void cfgSetFromStream(std::istream& is, std::set<T>& set, const std::string& subVar=""){
		cfgContainerSetFromStream(is, set, subVar);
	}

	/// cfgSetFromStream for map 
	/// Parses map from stream of format: "[key1, val1, key2, val2]" 
	/// wrapper for cfgContainerSetFromStream
	template <typename T1, typename T2>
	static void cfgSetFromStream(std::istream& is, std::map<T1,T2>& map, const std::string& subVar=""){
		cfgContainerSetFromStream(is, map, subVar);
	}	

	/// cfgSetFromStream for Optional<T>
	template <typename T>
	static void cfgSetFromStream(std::istream& is, Optional<T>& val, const std::string& subVar=""){
		cfgSetFromStream(is, (T&)val, subVar);
	}

	/// cfgSetFromStream for all other types
	/// the enable_if is required to prevent it from matching on Configurator descendants
	template <typename T>
	static typename std::enable_if<!std::is_base_of<Configurator,T>::value,void>::type
		cfgSetFromStream(std::istream& ss,  T& val, const std::string& subVar=""){
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
	// Overloaded for multiple types: string, configurator descendants, bool, 
	//   pair, various STL containers, and primatives

	/// cfgWriteToStreamHelper for string 
	static void cfgWriteToStreamHelper(std::ostream& stream, std::string& str, int indent);

	/// cfgWriteToStreamHelper for descendants of Configurator
	static void cfgWriteToStreamHelper(std::ostream& stream, Configurator& cfg, int indent);

	/// cfgWriteToStreamHelper for bool (writes true/false)
	static void cfgWriteToStreamHelper(std::ostream& stream, bool& b, int indent);

	/// cfgWriteToStreamHelper for std::pair
	template <typename T1, typename T2>
	static void cfgWriteToStreamHelper(std::ostream& stream, std::pair<T1,T2>& pair, int indent){
		cfgWriteToStreamHelper(stream, pair.first, indent);
		stream << ",";
		cfgWriteToStreamHelper(stream, pair.second, indent);
	}

	/// cfgWriteToStreamHelper for anything with iterators
	/// Prints container to stream in format: "[1,2,3,4,5]"
	template <typename Container>
	static void cfgContainerWriteToStreamHelper(std::ostream& stream, Container& container, int indent);

	/// cfgWriteToStreamHelper for vectors
	/// Prints vector to stream in format: "[1,2,3,4,5]"
	template <typename T>
	static void cfgWriteToStreamHelper(std::ostream& stream, std::vector<T>& vec, int indent){
		cfgContainerWriteToStreamHelper(stream, vec, indent);
	}

	/// cfgWriteToStreamHelper for stl array
	/// Prints array to stream in format: "[1,2,3,4,5]"
	template <typename T, size_t N>
	static void cfgWriteToStreamHelper(std::ostream& stream, std::array<T,N>& vec, int indent){
		cfgContainerWriteToStreamHelper(stream, vec, indent);
	}

	/// cfgWriteToStreamHelper for sets
	/// Prints set to stream in format: "[1,2,3,4,5]"
	template <typename T>
	static void cfgWriteToStreamHelper(std::ostream& stream, std::set<T>& set, int indent){
		cfgContainerWriteToStreamHelper(stream, set, indent);
	}

	/// cfgWriteToStreamHelper for maps
	/// Prints map to stream in format: "[key1,val1,key2,val2]"
	template <typename T1, typename T2>
	static void cfgWriteToStreamHelper(std::ostream& stream, std::map<T1,T2>& map, int indent){
		cfgContainerWriteToStreamHelper(stream, map, indent);
	}

	/// cfgWriteToStreamHelper for Optional<T>
	/// Prints contents of Optional.
	template <typename T>
	static void cfgWriteToStreamHelper(std::ostream& stream, Optional<T>& opt, int indent){
		// shouldn't be able to get this far if not set
		if(!opt.isSet()) throw std::runtime_error("cfgWriteToStreamHelper Optional<T>: this shouldn't happen");
		cfgWriteToStreamHelper(stream, (T&)opt, indent);
	}

	/// cfgWriteToStreamHelper for all other types
	/// the enable_if is required to prevent it from matching on Configurator descendants
	template <typename T>
	static typename std::enable_if<!std::is_base_of<Configurator,T>::value,void>::type
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

	template <typename Container>
	static int cfgContainerCompareHelper(Container& a, Container& b){
		int retVal = 0;
		typename Container::iterator i = a.begin();
		typename Container::iterator j = b.begin();
		while(i!=a.end() && j!=b.end()){
			retVal+=cfgCompareHelper(*i,*j);
			i++; j++;
		}
		if(i!=a.end() || j!=b.end()) return 1; // containers not same size

		return retVal;
	}
	
	/// cfgCompareHelper for any type with defined operator==
	template <typename T>
	static typename std::enable_if<!std::is_base_of<Configurator,T>::value,int>::type
		cfgCompareHelper(T& a, T& b){
			return !(a==b);
	}
	
	template <typename T>
	static int cfgCompareHelper(std::vector<T>& a, std::vector<T>& b){
		return cfgContainerCompareHelper(a,b);
	}

	template <typename T, size_t N>
	static int cfgCompareHelper(std::array<T,N>& a, std::array<T,N>& b){
		return cfgContainerCompareHelper(a,b);
	}

	template <typename T>
	static int cfgCompareHelper(std::set<T>& a, std::set<T>& b){
		return cfgContainerCompareHelper(a,b);
	}

	template <typename T1, typename T2>
	static int cfgCompareHelper(std::map<T1,T2>& a, std::map<T1,T2>& b){
		return cfgContainerCompareHelper(a,b);
	}

	template <typename T>
	static int cfgCompareHelper(Optional<T>& a, Optional<T>& b){
		if(!a.isSet() && !b.isSet()) return 0;  // both empty, so same
		if(!a.isSet() || !b.isSet()) return 1;  // one empty, so different
		return cfgCompareHelper((T&)a,(T&)b);
	}

	/// returns true if optional type and value is set
	template<typename T>
	static bool cfgIsSetOrNotOptional(Optional<T>& opt){
		return opt.isSet();
	}

	/// returns true if not instance of Optional<T>
	template<typename T>
	static bool cfgIsSetOrNotOptional(T& t){
		return true;
	}

	//////////////////////////////
	// Helper function to distinguish between stl arrays and other containers

	// clear array by filling it with default values
	template<typename T, size_t N>
	static void clear_helper(std::array<T,N>& arr){
		arr.fill(T());
	}

	// clear container
	template<typename Container>
	static void clear_helper(Container& container){
		container.clear();
	}

	// inserting into array by index
	template<typename T, size_t N>
	static void insert_helper(std::array<T,N>& arr, size_t i, T& val){
		if(i>=N) throw std::range_error("insert exceeds array size");
		arr[i] = val;
	}

	// inserting into end of container (ignoring index, but should match anyway)
	template<typename Container, typename T>
	static void insert_helper(Container& container, size_t i, T& val){
		assert(container.size()==i); 
		container.insert(container.end(),val);
	}


public:  // needs to be public for certain versions of gcc
	/// Used by TTHelper::is_configurator to identify decendents of Configurator
	typedef void is_configurator;
};

//////////////////////////////////////////////////////////////////
// Implementation of templated static methods from Configurator

// setFromString for vectors
// Parses vector from stream of format: "[1 2 3 4 5]" (commas required for strings, optional for others)
template <typename Container>
void Configurator::cfgContainerSetFromStream(std::istream& is, Container& container, const std::string& subVar){
	if(!subVar.empty()) { //subVar should be empty
		is.setstate(std::ios::failbit); //set fail bit to trigger error handling
		return;
	}
	std::string line;
	clear_helper(container);
	
	// find next non-space
	while(isspace(is.peek())) is.ignore();
	
	// make sure '['
	char c = is.get();
	if(c!='['){ // if c is not '[' set failbit in stream
		is.setstate(std::ios::failbit); 
		return;
	}

	// find next non-space
	while(isspace(is.peek())) is.ignore();

	// parse each element
	size_t index=0;
	while(is.good()){
		// check for each of vector
		if(is.peek()==']'){
			is.ignore();
			break;
		}

		// read element and add to vector
		typename Container::value_type val;
		cfgSetFromStream(is,val);
		try{
			if(is) insert_helper(container, index, val);
		}catch(std::range_error&){
			is.setstate(std::ios::failbit);  // exceeded container size
			return;
		}

		// push to next element, removing comments
		while(isspace(is.peek())||is.peek()==','||is.peek()=='#') {
			std::string dumpStr;
			if(is.peek()=='#') getline(is,dumpStr);
			else is.ignore();
		}

		index++;
	}
}

// cfgWriteToStreamHelper for vectors
// Prints container to stream in format: "[1,2,3,4,5]"
template <typename Container>
void Configurator::cfgContainerWriteToStreamHelper(std::ostream& stream, Container& c, int indent){
	stream<<"[";
	for(typename Container::iterator i=c.begin(); i!=c.end(); i++){
		if(i!=c.begin()) stream<<",";
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
	if(mfType==CFG_INIT_ALL) { \
		if(cfgIsSetOrNotOptional(varName)) {varName = defaultVal;retVal++;} \
    } else if(mfType==CFG_SET && #varName==*str) { cfgSetFromStream(*streamIn,varName,*subVar);retVal++;} \
	else if(mfType==CFG_WRITE_ALL) { \
		if(cfgIsSetOrNotOptional(varName)) { \
			*streamOut<<cfgIndentBy(indent)<<#varName<<"="; \
			cfgWriteToStreamHelper(*streamOut,varName,indent); \
			*streamOut<<std::endl;retVal++; \
			if(streamOut->fail()) \
				throwError("Configurator ("+getStructName()+") error, can't write variable: "+#varName); \
		} \
	} else if(mfType==CFG_COMPARE) { \
		retVal+=cfgCompareHelper(this->varName,otherPtr->varName); \
	}

// alternative to CFG_ENTRY2 used when default defaultVal is sufficient
#define CFG_ENTRY1(varName) CFG_ENTRY2(varName, cfgGetDefaultVal(varName))

// calls cfgMultiFunction method of parent
// allows for inheritance
#define CFG_PARENT(parentName) \
	int rc=parentName::cfgMultiFunction(mfType,str,subVar,streamIn,streamOut,indent,other); \
	retVal+=rc;
	

// closes out cfgMultiFunction method
#define CFG_TAIL return retVal; }

} //end namespace codepi
