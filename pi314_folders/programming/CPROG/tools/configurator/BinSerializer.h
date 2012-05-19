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

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "type_traits_helper.h"

#ifdef _MSC_VER // if Visual Studio
#pragma warning( error : 4002 ) // treat macros with incorrect number of args as error
#endif

class BinSerializer{
public:
	/// Serialize methods
	void serialize(std::ostream& os);
	void serialize(std::vector<char>& blob);
	int  serialize(char* blob, int bufferSize); // returns size used
	void serializeFile(const char* filename);

	/// Deserialize methods
	void deserialize(std::istream& is);
	void deserialize(std::vector<char>& blob);
	void deserialize(const char* blob, int bufferSize);
	void deserializeFile(const char* filename);

	/// Returns serialized size of structure
	int getSerializeSize();

	/// Dump text represenation to stream
	void dumpText(std::ostream& os);

	/// return name of current struct
	virtual std::string getStructName()=0;

	//virtualized destructor for proper inheritance
	virtual ~BinSerializer(){}   

protected:
	enum MFType{INIT_ALL,SERIALIZE,DESERIALIZE,DUMP_TEXT,GET_SIZE,GET_NUM_ENTRIES};

	/// Helper method that is called by all of the public methods above.
	///   This method is automatically generated in subclass using macros below
	///   Returns the number of variables matched
	virtual int multiFunction(MFType mfType, std::istream* streamIn, 
		std::ostream* streamOut, int numEntries)=0;

	/// returns default value of type T
	template <typename T> static T getDefaultVal(const T&var){ return T();}
	/// overridable method called on parse error
	virtual void throwError(std::string error);

	//////////////////////////////////////////////////////////////////////////
	// Helper methods to serialize data types 
	//////////////////////////////////////////////////////////////////////////

	// String serializer
	void serializeHelper(std::string& var, std::ostream& streamOut);

	// Nested class serializer
	void serializeHelper(BinSerializer& var, std::ostream& streamOut);

	// Vector serializer
	template <typename T>
	void serializeHelper(std::vector<T>& var, std::ostream& streamOut){
		int size = var.size();
		streamOut.write((char*)&size, sizeof(size));  // output size to stream
		if(!streamOut) throwError("serializeHelper: vector size write error");
		// TODO: bulk write if possible
		for(int i=0;i<size;i++){              // output each element to stream
			serializeHelper(var[i], streamOut);
		}
	}

	// "other" serializer, works on any contiguous object or basic type
	template <typename T>
	typename TTHelper::enable_if<!TTHelper::is_configurator<T>::value,void>::type
		serializeHelper(T& var, std::ostream& streamOut){
			streamOut.write((char*)&var, sizeof(var));
			if(!streamOut) throwError("serializeHelper: \"other\" write error");
	}

	//////////////////////////////////////////////////////////////////////////
	// Helper methods to deserialize data types 
	//////////////////////////////////////////////////////////////////////////

	// String deserializer
	void deserializeHelper(std::string& var, std::istream& streamIn);

	// Nested class deserializer
	void deserializeHelper(BinSerializer& var, std::istream& streamIn);

	// Vector deserializer
	template <typename T>
	void deserializeHelper(std::vector<T>& var, std::istream& streamIn){
		int size;
		streamIn.read((char*)&size, sizeof(size));  // read size to stream
		if(!streamIn) throwError("deserializeHelper: vector size read error");
		var.resize(size);
		// TODO: bulk read if possible
		for(int i=0;i<size;i++){              // output each element to stream
			deserializeHelper(var[i], streamIn);
		}
	}
	
	// "other" deserializer, works on any contiguous object or basic type
	template <typename T>
	typename TTHelper::enable_if<!TTHelper::is_configurator<T>::value,void>::type
		deserializeHelper(T& var, std::istream& streamIn){
			streamIn.read((char*)&var, sizeof(var));
			if(!streamIn) throwError("deserializeHelper: \"other\" read error");
	}

	//////////////////////////////////////////////////////////////////////////
	// Helper methods to get serialized size of data
	//////////////////////////////////////////////////////////////////////////

	// String size calculator
	int getSizeHelper(std::string& var);

	// Nested class size calculator
	int getSizeHelper(BinSerializer& var);

	// Vector size calculator
	template <typename T>
	int getSizeHelper(std::vector<T>& var){
		int size=0;
		size+=sizeof(int);  // size of counter
		for(size_t i=0;i<var.size();i++){
			size+=getSizeHelper(var[i]); //size of each element
		}
		return size;
	}

	// "other" size calculator, works on any contiguous object or basic type
	template <typename T>
	typename TTHelper::enable_if<!TTHelper::is_configurator<T>::value,int>::type
		getSizeHelper(T& var){
			return sizeof(var);
	}

	//////////////////////////////////////////////////////////////////////////
	// Helper methods to dump text to stream
	//////////////////////////////////////////////////////////////////////////

