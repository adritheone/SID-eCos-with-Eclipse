# eCos makefile

# This is a generated file - do not edit

export PREFIX := /home/adrian/_ETSIT-UPM/BECA-SimAntares/ecos/arm-pid/arm_pid_install
export COMMAND_PREFIX := arm-eabi-
export CC := $(COMMAND_PREFIX)gcc
export OBJCOPY := $(COMMAND_PREFIX)objcopy
export HOST := UNIX
export AR := $(COMMAND_PREFIX)ar

export REPOSITORY := /home/adrian/_ETSIT-UPM/BECA-SimAntares/ecos/ecos-3.0/packages
PACKAGE := devs/serial/arm/pid/v3_0
OBJECT_PREFIX := devs_serial_arm_pid
CFLAGS := -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -mno-thumb-interwork -mcpu=arm7tdmi -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions
LDFLAGS := -mno-thumb-interwork -mcpu=arm7tdmi -g -nostdlib -Wl,--gc-sections -Wl,-static
VPATH := $(REPOSITORY)/$(PACKAGE)
INCLUDE_PATH := $(INCLUDE_PATH) -I$(PREFIX)/include $(foreach dir,$(VPATH),-I$(dir) -I$(dir)/src -I$(dir)/tests) -I.
MLT := $(wildcard $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.ldi $(REPOSITORY)/$(PACKAGE)/include/pkgconf/mlt*.h)
TESTS := 

build: headers

headers: mlt_headers $(PREFIX)/include/cyg/io/arm_arm7_pid_ser.inl

$(PREFIX)/include/cyg/io/arm_arm7_pid_ser.inl: $(REPOSITORY)/$(PACKAGE)/include/arm_arm7_pid_ser.inl
	@mkdir -p $(dir $@)
	@cp $< $@
	@chmod u+w $@

include $(REPOSITORY)/pkgconf/rules.mak

