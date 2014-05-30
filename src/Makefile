APP=dpu

CC=gcc
CFLAGS += -g -ggdb3 -O0 -fno-inline -Iinclude -pthread -Wall

objects = main.o operation.o icmp.o

.PHONY: all clean
all: $(objects)
	$(CC) $(CFLAGS) $^ -o $(APP)
$(objects):%.o:%.c
	@echo [CC] $@;\
$(CC) -c $(CFLAGS) $< -o $@


clean:
	rm -rf *.o $(APP)
