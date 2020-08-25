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
#include <stdexcept>

namespace codepi {

template <typename T>
class Optional{
public:
  // object starts empty
  Optional() = default;

  // makes a copy of rhs.
  template <typename U>
  Optional(U&& rhs){
    *this = std::forward<U>(rhs); // call assignment operator
  }

  // deallocates if necessary
  ~Optional() { unset(); }

  // returns true if allocated
  bool isSet() const { return mpVal!=nullptr; }

  // sets to empty
  void unset(){
    delete mpVal;
    mpVal = nullptr;
  }

  // returns reference.  Allocates if necessary.
  T& get(){
    if(!mpVal) mpVal = new T;
    return *mpVal;
  }

  // returns const reference.  Throws if empty.
  const T& get() const {
    if(mpVal == nullptr) throw std::runtime_error("taking ref of empty const Optional");
    return *mpVal;
  }

  // returns reference.  Allocates if necessary.
  operator T&(){
    return get();
  }

  // returns const reference.  Throws if empty.
  operator const T&() const {
    return get();
  }

  // assigns value.  Allocates if necessary
  Optional<T>& operator=(const T& rhs){
    if(mpVal!=&rhs) get() = rhs;
    return *this;
  }

  // copies value.  Allocates if necessary
  Optional<T>& operator=(const Optional<T>& rhs){
    if(this == &rhs) ;             // if self assignment, do nothing
    else if(!rhs.isSet()) unset(); // if rhs empty, empty lhs
    else *this = *rhs.mpVal;       // else copy contents (call T assignment operator)
    return *this;
  }

  // assigns value.  Allocates if necessary
  Optional<T>& operator=(T&& rhs){
    if(mpVal!=&rhs) get() = std::move(rhs);
    return *this;
  }

  // moves value.
  Optional<T>& operator=(Optional<T>&& rhs){
    if(this != &rhs) {       // if self assignment, do nothing
      unset();               // unset current value
      mpVal = rhs.mpVal;     // move from rhs to lhs
      rhs.mpVal = nullptr;      // clear rhs
    }
    return *this;
  }

  // allows access to instance methods and variables if payload is struct/class
  T* operator->() { return &get(); }
  const T* operator->() const { return &get(); }

private:
  T* mpVal = nullptr;   // payload
 };

} // end namespace codepi
