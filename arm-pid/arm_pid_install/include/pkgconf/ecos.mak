ECOS_GLOBAL_CFLAGS = -Wall -Wpointer-arith -Wstrict-prototypes -Wundef -Woverloaded-virtual -Wno-write-strings -mno-thumb-interwork -mcpu=arm7tdmi -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions
ECOS_GLOBAL_LDFLAGS = -mno-thumb-interwork -mcpu=arm7tdmi -g -nostdlib -Wl,--gc-sections -Wl,-static
ECOS_COMMAND_PREFIX = arm-eabi-
