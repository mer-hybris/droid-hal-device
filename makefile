# Path to the Android sources
ANDROID_ROOT ?= ..

# List of local tools we need to build
TOOLS := apply-permissions usergroupgen

# Include directories
CFLAGS += -I$(ANDROID_ROOT)/system/core/include/private/

# C99 support
CFLAGS += -std=c99

all: $(TOOLS)

clean:
	rm -f $(TOOLS)

.PHONY: all clean
