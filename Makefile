C       = g++
LIB     =  -I/usr/csshare/pkgs/csim_cpp-19.0/lib
CSIM    = /usr/csshare/pkgs/csim_cpp-19.0/lib/csim.cpp.a
RM      = /bin/rm -f
CFLAG   = -DCPP -DGPP
 
INPUT1 = part1.csim.c
TARGET = csim

all: part1

part1: part1.csim.c
	$(C) $(CFLAG) $(LIB) $(INPUT1) $(CSIM) -lm -o $(TARGET)

clean:
	$(RM) *.o *.output core *~ *# $(TARGET)
