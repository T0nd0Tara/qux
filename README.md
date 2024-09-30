# qux
a sane programming language until we get JAI.

this is my take of what a fun programming language would look like.


## Comments
```c
// this is a comment

/*
    this is a 
    multi line comment
*/
```
qux also have nested comments... 
```
/*
    this is 
    /* nested comment */
    ... still a comment
    // also a comment
*/
```

## Variables
```
// variables are declared with
var_name: type;

// and assigned with
var_name = instance_of_type;

second_var : type = instance_of_type; // inline both
third_var := instance_of_type; // infered type
```
### compile time variables
```
// pretty much the same as regular variables, but with a ":" instead of "="
compile_time_variable :: 4;
```

## Types
```
i8, i16, i32, i64 // signed integer of the said bits
u8, u16, u32, u64 // unsigned integer of the said bits
f32, f64          // floating point of the said bits

bool // a boolean value, takes up 8 bits

int  // an int that enlarges it's size in need
uint // an unsigned int that enlarges it's size in need

[]type             // dynamic array of `type`s
[5]type            // array of `type`s, with size of 5
[type_1, type_2]   // an array of 2 types
[][type_1, type_2] // dynamic array where each cell is an array of 2 types

char // an 8 bit value (differs from u8 by how it's percieved in strings / printing)
str  // well... a string. defined as str :: char[]
dict // a hash map / json / dictionary

*type // a pointer to an instance of type
const *type  // a pointer to a type (the value can be changed but the pointer can't)
* const type // a pointer to a type (the pointer can be changed but the value can't)
const * const type // a pointer to a type (the pointer can't be changed nor the value)

// you can also define your own types
my_type :: int;
```
### Default Conversion
default conversion between types probably creates more bugs than it helps you.
Therefor to auto convert your type you'd have to use the keyword `xx`.
Although it looks like a function (and you can use it just like a function),
it only does the conversion under the hood when needed.
i.e. `a : *int8 = xx ptr` will only be used for readabilty, not by the compiler.

Example
```
a : int = 8;
b : i8 = 10;
c : bool;

c = 7 < a; // Valid, treats 7 as int

c = 7 < b; // Valid, treats 7 as i8

c = a < b; // Error, cannot do arithmetic operation on different types

c = xx a < b; // Valid, converts a to i8 before checking the condition
c = a < xx b; // Valid, converts b to int before checking the condition

```

## Structs
```
// creating a struct
customer :: struct {
    id: int; // no default value
    name: string = ---; // the default value of the type string
    age: i32 = 42; // default value of 42
};

// instanciating
first_customer : customer = {
    age = 3;
    id = 1;
};

first_customer.name == ""; // true

// ERROR: ambiguity on type
second_customer := { 
    age = 3;
    id = 1;
};

// ERROR: no default value for property "id"
second_customer : customer = { 
    age = 3;
};

// OK
second_customer : customer = { 
    age = 3;
    id = ---; // sets 0, as it is the default for type int
};
```

## Functions
```
sum :: (a: int, b: int) -> int {
    return a + b;
};

call_count := 0; // type of int
lambda_inc := (a: int) -> int {
    call_count++;
    return a + 1;
};

// void return type shouldn't be set
inc :: () { call_count++; };

// using output variables as, well, variables
inc :: (a: int) -> out: int {
    out = a + 1;
};

default :: () -> int {
    return ---; // we return the return_type's default value
}
```

### Calling them
```
sum 4, 5;  // returns 9
sum 4 + 5; // ERROR: sum requires 2 values, 1 where given
inc   // doesnt call the function
inc. // calls the function
```

### Default Values
A function's default value can be called only if you specify the name of the variable
i.e.
```
foo :: (a: int, b: int = 0) {
    // ...
};

foo 4;        // OK, will be calling with b = 0;
foo a=4;      // OK, will be calling with b = 0;
foo 4, b = 5; // OK, will be calling with b = 5;
foo 4, 5;     // Error, you have to specify the name of the default arguement
```

