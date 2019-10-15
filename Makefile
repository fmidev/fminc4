# Makefile for Writing Make Files Example

# *****************************************************
# Variables to control Makefile operation

CXX = g++
CXXFLAGS = -std=c++11

# ****************************************************
# Targets needed to bring the executable up to date

lib/libnc4.so: lib/dimension.o lib/group.o lib/variable.o lib/fminc4.o
	$(CXX) $(CXXFLAGS) -shared -o lib/libnc4.so lib/fminc4.o lib/group.o lib/dimension.o lib/variable.o

# The main.o target can be written more simply

lib/fminc4.o: source/fminc4.cpp include/fminc4.h
	$(CXX) $(CXXFLAGS) -I include/ -fPIC -c source/fminc4.cpp -o lib/fminc4.o

lib/group.o: source/group.cpp include/group.h include/dimension.h include/common.h
	$(CXX) $(CXXFLAGS) -I include/ -fPIC -c source/group.cpp -o lib/group.o

lib/dimension.o: source/dimension.cpp include/group.h include/dimension.h include/common.h
	$(CXX) $(CXXFLAGS) -I include/ -fPIC -c source/dimension.cpp -o lib/dimension.o

lib/variable.o: source/variable.cpp include/group.h include/dimension.h include/common.h include/variable.h
	$(CXX) $(CXXFLAGS) -I include/ -fPIC -c source/variable.cpp -o lib/variable.o

