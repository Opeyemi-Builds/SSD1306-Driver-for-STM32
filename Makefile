# Build: make            (builds firmware.elf/.bin in build/)
# Flash: make flash      (needs OpenOCD + ST-LINK, i.e. the Nucleo plugged in)
# Other: make size | make clean
CROSS   ?= arm-none-eabi-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
SIZE    := $(CROSS)size

EXAMPLE ?= 02_oled_hello
BUILD   := build

SRCS := platform/stm32f446/startup.c \
        platform/stm32f446/libc_min.c \
        platform/stm32f446/timebase.c \
        drivers/common/status.c \
        drivers/gpio/gpio.c \
        drivers/i2c/i2c.c \
        drivers/ssd1306/ssd1306.c \
        examples/$(EXAMPLE)/main.c

INCS := -Iplatform/stm32f446 -Idrivers/common -Idrivers/gpio -Idrivers/i2c -Idrivers/ssd1306

CPU    := -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS := $(CPU) -std=c11 -Os -g3 -Wall -Wextra -Wshadow \
          -ffreestanding -fno-tree-loop-distribute-patterns \
          -ffunction-sections -fdata-sections $(INCS)
LDFLAGS := $(CPU) -nostdlib -Tplatform/stm32f446/stm32f446re.ld \
           -Wl,--gc-sections -Wl,-Map=$(BUILD)/firmware.map
OBJS := $(patsubst %.c,$(BUILD)/%.o,$(SRCS))

all: $(BUILD)/firmware.bin size

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD)/firmware.elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -lgcc -o $@

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(OBJCOPY) -O binary $< $@

size: $(BUILD)/firmware.elf
	$(SIZE) $<

flash: $(BUILD)/firmware.elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
	        -c "program $< verify reset exit"

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)
.PHONY: all size flash clean