### Capture
Functions can also have a specific capture.
This says what the function can access out side of it. If not specified it will capture all.
<BR>
Unlike c++, captures will never copy the value, only use it as a reference
```
a := 0;
b := 1;

foo :: [a]() {
    print a; // Valid
    a++;     // Valid
    print b; // Error, b not in capture
};
bar :: () {
    print a; // Valid
    a++;     // Valid
    print b; // Valid
};
```

### Pipes
The only good thing in OOP, is that some times there is code like
```python
is_title = "random string".capitalize().split()[0].istitle()
```
We concatinate function because the last one return a type we know of.
<BR>
This is much more readable then something in c for example
```c
bool is_title = is_title(split(capitalize("random string"))[0])
```
suddenly to logic is backwords, we have to read from the middle of the line and go back, to understand the logic.
<BR>
So what can we do? PIPES!
```
is_title := "random string" . capitalize . split . first . is_title;
```
The value the previous statement generated is parsed as the **first** argument for that function.
<BR>
If we dont want it to be the first argument, we can use `$`
```
is_title := "random string" . capitalize . split . $[0] . is_title;
```

**NOTICE:** the last function (`is_title`) takes one argument, therefore id doesn't need a `.` after it.
<BR>
i.e. it translates to `is_title the_result_of_the_last_pipe`.

## Error Handling
```
// returns an int and an error sometimes of type string
divide :: (a: int, b: int) -> int !str { 
    if b == 0 return --- !"You can't divide by zero";
    return a / b;
};
```

**NOTICE:** We seperate normal return arguements from the errors.
<BR>
This is useful for handling.

lets say we use that function
```
using_divide :: () -> int {
    a : int = // getting the value somewhere in the function
    b : int = // getting the value somewhere in the function (can be 0)
    
    return divide a, b !! 0; // return 0 on error
};
```

This syntax means we can concatinate multiple function where each returns an error
```
value := func1 val
    !! return func2 $ 
    !! return func3 !$;
```

where `!$` is the error of the previous function.
<BR>
and  `$` is the result of the previous function.

which is much less verbose than go for example
```go
value, err := func1(val);
if (err != nil) {
    _, err2 := func2(value);
    if (err2 != nil) {
        return func3(err2);
    }
}
```

## Control Flow
### Loops
you might think that because of the pipes, there is no loops in this language, but you can't be more wrong!
Recursion although available in the language, is just as confusing as in any other language

We don't like to be confused, so we do use functions
```
// for loop
for "some string" {
    print it; // it is the current value of the last loop
}

for 1..10 {
    print it;
}

// inlined
for 1..10 print it;

// setting a variable to the current it
for i 1..10 { 
    print i;
}

// setting a variable to the current it
for i:i8 1..10 { 
    print i;
}


// while loop
x := 0;
while true {
    if x == 5 break;
    x++;
}

while x > 5 {
    if x == 7 skip; // skip is much better wording than continue
    x = rand.;
    print x;
}

// do while
do {
    a := rand.;
} while a < 5; // a can be seen in the  while statement as it is part of the loop
```

#### Breaking / skiping nested loops
Other programming languages typically let you use `goto` to exit nested loops. But using `goto` usually makes spaghetti code
that's why it is not implemented.
<BR>
What youd want to use instead is
```
for y 0..10 {
    for x 0..10 {
        if cell_is_problematic x, y
            break y; // this tells the compiler to break the loop with variable y. also works with the skip keyword
    }
}
```

### Turnery Operator
if an `if` statement is used as a turnery operator. i.e. a regular if statement, but returns the last statement as a value
```
a := if rand. < 5 10; else 0; // Valid

a := if rand. < 5 10; else ---; // Valid
a := if rand. < 5 ---; else ---; // Error: not enough information for what "a" is
a := if rand. < 5 10; // Error: no default value for a
```

### Switch Statements
switch statements don't fallthrough to the next case.

To do so, you need to explicitly state `fallthrough` at the end of the scope
```
a := if x == {
    case 0; 0;
    case 5; 1;
    case 10; 2;
    case; 3; // default
};

b := if x == {
    case 0; 0;
    case 5; 1;
    case 10; 2;
} else 3; // default (possible but not best practice)

// doesn't have to be equal
grade := if score <= {
    case 60;  "F";
    case 70;  "D";
    case 80;  "C";
    case 90;  "B";
    case 100; "A";
    case; "A+";
};
```

