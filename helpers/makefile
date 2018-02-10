# Path to the Android sources
ANDROID_ROOT ?= ..

# List of local tools we need to build
TOOLS := usergroupgen

ifdef MKBOOTIMG_MK
MKBOOTIMG := mkbootimg
MKBOOTIMG_CLEAN := $(ANDROID_ROOT)/system/core/mkbootimg/mkbootimg
endif

IMG2SIMG_MK ?= $(shell pwd)/img2simg.mk
SIMG2IMG_MK ?= $(shell pwd)/simg2img.mk

# Include directories
CFLAGS += -I$(ANDROID_ROOT)/system/core/include/ -I$(ANDROID_ROOT)/

# C99 support
CFLAGS += -std=c99

all: $(TOOLS) $(MKBOOTIMG) image_tools

mkbootimg:
	make -f $(MKBOOTIMG_MK) -C $(ANDROID_ROOT)/system/core/mkbootimg/

image_tools:
	make -f $(IMG2SIMG_MK) -C $(ANDROID_ROOT)/system/core/libsparse/
	make -f $(SIMG2IMG_MK) -C $(ANDROID_ROOT)/system/core/libsparse/
clean:
	rm -f $(TOOLS) $(MKBOOTIMG_CLEAN) \
	$(ANDROID_ROOT)/system/core/libsparse/img2simg \
	$(ANDROID_ROOT)/system/core/libsparse/simg2img

.PHONY: all clean
