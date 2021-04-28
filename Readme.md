
SIMPL - Simple Intuative Micro Programming Language
===

A simple language, with not much functionality. More of a tool for learning, than anything else.
simpl has support for comments, variables, assignment, functions, and loops.

Usage
---

You can use SIMPL in your C++ application (requires C++17 or higher) and create bindings.
1) Add SIMPL to your include path
3) `#include <simpl/simpl.h>`
12) Do some other stuff...

Examples
---

 ```
    ### Comments ###
    # this is a comment
    
    ### Features ###
    
    ## Equations
    1 + 3;

    ## Variables
    
    # Declare a variable
    let hw = "hello world!";
    println(hw);
    
    # I have loose typing so you can assign a number to a string, and vice versa
    hw = 4+4;
    println(hw);
    
    # quack-quack
    hw = "goodbye " + 4;
    println(hw);
    
    ## Loops
    let i=10;
    println(i);
    while(i>0)
    {
        println("in a while crocodile");
        i = i-1;
    }
    
    # I don't have for loops yet
    # for(let i=0; i<5; ++i); 
    
    ## Functions
    
    # I can define a function
    def foo(a,b)  {
        println(a);
        println(b);
        return "foo";
    }
    # I can call a function
    println(foo("test",5));
    
    # I even supports recursion
    def recursive(i)  {
        println(i);
        if(i>0) {
            println("i am recursive");
            recursive(i-1);
        }
    }
    
    recursive(5);

    ## Arrays

    let arr = new [];
    let arr = new ["foo", 1, new {}, 4*4];

    ## Structures

    let kh = new { name="king", suit="hearts" };
    
    ### Future Work ###
    
    ## Arrays   
    # arr[0]
    
    ## Structures
    # kh.name = "queen";
    # println(kh.name);

    ## Multi-methods

    #def multi_method(a [is string], b [is number]) {
        println("m(string,number)");
    }

    #def multi_method(a is number, b is number) {
        println("m(number,number)");
    }

    # let foo = [ "hello", "world" ];
    # let bar = ["hello", 10];

    #multi_method(foo...);
    # prints "m(string,number)"
    #multi_method(bar...)
    # prints "m(number,number)"

    


 ```

 ### References

 https://ruslanspivak.com/lsbasi-part18/
 https://craftinginterpreters.com/
 https://github.com/ChaiScript