CXX = g++
#CXXFLAGS = -std=c++11 -Wall -Wno-unused-function -g -fprofile-arcs -ftest-coverage -I./unity/src -I../src -DTESTRUNNER
CXXFLAGS = -std=c++11 -Wall -Wno-unused-function -g -I./unity/src -I../src -DTESTRUNNER
LDFLAGS =

unity-dir = unity
unity-repo = https://github.com/ThrowTheSwitch/Unity.git

.PHONY: all clean

all: fetch unity.o runner

fetch:
	@if [ ! -d $(unity-dir) ] ;\
	then \
	  echo "Unity not available, fetching"; \
	  git clone $(unity-repo) $(unity-dir); \
	else \
	  echo "Unity available, not fetching"; \
	fi

unity.o: $(unity-dir)/src/unity.h $(unity-dir)/src/unity_internals.h
	$(CXX) $(CXXFLAGS) -c unity/src/unity.c

String.o: ../src/Packets/String.h
	$(CXX) $(CXXFLAGS) -c ../src/Packets/String.cpp

RemainingLength.o: ../src/Packets/RemainingLength.h
	$(CXX) $(CXXFLAGS) -c ../src/Packets/RemainingLength.cpp

Packet.o: ../src/Packets/Packet.h
	$(CXX) $(CXXFLAGS) -c ../src/Packets/Packet.cpp

Parser.o: ../src/Packets/Parser.h
	$(CXX) $(CXXFLAGS) -c ../src/Packets/Parser.cpp

runner: runner.o unity.o RemainingLength.o String.o Packet.o Parser.o
	$(CXX) $(CXXFLAGS) -o runner runner.o unity.o String.o RemainingLength.o Packet.o Parser.o 

clean:
	@rm -f *.o *.gcov *.gcda *.gcno runner
	@echo "All clean";
