ifeq ($(filter --no-print-directory,$(MAKEFLAGS)),)
MAKEFLAGS += --no-print-directory
endif
ifeq ($(filter --silent,$(MAKEFLAGS)),)
MAKEFLAGS += --silent
endif

all: sbf

sbf:
	@$(MAKE) -C src

clean:
	@$(MAKE) -C src clean

mrproper:
	@$(MAKE) -C src mrproper

.PHONY: all sbf clean mrproper
