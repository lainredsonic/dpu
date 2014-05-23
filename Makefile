
.PHONY: all clean

all: src
	$(MAKE) -C src

clean: src
	$(MAKE) -C src clean

