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

int  // an int that enlarges it's size in need
uint // an unsigned int that enlarges it's size in need

type[]             // array of `type`s
[type_1, type_2]   // an array of 2 types
[type_1, type_2][] // array where each cell is an array of 2 types

char // an 8 bit value (differs from u8 by how it's percieved in strings)
str  // well... a string. defined as str :: char[]
dict // a hash map / json / dictionary

*type // a pointer to an instance of type
const *type  // a pointer to a type (the value can be changed but the pointer can't)
* const type // a pointer to a type (the pointer can be changed but the value can't)
const * const type // a pointer to a type (the pointer can be changed nor the value)

// you can also define your own types
my_type :: int;
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
    
    result := divide a, b ! return 0;
    return result;
};
```

this syntax means we can concatinate multiple function where each returns an error
```
value := func1 val
    ! return func2 $ 
    ! return func3 !$;
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
