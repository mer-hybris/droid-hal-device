TOOLS := apply-permissions usergroupgen

CFLAGS += -std=c99 -I../system/core/include/private/

all: $(TOOLS)

clean:
	rm -f $(TOOLS)

.PHONY: all clean
