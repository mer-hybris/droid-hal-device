# Path to the Android sources
ANDROID_ROOT ?= ..

# List of local tools we need to build
TOOLS := usergroupgen
MKBOOTIMG_MK ?= $(shell pwd)/mkbootimg.mk
IMG2SIMG_MK ?= $(shell pwd)/img2simg.mk
SIMG2IMG_MK ?= $(shell pwd)/simg2img.mk

# Include directories
CFLAGS += -I$(ANDROID_ROOT)/system/core/include/private/

# C99 support
CFLAGS += -std=c99

all: $(TOOLS) image_tools

image_tools:
	make -f $(MKBOOTIMG_MK) -C $(ANDROID_ROOT)/system/core/mkbootimg/
	make -f $(IMG2SIMG_MK) -C $(ANDROID_ROOT)/system/core/libsparse/
	make -f $(SIMG2IMG_MK) -C $(ANDROID_ROOT)/system/core/libsparse/
clean:
	rm -f $(TOOLS) $(ANDROID_ROOT)/system/core/mkbootimg/mkbootimg \
	$(ANDROID_ROOT)/system/core/libsparse/img2simg \
	$(ANDROID_ROOT)/system/core/libsparse/simg2img

.PHONY: all clean
