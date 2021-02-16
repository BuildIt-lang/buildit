# Build&rarr;It [![Build Status](https://travis-ci.com/BuildIt-lang/buildit.svg?branch=master)](https://travis-ci.com/github/BuildIt-lang/buildit)

<p align="center">"Every problem in Computer Science can be solved with a level of indirection and every problem in Software Engineering can be solved with a step of code-generation"</p>

Build→It is a lightweight<sup>1</sup> type-based<sup>2</sup> multi-stage programming<sup>3</sup> framework in C++. Build→It makes it easy to implement Domain Specific Languages (DSLs) with very little to no knowledge about compilers. Besides extracting expressions and statements using operator overloading, Build→It supports extracting rich data-dependent control flow like if-then-else conditions and for and while loops using its novel re-execution strategy to explore all control-flow paths in the program.

Build→It turns -

```
template <typename BT, typename ET>
dyn_var<int> power_f(BT base, ET exponent) {
  dyn_var<int> res = 1, x = base;
  while (exponent > 1) {
    if (exponent % 2 == 1)
      res = res * x;
    x = x * x;
    exponent = exponent / 2;
  }
  return res * x;
}
...
int power = 15;
context.extract_function_ast(power_f<dyn_var<int>, static_var<int>>, "power_15", power);
...
int base = 5;
context.extract_function_ast(power_f<static_var<int>, dyn_var<int>>, "power_5", base);
...
```

into -

```
int power_15 (int arg0) {
  int var0 = arg0;
  int var1 = 1;
  int var2 = var0;
  var1 = var1 * var2;
  var2 = var2 * var2;
  var1 = var1 * var2;
  var2 = var2 * var2;
  var1 = var1 * var2;
  var2 = var2 * var2;
  int var3 = var1 * var2;
  return var3;
}

int power_5 (int arg1) {
  int var0 = arg1;
  int var1 = 1;
  int var2 = 5;
  while (var0 > 1) {
    if ((var0 % 2) == 1) {
      var1 = var1 * var2;
    }
    var2 = var2 * var2;
    var0 = var0 / 2;
  }
  int var3 = var1 * var2;
  return var3;
}
```

Build→It is available opensource on GitHub under the MIT license. Many more samples are available in the samples/ directory including turning an interpreter for BrainFuck into a compiler using Futamura projections.

1. Build→It uses a purely library based approach and does not require any special compiler modifications making it extremely portable and easy to integrate into existing code bases. Using Build→It is as easy as including a few header files and linking against the Build→It library.

2. Build→It uses the declared types of variables and expressions to decide the binding time. Build→It adds 2 new generic types - static_var<T> and dyn_var<T> which lets the user program with 2 stages. These types can be nested arbitrarily to produce code for more stages.

3. What exactly is multi-stage programming and why it is important for high-performance DSLs is explained in our paper.

## Build and Run

Build→It is a pure library framework and does not require any special compiler support. To build the library, clone the repository, navigate to the top-level directory and run -

   make
   
For building the library with Debugging support, run 

   make DEBUG=1 
   
To run the samples provided with the library (that also serve as simple test cases), run -

   make run
 
The make system will report the first failing test case if any. 

Build→It is currently under active development. More features are being added to the framework to make building DSLs easy. If you notice Build→It not functioning correctly on certain inputs, please create an issue with the code snippet and expected behavior.

 

