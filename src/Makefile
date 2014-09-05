APP=dpu
config_tool=config_tool

CC=gcc
INC=-Iclib/include -Iinclude
LIBS = -Lclib/lib -lclib -static
CFLAGS += -g -ggdb3 -O0 -fno-inline -pthread -Wall $(INC) -static

dpu_core_obj = main.o operation.o icmp.o dns.o utils.o server.o
config_tool_obj = config_tool.o

dpu_core_dep = $(dpu_core_obj:.o=.d)
config_tool_dep = $(config_tool_obj:.o=.d)

.PHONY: all clean dep dpu config_tool

all: dep dpu config_tool

-include $(dpu_core_dep) $(config_tool_dep)

$(dpu_core_dep):%.d:%.c 
	@echo [DEP] $@;\
$(CC) -MM $(INC) $< > $@

$(config_tool_dep):%.d:%.c 
	@echo [DEP] $@;\
$(CC) -MM $< $(INC) > $@

LIB:
	@echo [$@];\
	$(MAKE) -C clib/src

LIB_CLEAN:
	@echo [$@];\
cd clib/src;\
make clean;\
cd -

dep: $(dpu_core_dep) $(config_tool_dep)

dpu: LIB $(dpu_core_obj)
	@echo [LD] $@;\
	$(CC) $(CFLAGS) $(dpu_core_obj) -o $(APP) $(LIBS)

config_tool: $(config_tool_obj)
	@echo [LD] $@;\
	$(CC) $(CFLAGS) $^ -o $(config_tool) $(LIBS)

$(dpu_core_obj) $(config_tool_obj) :%.o:%.c
	@echo [CC] $@;\
$(CC) -c $(CFLAGS) $< -o $@

clean: LIB_CLEAN
	-rm -rf *.o $(APP) $(config_tool) *.d
