# BuildIt [![Build Status](https://api.travis-ci.org/AjayBrahmakshatriya/buildit.svg?branch=master)](https://travis-ci.org/AjayBrahmakshatriya/buildit)

BuildIt is a type based library framework for multi-stage imperative programming. BuildIt supports programmatically generating ASTs for an embedded language in C++. BuildIt extracts expressions and statements by overloading the basic operators. BuildIt also extracts the control flow by repeated execution and exploration of all branches. BuildIt uses static tags to detect loops and unroll static loops. 

BuildIt is a pure library framework and does not require any special compiler support. 
To build the library, clone the repository, navigate to the top-level directory and run - 

```
make
```

For building the library with debug support, run -

```
make DEBUG=1
```

To run the samples provided with the library (that also serve as simple test cases), run - 

```
make run
```

The make system will report the first failing test case if any. 
