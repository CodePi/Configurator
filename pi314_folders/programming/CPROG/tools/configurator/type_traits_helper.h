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

// type_traits is from the c++0x standard, however compiler support is spotty.
// This file gets rid of the dependency on type_traits.
// is_configurator used in conjunction with enable_if allows for proper 
//   compile time template resolution.
// Tested with gcc 4.1, gcc 4.3, vs2005, vs2010.
// :TODO: Replace with real type_traits when support for older compilers
//        is no longer needed.
//        type_traits is currently supported in vs2008 and gcc 4.1 and later

namespace TTHelper{
   template <bool B, class T = void>
   struct enable_if {
      typedef T type;
   };

   template <class T>
   struct enable_if<false, T> {};

   // SFINAE trick to detect if typename T contains typedef is_configurator
   // is_configurator<T>::value resolves to true if T contains 
   //   a typedef named is_configurator.
   template <typename T>
   struct is_configurator {
      // yes and no are guaranteed to have different sizes,
      // specifically sizeof(yes) == 1 and sizeof(no) == 2
      typedef char yes[1];
      typedef char no[2];

      template <typename C>
      static yes& test(typename C::is_configurator*);

      template <typename>
      static no& test(...);

      // if the sizeof the result of calling test<T>(0) is equal to the sizeof(yes),
      // the first overload worked and T has a nested type named is_configurator.
      static const bool value = sizeof(test<T>(0)) == sizeof(yes);
   };

};
