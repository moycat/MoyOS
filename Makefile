# Put all the source files here
SRCS = src/main.c src/moyos.c src/port.c src/user_main.c

# Binary will be generated with this name (.elf, etc)
PROJ_NAME = moyos

# Binary will be generated here
OUTPUT_PATH = build/

# Normally you shouldn't need to change anything below this line
################################################################

CMSIS = ./CMSIS

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size
RM = rm

OUTPUT_NAME = $(OUTPUT_PATH)$(PROJ_NAME)

CFLAGS = -g -O0 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3
CFLAGS += -I.
CFLAGS += -I$(CMSIS)/CM3/CoreSupport -I$(CMSIS)/CM3/DeviceSupport/ST/STM32F10x
CFLAGS += -DSTM32F10X_MD

STARTUP = $(CMSIS)/CM3/DeviceSupport/ST/STM32F10x/startup/gcc_ride7/startup_stm32f10x_md.s
SRCS += $(CMSIS)/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
SRCS += $(CMSIS)/CM3/CoreSupport/core_cm3.c
OBJS = $(SRCS:.c=.o)

LDFLAGS = --specs=rdimon.specs -Tstm32_flash.ld

.PHONY: all clean

all: $(OUTPUT_NAME).elf $(OUTPUT_NAME).bin

$(OUTPUT_NAME).bin: $(OUTPUT_NAME).elf
	$(OBJCOPY) -O binary $^ $@

$(OUTPUT_NAME).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(STARTUP) $^ -o $@
	$(SIZE) $@

$(OBJS): %.o:%.c
	$(CC) -c $(CFLAGS) -c $< -o $@

clean:
	-$(RM) $(OUTPUT_NAME).bin $(OUTPUT_NAME).elf $(OBJS)
