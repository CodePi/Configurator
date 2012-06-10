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

#include "BinSerializer.h"
#include <sstream>
#include <fstream>
#include <stdexcept>

using namespace std;

////////////////////////////////////////////////////////
// Helper class for wrapping a char* without copying
// This allows use of a c-style array a streambuf
// The streambuf can then be used in a istream or ostream

class StreambufWrapper : public streambuf {
public:
	StreambufWrapper(char* s, std::size_t n) {
		setg(s, s, s + n);  // set up as input buffer
		setp(s, s + n);     // set up as output buffer
	}
};

////////////////////////////////////////////////////////
// BinSerializer::serialize methods

void BinSerializer::serialize(ostream& os){
	// output number of entries
	int numEntries = multiFunction(GET_NUM_ENTRIES, NULL, NULL, 0);
	os.write((char*)&numEntries,sizeof(numEntries));
	if(!os) throwError("serialize: Error writing numEntries");

	// serialize struct
	multiFunction(SERIALIZE, NULL, &os, 0);
}

void BinSerializer::serialize(vector<char>& blob){
	// serialize to blob
	int size = getSerializeSize();
	blob.resize(size);                    // alloc enough space for serialization
	StreambufWrapper sb(&blob[0], size);  // use blob as streambuf
	ostream os(&sb);                      // wrap ostream around blob
	serialize(os);                        // serialize to streambuf
}

int BinSerializer::serialize(char* blob, int bufferSize){
	// serialize to blob
	int size = getSerializeSize();
	if(size > bufferSize) throwError("serialize: bufferSize too small");
	StreambufWrapper sb(blob, bufferSize); // use blob as streambuf
	ostream os(&sb);                       // wrap ostream around blob
	serialize(os);                         // serialize to streambuf
	return size;
}

void BinSerializer::serializeFile(const char* filename){
	// serialize to file
	ofstream fs(filename, ios_base::binary);
	if(!fs) throwError("serializeFile: error opening file: "+string(filename));
	serialize(fs);
}

////////////////////////////////////////////////////////
// BinSerializer::deserialize methods

void BinSerializer::deserialize(istream& is){
	int structNumEntries = multiFunction(GET_NUM_ENTRIES, NULL, NULL, 0);
		
	// read number of entries from stream and check
	int numEntries=0;
	is.read((char*)&numEntries,sizeof(numEntries));
	if(!is) throwError("deserialize: error reading numEntries");
	if(numEntries<1 || numEntries>structNumEntries) {
		stringstream err;
		err << "deserialize: invalid numEntries: " << numEntries;
		throwError(err.str());
	}

	// deserialize stream into object
	multiFunction(DESERIALIZE, &is, NULL, numEntries);
}

void BinSerializer::deserialize(vector<char>& blob){
	deserialize(&blob[0], blob.size());
}

void BinSerializer::deserialize(const char* blob, int bufferSize){
	// wrap blob as streambuf and deserialize
	StreambufWrapper sb((char*)blob,bufferSize); // use blob as streambuf
	istream is(&sb);                             // wrap sb in istream
	deserialize(is);                             // deserialize from streambuf
}

void BinSerializer::deserializeFile(const char* filename){
	ifstream fs(filename, ios_base::binary);
	if(!fs) throwError("deserializeFile: error opening file: "+string(filename));
	deserialize(fs);
}

///////////////////////////////////////////////////////
// BinSerializer::getSerializeSize methods

int BinSerializer::getSerializeSize(){
	int size = sizeof(int);   //size of numEntries;
	return size + multiFunction(GET_SIZE, NULL, NULL, 0);
}

////////////////////////////////////////////////////////
// BinSerializer::dumpText methods

void BinSerializer::dumpText(std::ostream& os){
	multiFunction(DUMP_TEXT, NULL, &os, 0);
}

///////////////////////////////////////////////////////
// BinSerializer::serializeHelper methods

void BinSerializer::serializeHelper(std::string& varName, std::ostream& streamOut){
	int size = varName.size();
	streamOut.write((char*)&size, sizeof(size));   // output size to stream
	streamOut.write(varName.c_str(), size); // output string to stream
	if(!streamOut) throwError("serializeHelper: string write error");
}

// Nested class serializer
void BinSerializer::serializeHelper(BinSerializer& varName, std::ostream& streamOut){
	varName.serialize(streamOut);
}

///////////////////////////////////////////////////////
// BinSerializer::deserializeHelper methods

// String deserializer
void BinSerializer::deserializeHelper(std::string& varName, std::istream& streamIn){
	int size;
	streamIn.read((char*)&size, sizeof(size));   // output size to stream
	if(!streamIn) throwError("deserializeHelper: string size read error");
	varName.resize(size);
	streamIn.read((char*)varName.c_str(), size); // output string to stream
	if(!streamIn) throwError("deserializeHelper: string read error");
}

// Nested class deserializer
void BinSerializer::deserializeHelper(BinSerializer& varName, std::istream& streamIn){
	varName.deserialize(streamIn);
}

///////////////////////////////////////////////////////
// BinSerializer::getSizeHelper methods

// String size calculator
int BinSerializer::getSizeHelper(std::string& varName){
	return sizeof(int) + varName.size();  // int + string size
}

// Nested class size calculator
int BinSerializer::getSizeHelper(BinSerializer& varName){
	return varName.getSerializeSize();
}

////////////////////////////////////////////////////////

void BinSerializer::throwError(std::string error){
	throw std::runtime_error(getStructName()+": "+error);
}
