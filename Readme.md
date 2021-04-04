
SIMPL - Simple Intuative Micro Programming Langugage
===

A simple language, with not much functionality. More of a tool for learning, than anything else.
simpl has support for comments, variables, assignment, functions, and loops.

Usage
---
*Note:* - as of now, the library is not consumable. So these steps are bogus, and won't work.

You can use SIMPL in your C++ application (requires C++17 or higher) and create bindings.
1) Add SIMPL to your include path
3) `#include <simpl.h>`

Examples
---

 ```
 # The one and only, Hello World

 print("hello world");

 ```


 ```
 # This is a loop.
 let i=10;
 while(i>0) {
    println("I'm in a loop.");
    i=i-1;
 }


 ```