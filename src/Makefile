APP=dpu

CC=gcc
CFLAGS += -g -ggdb3 -O0 -fno-inline -Iinclude -pthread -Wall -Iclib/include -static
LIBS = -Lclib/lib -lclib

#objects = main.o operation.o icmp.o dns.o utils.o clib/lib/libclib.a
objects = main.o operation.o icmp.o dns.o utils.o

.PHONY: all clean
all: $(objects)
	$(CC) $(CFLAGS) $^ -o $(APP) $(LIBS)

$(objects):%.o:%.c
	@echo [CC] $@;\
$(CC) -c $(CFLAGS) $< -o $@


clean:
	rm -rf *.o $(APP)
