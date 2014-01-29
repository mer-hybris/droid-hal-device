# Prepare a set of pre-built RPMs for OBS upload
# Copyright (c) 2014 Jolla Ltd.
# Contact: Thomas Perl <thomas.perl@jolla.com>

DEVICE ?=
INDIR ?= ../../RPMS
OUTDIR ?= out_$(DEVICE)

RPM_REGEX := 'droid-hal-\([^-]*\)-\([^-]*\)-\(.*\)\.\([^.]*\)\.rpm'

# Sort makes sure droid-hal-device-x.y.z-w.arch.rpm is sorted before sub-packages
BASE_RPM := $(notdir $(firstword $(sort $(wildcard $(INDIR)/droid-hal-*-[0-9.]*.*-*.rpm))))

ifeq ($(BASE_RPM),)
  $(error No droid-hal-*.rpm found in $(INDIR))
endif

RPM_DEVICE := $(shell echo $(BASE_RPM) | sed -e s/$(RPM_REGEX)/'\1'/)
RPM_VERSION := $(shell echo $(BASE_RPM) | sed -e s/$(RPM_REGEX)/'\2'/)
RPM_RELEASE := $(shell echo $(BASE_RPM) | sed -e s/$(RPM_REGEX)/'\3'/)
RPM_ARCH := $(shell echo $(BASE_RPM) | sed -e s/$(RPM_REGEX)/'\4'/)

ifeq ($(DEVICE),)
  ifeq ($(RPM_DEVICE),)
    $(error "Could not determine DEVICE, please set manually")
  else
    $(warning "Determining DEVICE from $(BASE_RPM) -> $(RPM_DEVICE)")
    DEVICE = $(RPM_DEVICE)
  endif
endif

TEMPLATE_SED += -e 's/%DEVICE%/$(DEVICE)/g'
TEMPLATE_SED += -e 's/%ARCH%/$(RPM_ARCH)/g'
TEMPLATE_SED += -e 's/%VERSION%/$(RPM_VERSION)/g'
TEMPLATE_SED += -e 's/%RELEASE%/$(RPM_RELEASE)/g'

RPMS := $(wildcard $(INDIR)/droid-hal-$(DEVICE)-*.rpm)

TODO += $(OUTDIR)
TODO += $(OUTDIR)/build.script
TODO += $(OUTDIR)/droid-hal-$(DEVICE).spec
TODO += $(patsubst $(INDIR)%,$(OUTDIR)%,$(RPMS))

all: $(TODO)

$(OUTDIR):
	mkdir -p $@

$(OUTDIR)/build.script: build.script.in
	sed $(TEMPLATE_SED) $^ >$@

$(OUTDIR)/droid-hal-$(DEVICE).spec: droid-hal-device.spec.in
	sed $(TEMPLATE_SED) $^ >$@

$(OUTDIR)/%.rpm: $(INDIR)/%.rpm
	cp $^ $@

clean:
	rm -f $(OUTDIR)/build.script
	rm -f $(OUTDIR)/droid-hal-$(DEVICE).spec
	rm -f $(OUTDIR)/droid-hal-$(DEVICE)-*.rpm
	-rm -fd $(OUTDIR)

.DEFAULT: all
.PHONY: all clean
