# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/adrian/_ETSIT-UPM/BECA-SimAntares/ecos/nuevo/nuevoecc_install
export COMMAND_PREFIX := arm-eabi-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/adrian/_ETSIT-UPM/BECA-SimAntares/ecos/ecos-3.0/packages
PACKAGE := hal/arm/pid/v3_0
OBJECT_PREFIX := hal_arm_pid
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -mno-thumb-interwork -mcpu=arm7tdmi -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions
LDFLAGS := -mno-thumb-interwork -mcpu=arm7tdmi -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := 

build: headers libtarget.a.stamp

LIBRARY := libtarget.a
COMPILE := src/hal_diag.c src/pid_misc.c
OBJECTS := $(COMPILE:.cxx=.o.d)
OBJECTS := $(OBJECTS:.cpp=.o.d)
OBJECTS := $(OBJECTS:.c=.o.d)
OBJECTS := $(OBJECTS:.S=.o.d)

$(LIBRARY).stamp: $(OBJECTS)
	$(AR) rcs $(PREFIX)/lib/$(@:.stamp=) $(foreach obj,$?,$(if $(obj:%.o=),$(dir $(obj))$(OBJECT_PREFIX)_$(notdir $(obj:.o.d=.o)),$(obj)))
	@cat $(foreach obj,$^,$(obj:.o=.o.d)) > $(@:.stamp=.deps)
	@touch $@

$(PREFIX)/runsid: $(wildcard $(PREFIX)/include/pkgconf/hal_arm_pid.h)
	@echo "arm-elf-sid CYGTST_SID_FLAGS \$$@" > $(notdir $@).tmp
	$(CC) -E -P -c -xc -undef -imacros $< -o $@ $(notdir $@).tmp
	@rm $(notdir $@).tmp
	

headers: mlt_headers $(PREFIX)/include/cyg/hal/hal_cache.h $(PREFIX)/include/cyg/hal/hal_platform_setup.h $(PREFIX)/include/cyg/hal/hal_diag.h $(PREFIX)/include/cyg/hal/plf_io.h $(PREFIX)/include/cyg/hal/plf_stub.h $(PREFIX)/include/cyg/hal/hal_platform_ints.h

$(PREFIX)/include/cyg/hal/hal_cache.h: $(REPOSITORY)/$(PACKAGE)/include/hal_cache.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/hal_platform_setup.h: $(REPOSITORY)/$(PACKAGE)/include/hal_platform_setup.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/hal_diag.h: $(REPOSITORY)/$(PACKAGE)/include/hal_diag.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/plf_io.h: $(REPOSITORY)/$(PACKAGE)/include/plf_io.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/plf_stub.h: $(REPOSITORY)/$(PACKAGE)/include/plf_stub.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

$(PREFIX)/include/cyg/hal/hal_platform_ints.h: $(REPOSITORY)/$(PACKAGE)/include/hal_platform_ints.h
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

