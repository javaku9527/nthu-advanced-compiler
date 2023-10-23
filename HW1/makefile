LLVM_CONFIG=/home/javaku/llvm_build/bin/llvm-config

CXX=`$(LLVM_CONFIG) --bindir`/clang
CXXFLAGS=`$(LLVM_CONFIG) --cppflags` -fPIC -fno-rtti
LDFLAGS=`$(LLVM_CONFIG) --ldflags`

all: pass.so test

pass.o: pass.cpp
	$(CXX) -c pass.cpp $(CXXFLAGS) -o pass.o

pass.so: pass.o
	$(CXX) -shared pass.o $(LDFLAGS) -fPIC -o pass.so

test: test1.c test2.c #test3.c
	$(CXX) -S -emit-llvm test1.c -fno-discard-value-names -o test1.ll
	$(CXX) -S -emit-llvm test2.c -fno-discard-value-names -o test2.ll
	#$(CXX) -S -emit-llvm test3.c -fni-discard-value-names -o test3.ll

clean:
	rm -f *.o *.ll *.so
