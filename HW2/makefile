LLVM_CONFIG=/home/javaku/llvm_build/bin/llvm-config

CXX=`$(LLVM_CONFIG) --bindir`/clang
CXXFLAGS=`$(LLVM_CONFIG) --cppflags` -fPIC -fno-rtti
LDFLAGS=`$(LLVM_CONFIG) --ldflags`

all: pass.so test

pass.o: pass.cpp
	$(CXX) -c pass.cpp $(CXXFLAGS) -o pass.o

pass.so: pass.o
	$(CXX) -shared pass.o $(LDFLAGS) -fPIC -o pass.so

test: foo.c icpp.c icpp2.c icpp3.c
	$(CXX) -S -emit-llvm foo.c -fno-discard-value-names -o foo.ll
	$(CXX) -S -emit-llvm icpp.c -fno-discard-value-names -o icpp.ll
	$(CXX) -S -emit-llvm icpp2.c -fno-discard-value-names -o icpp2.ll
	$(CXX) -S -emit-llvm icpp3.c -fno-discard-value-names -o icpp3.ll

clean:
	rm -f *.o *.ll *.so
