CPPUTEST_LOC=/cygdrive/e/Programming/cpputest-3.6

CC = gcc
CPP = g++

CFLAGS=-O0 -g3 -c -Wall
LDFLAGS=-L$(CPPUTEST_LOC)/lib

RTOSDIR = .
TESTDIR = test
BUILDDIR = build

EXE = HobbyOS.exe


INCLUDE=inc test/inc $(CPPUTEST_LOC)/include
INC_PARAM=$(foreach d, $(INCLUDE), -I$d)

RTOS_S = $(wildcard $(RTOSDIR)/*.c)
RTOS_O = $(patsubst $(RTOSDIR)/%.c,   $(BUILDDIR)/%.o, $(RTOS_S))

TESTCPP_S = $(wildcard $(TESTDIR)/*.cpp)
TESTCPP_O = $(patsubst $(TESTDIR)/%.cpp, $(BUILDDIR)/$(TESTDIR)/%.o, $(TESTCPP_S))

TESTC_S = $(wildcard $(TESTDIR)/*.c)
TESTC_O = $(patsubst $(TESTDIR)/%.c,   $(BUILDDIR)/$(TESTDIR)/%.o, $(TESTC_S))

.PHONY: test clean

all: dir $(EXE) test

dir:
	mkdir -p $(BUILDDIR)/$(TESTDIR)

$(EXE): $(RTOS_O) $(TESTCPP_O) $(TESTC_O)
	$(CPP) $(LDFLAGS) $(RTOS_O) $(TESTCPP_O) $(TESTC_O) -o $(BUILDDIR)/$@ -lCppUTest -lCppUTestExt

$(RTOS_O): $(BUILDDIR)/%.o : $(RTOSDIR)/%.c
	$(CC) $(INC_PARAM) $(CFLAGS) $< -o $@
	
$(TESTCPP_O): $(BUILDDIR)/$(TESTDIR)/%.o : $(TESTDIR)/%.cpp
	$(CPP) $(INC_PARAM) $(CFLAGS) $< -o $@
	
$(TESTC_O):  $(BUILDDIR)/$(TESTDIR)/%.o : $(TESTDIR)/%.c
	$(CC) $(INC_PARAM) $(CFLAGS) $< -o $@

clean:
	rm -rf build
	
test:
	./build/HobbyOS.exe