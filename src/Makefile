ifeq ($(filter --no-print-directory,$(MAKEFLAGS)),)
MAKEFLAGS += --no-print-directory
endif
ifeq ($(filter --silent,$(MAKEFLAGS)),)
MAKEFLAGS += --silent
endif

include config.mk

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)

-include $(DEP)

all: sbf

%.o: %.c
	@printf "  %-7s %s\n" "CC" "$(SUBDIR)/$@"
	@$(CC) $(CFLAGS) -c $< -o $@

sbf: $(OBJ)
	@printf "  %-7s %s\n" "LD" "$(SUBDIR)/sbf"
	@$(LD) $(LDFLAGS) -o $@ $^

clean:
	@printf "  %-7s %s\n" "CLEAN" "$(SUBDIR)/sbf $(SUBDIR)/*.o $(SUBDIR)/*.d"
	@rm -rf sbf *.o *.d

mrproper: clean

.PHONY: all clean mrproper
	
