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

// Can wrap any type.  Can be used seamlessly in place of the type it wraps.  
// Can be empty.  Automatically allocates space when needed

/* Example usage:
  optional<int> i;            // i starts empty
  cout << i.isSet() << endl;  // returns false
  i=1;                        // allocates i and sets to 1
  cout << i.isSet() << " " << i << endl; // returns true and 1
  foo(i);                     // passes i as int or int&
*/

#pragma once

#include <iostream>

template <typename T>
class Optional{
public:
  // object starts empty
  Optional(): mpVal(NULL){}

  // makes a copy of rhs.  
  Optional(const Optional<T>& rhs){
    mpVal = NULL;
    *this = rhs; // call assignment operator
  }
  
  // initialize value  
  Optional(const T& rhs){
    mpVal = NULL;
    *this = rhs; // call assignment operator
  }

  // construct by move  
  Optional(Optional<T>&& rhs){
    mpVal = NULL;
    *this = move(rhs); // call move assignment operator
  }
  
  // construct by move  
  Optional(T&& rhs){
    mpVal = NULL;
    *this = move(rhs); // call move assignment operator
  }

  // deallocates if necessary
  ~Optional() { unset(); }

  // returns true if allocated
  bool isSet() const { return mpVal!=NULL; }

  // sets to empty
  void unset(){
    delete mpVal;
    mpVal = NULL;
  }

  // returns reference.  Allocates if necessary
  operator T&(){
    if(!mpVal) mpVal = new T;
    return *mpVal;
  }

  // assigns value.  Allocates if necessary
  Optional<T>& operator=(const T& rhs){
    if(!mpVal) mpVal = new T;
    *mpVal = rhs;
	return *this;
  }

  // copies value.  Allocates if necessary
  Optional<T>& operator=(const Optional<T>& rhs){
    if(this == &rhs) ;             // if self assignment, do nothing
    else if(!rhs.isSet()) unset(); // if rhs empty, empty lhs
    else *this = *rhs.mpVal;    // else copy contents (call T assignment operator)
    return *this;
  }

  // assigns value.  Allocates if necessary
  Optional<T>& operator=(T&& rhs){
    if(!mpVal) mpVal = new T;
    *mpVal = move(rhs);
	return *this;
  }

  // moves value. 
  Optional<T>& operator=(Optional<T>&& rhs){
    if(this == &rhs) ;             // if self assignment, do nothing
    else {
		unset();               // unset current value
		mpVal = rhs.mpVal;     // move from rhs to lhs
		rhs.mpVal = NULL;      // clear rhs
	}
    return *this;
  }

  // allows access to instance methods and variables if payload is struct/class
  T* operator->() {
    T& me = (T&)*this;
    return &me;
  }

private:
  T* mpVal;   // payload
};

