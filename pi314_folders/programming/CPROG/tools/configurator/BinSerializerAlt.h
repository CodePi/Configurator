// BinSerializer with a slightly altered interface to allow macros to go into cpp files

#pragma once

#include "BinSerializer.h"

// automatically generates subclass constructor and begins multiFunction method
#define BS_HEADER_H(structName) \
	structName(); \
	std::string getStructName(); \
	int multiFunction(MFType mfType, std::istream* streamIn, std::ostream* streamOut, int numEntries);

#define BS_HEADER_CPP(structName) \
	structName::structName() { multiFunction(INIT_ALL,NULL,NULL, 0); } \
	std::string structName::getStructName() { return #structName; } \
	int structName::multiFunction(MFType mfType, std::istream* streamIn, std::ostream* streamOut, int numEntries){ \
		int retVal=0; 

// Similar to BS_HEADER, but prepends and checks structId in serialization
#define BS_HEADER_CPP_WITH_ID(structName,structId) \
	BS_HEADER_CPP(structName) \
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