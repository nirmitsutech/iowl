SRCS := acq_log.c acq_handle.c acq_daemon.c acq_hw.c acq_manage.c acq_device.c acq_config.c acqd.c
OBJS=$(SRCS:.c=.o)
CC=gcc
CFLAGS += -DDAEMONIZE -I. -I../include -I..
TARGET := acqd
ACQSERV= acqserv
ACQCLI = acqcli
ACQSERVOBJS=  acq_proto.o acq_thread.o acqserv.o
all: $(TARGET)  $(ACQSERV) 

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)  $(LIBS)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
clean:
	rm -f $(TARGET) $(ACQSERV) $(OBJS) *.o *.exe core 

test:station.o
	$(CC) -o $@ $^ -lxml2
	
$(ACQSERV):$(ACQSERVOBJS)
	$(CC) $(ACQSERVOBJS) -lpthread -o $@
