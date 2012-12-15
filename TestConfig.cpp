#include "TestConfig.h"
#include <iostream>

using namespace std;

int main(){
	TestConfig tc, tc2, tc3;

	try{
		tc.readString("s.i=999");

		tc.opt1 = 1;

		tc.jjj=6;

		tc.set("jjj","8");

		tc.readString("jjj=12");
		tc.readString("jjj=0xBAD");

		tc.set("k","[5,4,3,2,1]");

		tc.b=true;
		tc.readString("b=false");
		tc.readString("b=true");
		tc.readString("b=f");
		tc.readString("b=TrUE");
		tc.readString("b=0");

		tc.readFile("file.txt");

		tc.intSet.insert(100);
		tc.intSet.insert(200);

		tc.map["hello"] = 789;
		tc.map["goodbye"] = 987;

		tc.pair = make_pair(123, "test test");
		tc.pair2 = make_pair("test2 test", 1.1f);

		tc.writeToStream(cout);

		string str = tc.toString();
		tc2.readString(str);
		if(tc!=tc2) throw runtime_error("Error: tc!=tc2");

		static const int STRSIZE = 1000;
		char str2[STRSIZE];
		int size = tc.writeToString(str2,STRSIZE);
		tc3.readString(str2, size);
		if(tc!=tc3) throw runtime_error("Error: tc!=tc3");

	}catch(exception& e){
		cout<<e.what()<<endl;
	}

	#ifndef __GNUC__ 
	cout << "Press enter to exit.\n";
	getchar();
	#endif
}
