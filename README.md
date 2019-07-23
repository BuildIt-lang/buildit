# mirror-cpp [![Build Status](https://api.travis-ci.org/AjayBrahmakshatriya/mirror-cpp.svg?branch=master)](https://travis-ci.org/AjayBrahmakshatriya/mirror-cpp)
Mirror-CPP is a library for bringing reflection into C++ by overloading the basic operators. Mirror-CPP also extracts the control flow in the function by repeated execution and exploration of all branches. 

Mirror-CPP does not require any special compiler support and provides a library for embedding any DSL in C++. 

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

