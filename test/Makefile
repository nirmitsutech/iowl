CFLAGS +=  -I. -Iinclude -I../include -I../
CC=gcc
CXX=g++

CXXFLAGS += -I. $(shell wx-config --cflags) -I../include
LDFLAGS += $(shell wx-config --libs) $(shell wx-config --ldflags)
ACQCLI = acqcli
TARGET = $(ACQCLI)
ACQCLIOBJS=  ../src/acq_proto.o acq_cli.o thread.o
all:   $(ACQCLI) 


%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@
clean:
	rm -f $(TARGET) $(OBJS) *.o *.exe core

	
$(ACQCLI):$(ACQCLIOBJS)
	$(CXX) $(LDFLAGS) $(ACQCLIOBJS) -o $@
