# BuildIt &nbsp;&nbsp; [![Build Status](https://github.com/BuildIt-lang/buildit/actions/workflows/ci-all-samples.yml/badge.svg)](https://github.com/BuildIt-lang/buildit/actions/workflows/ci-all-samples.yml) 


<p align="center"><img src="https://intimeand.space/images/logo-buildit.png" width="400"></p>
<p align="center">"Every problem in Computer Science can be solved with a level of indirection and every problem in Software Engineering can be solved with a step of code-generation"</p>

[BuildIt](https://buildit.so) is a a framework for rapidly developing high-performance Domain Specific Languages (DSLs) with little to no compiler knowledge. A lightweight type-based multi-stage programming framework in C++ that supports generating expresisons and rich data-dependent control flow like if-then-else conditions and for and while loops using its novel re-execution strategy for a variety of targets.

## Some completed and WIP projects with BuildIt
1. [NetBlocks](https://github.com/BuildIt-lang/net-blocks): A Performant, customizable and modular network stack to specialize protocols and implementations
2. [Easy-GraphIt](https://github.com/BuildIt-lang/easy_graphit): A 2021 LoC GraphIt compiler that generates high-performance GPU code!
3. [Mini-Einsum DSL](https://buildit.so/tryit?sample=einsum): Compiler for Einsum-expressions on N-dimensional dense tensors in 300 LoC (CPU and GPU parallel)

BuildIt is available opensource on GitHub under the MIT license. Many more samples are available in the samples/ directory and the [website](https://buildit.so/tryit)

## Key Features
**1. A portable light-weight multi-stage library that can be used with any standard C++ compiler.**
```
#include <builder/dyn_var.h>
#include <builder/static_var.h>
...
> g++ foo.cpp -lbuildit -I buildit/include/
```

**2. Write expressions and statements with dyn_var<T> variables to generate the same code. Easily port a high-performance library into a compiler by changing types of variables.**
```
// Code written with dyn_var<T>
for (dyn_var<int> i = 0; i < 512; i = i + 1) {
  A[i] = 0;
}
```
// Generates the same code
```
for (int var1 = 0; var1 < 512; var1 = var1 + 1) {
  var0[var1] = 0;
}
```

**3. Use conditions and expressions on static_var<T> to specialize generated code for high-performance.**
```
static_var<int> bound, block_size;
...
// Splitting of loop with known bounds
for (dyn_var<int> i0 = 0; i0 < bound / block_size; i0 = i0 + 1) {
  for (dyn_var<int> i1 = i0 * block_size; i1 < (i0 + 1) * block_size; i1 = i1 + 1) {
    ...
  }
}
// conditionally generate padding loops
if (bound % block_size != 0) {
  for (dyn_var<int> i1 = (bound / block_size) * block_size; i1 < bound; i1 = i1 + 1) {
    ...
  }
}
```

**4. Supports generation of parallel code for CPUs and GPUs with simple annotations**
```
builder::annotate("pragma: omp parallel for");
for (dyn_var<int> i = 0; i < 512; i = i + 1) {
  A[i] = 0;
}
```

## Debugging support
Checkout [D2X](https://buildit.so/d2x) a debugging library that works in tandem with BuildIt and provides debugging support for DSLs. 

## Build and Run

BuildIt is a pure library framework and does not require any special compiler support. To build the library, clone the repository, navigate to the top-level directory and run (please make sure you are using [GNU-make](https://www.gnu.org/software/make/) and not just make) - 

    make
   
For building the library with Debugging support, run 

    make DEBUG=1 
   
To run the samples provided with the library (that also serve as simple test cases), run -

    make run
 
The make system will report the first failing test case if any. 

BuildIt is currently under active development. More features are being added to the framework to make building DSLs easy. If you notice BuildIt not functioning correctly on certain inputs, please create an issue with the code snippet and expected behavior.

 