### Defer
defer does the same as every other language. i.e. called when exiting the scope
```
// prints: 1 2
foo :: () {
    defer print 2
    print 1
};
```
usefull for grouping simmilar lines together.
For example
```
write_to_file :: (file_name: str) {
    file := open file_name;
    defer close file;
    
    // do stuff with the file
};
```

## Compilation Time Statements
every command that run in the compilation has a `#` infront of it
here's a list of all of them
```
#include file_name; // assures the exported variables of the file, are seen from this one
namespace_name :: #include file_name; // same as the previous, but inside a local namespace

#extract struct_name; // unpacks all of the struct_name's variables where it's put

#complete // exhoustive checking
```

## Exporting
you can export any thing pretty much, as long as it is accessible at the file level (i.e. no exporting variables inside
functions)
```
foo :: () -> int {
    // ...
};

// makes foo accessible to other files
export foo;

// also accessible from other files
export bar :: int;

// only accessible inside the file
baz :: 5;

boop :: () {
  export a :: 0; // Error: can't export variables inside scope
};

bap :: struct {
  export a :: 0; // Error: can't export variables inside scope
};
```

## Flow
### Import Loop
Probably the most stupid feature in popular languages are the _import loops_

let's say we have 2 files: file1, file2


file1 uses exported variables from file2.
<BR>
and file2 uses exported variables from file1.

this wont create an import loop. how? simple!
<BR>
when the compiler compiles the program it has the file with the main function.
<BR>
it adds it to the list of "seen files". and does the same for each file imported
<BR>
if we want to import a file that is already in that list, we just ignore it -> as we already read it

This is the intuitive way C++'s `#pragma once` should work, but doesn't

so each _qux_ program actually compiles to one big file


### Reading order
Just like Java, you can call a function that is not yet defined or even declared

In qux it works for variables too.

The compiler believes you he'd find a function with the appropriate stub, and when it finishes reading all the files it
will raise an error if it didn't find any.

## Heap Allocation
all we talked about (except dynamic arrays), happens on the stack.
<BR>
well what happens if we want something on the heap? the `new` keyword assignes whatever you said to it on the heap.
<BR>
It will return a pointer of that data
```
a := 5; // stack allocation

b := new 5; // heap allocation

c : int = new 5; // Error: type *int cannot be assigned to type int

d : *my_struct = new {
    var1 = 1;
    var2 = 2;
};

```

This is all allocated in reference to the last context in the context stack
<BR>
A context is a pre-defined struct that contains nothing at default but can be overriden

For example
```
my_context : context = ---;
push_context my_context {
    function_that_allocates_memory_on_the_heap.;
}

// same as
push_context --- {
    function_that_allocates_memory_on_the_heap.;
}

// same as
push_context {
    function_that_allocates_memory_on_the_heap.;
}
```
When the context is poped off (at the end of the `push_context` brackets),
all things allocated on that context would be
deleted.

When the program starts there is a base context so that you could allocate some heap memory. But this context only
pop off when the program exits.
<BR>
i.e. whatever is allocated on the base context cannot be deleted during the program.

### Overriding the Context
The program will fail to compile if there are multiple defenitions of the context struct
```
context :: struct {
    // whatever you want...
    // Can be a logger, 
    // The current size of what was allocated on it
};
```
TODO: what happens if i use a library that have a different context than mine? It is currently an unreasolved problem.
      maybe this language shouldn't allow to override the struct at all, maybe a struct is not needed...

### Using the Current Context
What if you want to use the current context? there is a keyword `current_context` that reference the current context

For example
```
context :: struct {
    count := 0;
};

foo :: () {
    current_context.count = 5;
};

current_context.count++;
print current_context.count; // prints 1

push_context --- {
    current_context.count++;
    print current_context.count; // prints 1
    foo.;
    print current_context.count; // prints 5
    
}

current_context.count++;
print current_context.count; // prints 2
```

