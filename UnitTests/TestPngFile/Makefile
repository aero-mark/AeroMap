# TestPngFile makefile
#

TARGET = TestPngFile

# global common path
GPATH = ../../common

# compiler switches
CFLAGS = -g -Wall -std=c++11 -I../ -I$(GPATH)

# linker switches
LFLAGS = -L$(GPATH)/CppUnitLite -lCppUnitLite -lpng

all: CppUnitLite
	g++ $(CFLAGS) $(TARGET).cpp -o $(TARGET) $(GPATH)/PngFile.cpp $(GPATH)/Logger.cpp $(LFLAGS)

CppUnitLite:
	make $(GPATH)/CppUnitLite

clean:
	-rm a.out
	-rm $(TARGET)