	// Dumps text version of nested class to stream
	void dumpTextHelper(std::ostream& os, BinSerializer& var){
		os<<"{\n";
		var.dumpText(os);
		os<<"}\n";
	}

	// Dumps text version of vector contents to stream
	template <typename T>
	void dumpTextHelper(std::ostream& os, std::vector<T>& var){
		os<<"[";
		for(size_t i=0;i<var.size();i++) {
			dumpTextHelper(os, var[i]);  // dump element
			if(i<var.size()-1) os<<",";
		}
		os<<"]";
	}

	// Dumps text version of any type that supports << to stream
	template <typename T>
	typename TTHelper::enable_if<!TTHelper::is_configurator<T>::value,void>::type
		dumpTextHelper(std::ostream& os, T& var){
			os<<var;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// Used by TTHelper::is_configurator to identify decendents of Configurator (or BinSerializer)
	typedef void is_configurator;

};

//////////////////////////////////////////////////////////////////
// Macros to automatically generate the multiFunction method in
// descendant classes.

// automatically generates subclass constructor and begins multiFunction method
#define BS_HEADER(structName) \
	structName() { multiFunction(INIT_ALL,NULL,NULL, 0); } \
	std::string getStructName() { return #structName; } \
	int multiFunction(MFType mfType, std::istream* streamIn, std::ostream* streamOut, int numEntries){ \
		int retVal=0; 

// Similar to BS_HEADER, but prepends and checks structId in serialization
#define BS_HEADER_WITH_ID(structName,structId) \
	BS_HEADER(structName) \
		if(mfType==SERIALIZE) { \
			int structIdWrite = structId; \
			serializeHelper(structIdWrite,*streamOut);\
		}else if(mfType==DESERIALIZE) { \
			int structIdRead; \
			deserializeHelper(structIdRead, *streamIn); \
			if(structIdRead!=structId) {\
				std::stringstream err; \
				err << "deserialize: incorrect structId.  Got: " <<structIdRead<<" Expected: "<<structId; \
				throwError(err.str()); \
			} \
		}else if(mfType==GET_SIZE) retVal+=sizeof(int);

// continues multiFunction method, needed for each member varible in struct 
#define BS_ENTRY_D(varName, initVal) \
	if(mfType==INIT_ALL) {varName = initVal;retVal++;} \
	else if(mfType==SERIALIZE) {serializeHelper(varName,*streamOut);retVal++;}\
	else if(mfType==DESERIALIZE) {\
		if(retVal<numEntries) {deserializeHelper(varName,*streamIn);retVal++;}\
		else                  {varName = initVal;retVal++; }} \
	else if(mfType==GET_SIZE) {retVal+=getSizeHelper(varName);}\
	else if(mfType==GET_NUM_ENTRIES) {retVal++;} \
	else if(mfType==DUMP_TEXT) { \
		*streamOut<<#varName<<": "; \
		dumpTextHelper(*streamOut,varName); \
		*streamOut<<std::endl;}

// alternative to BS_ENTRY_D, initialized to type default
#define BS_ENTRY(varName) BS_ENTRY_D(varName, getDefaultVal(varName))

// batch version of BS_ENTRY
#define BS_ENTRY_MULTI1(varName1) \
	BS_ENTRY(varName1)  
#define BS_ENTRY_MULTI2(varName1,varName2) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI1(varName2)
#define BS_ENTRY_MULTI3(varName1,varName2,varName3) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI2(varName2,varName3)
#define BS_ENTRY_MULTI4(varName1,varName2,varName3,varName4) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI3(varName2,varName3,varName4)
#define BS_ENTRY_MULTI5(varName1,varName2,varName3,varName4,varName5) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI4(varName2,varName3,varName4,varName5)
#define BS_ENTRY_MULTI6(varName1,varName2,varName3,varName4,varName5,varName6) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI5(varName2,varName3,varName4,varName5,varName6)
#define BS_ENTRY_MULTI7(varName1,varName2,varName3,varName4,varName5,varName6,varName7) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI6(varName2,varName3,varName4,varName5,varName6,varName7)
#define BS_ENTRY_MULTI8(varName1,varName2,varName3,varName4,varName5,varName6,varName7,varName8) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI7(varName2,varName3,varName4,varName5,varName6,varName7,varName8)
#define BS_ENTRY_MULTI9(varName1,varName2,varName3,varName4,varName5,varName6,varName7,varName8,varName9) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI8(varName2,varName3,varName4,varName5,varName6,varName7,varName8,varName9)
#define BS_ENTRY_MULTI10(varName1,varName2,varName3,varName4,varName5,varName6,varName7,varName8,varName9,varName10) \
	BS_ENTRY(varName1) \
	BS_ENTRY_MULTI9(varName2,varName3,varName4,varName5,varName6,varName7,varName8,varName9,varName10)

// calls multiFunction method of parent
// allows for inheritance
#define BS_PARENT(parentName) \
	int rc=parentName::multiFunction(mfType,streamIn,streamOut); \
	retVal+=rc;

// closes out multiFunction method
#define BS_TAIL return retVal; }
