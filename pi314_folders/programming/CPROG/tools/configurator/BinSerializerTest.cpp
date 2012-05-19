#include "BinSerializer.h"
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

struct TestNestedClass : public BinSerializer{
	int i;
	int j;

	BS_HEADER(TestNestedClass)
		BS_ENTRY_D(i,9)
		BS_ENTRY_D(j,9)
		BS_TAIL
};

struct TestClass : public BinSerializer{
	int i;
	string str;
	vector<int>vi;
	vector<string>vs;
	float f;
	TestNestedClass nc;

	BS_HEADER(TestClass)
		BS_ENTRY_MULTI6(i,str,vi,vs,f,nc)
		BS_TAIL
};

struct TestClassAppended : public BinSerializer{
	int i;
	string str;
	vector<int>vi;
	vector<string>vs;
	float f;
	TestNestedClass nc;
	int additional;

	BS_HEADER(TestClassAppended)
		BS_ENTRY_MULTI6(i,str,vi,vs,f,nc)
		BS_ENTRY(additional)
		BS_TAIL	
};

int main(){

	TestClass tc;
	tc.i = 1;
	tc.f = 1.1f;
	tc.vi.push_back(4);
	tc.vi.push_back(5);
	tc.vi.push_back(6);
	tc.vs.push_back("123");
	tc.vs.push_back("456");
	tc.vs.push_back("789");
	tc.str = "hello";
	tc.nc.i = 5;
	tc.nc.j = 6;
	
	try{

		// test serialization to/from a file
		tc.serializeFile("test.bin");
		TestClass tc2;
		tc2.deserializeFile("test.bin");

		// test serialization to/from a stream
		TestClass tc3;
		stringstream ss;
		tc.serialize(ss);
		tc3.deserialize(ss);

		// test serialization to/from a char vector
		vector<char>vc;
		tc.serialize(vc);
		TestClass tc4;
		tc4.deserialize(vc);

		// test serialization to/from a char*
		int size = tc.getSerializeSize();
		char* buffer = new char[size];
		tc.serialize(buffer,size);
		TestClass tc5;
		tc5.deserialize(buffer,size);
		delete[]buffer;
	
		// test deserialization to a class with more entries
		TestClassAppended tcb;
		tcb.deserialize(vc);

		tc.dumpText(cout);

	}catch(runtime_error& e){
		cout << "runtime_error caught: " << e.what() << endl;
	}

	getchar();

}