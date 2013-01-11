################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := cs-rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/os/rt-thread/src/subdir.mk
-include src/os/rt-thread/libcpu/arm/cortex-m3/subdir.mk
-include src/os/rt-thread/components/init/subdir.mk
-include src/os/rt-thread/components/drivers/src/subdir.mk
-include src/os/rt-thread/components/drivers/spi/subdir.mk
-include src/os/rt-thread/components/drivers/serial/subdir.mk
-include src/os/rt-thread/components/drivers/rtc/subdir.mk
-include src/os/rt-thread/components/drivers/mtd/subdir.mk
-include src/lib/lpc177x_8x/Drivers/source/subdir.mk
-include src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/GCC/subdir.mk
-include src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/subdir.mk
-include src/bsp/driver_periph/usart/subdir.mk
-include src/bsp/subdir.mk
-include src/app/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
RTT_Demo0.hex \

SECONDARY_SIZE += \
RTT_Demo0.siz \


# All Target
all: RTT_Demo0.elf secondary-outputs

# Tool invocations
RTT_Demo0.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Sourcery Windows GCC C Linker'
	arm-none-eabi-gcc -T"E:/workspaces/Juno_Gnu/RTT_Demo0/src/config/ldscript_ram_gnu.ld" -Xlinker --gc-sections -Wl,-Map,RTT_Demo0.map -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "RTT_Demo0.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

RTT_Demo0.hex: RTT_Demo0.elf
	@echo 'Invoking: ARM Sourcery Windows GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex RTT_Demo0.elf  "RTT_Demo0.hex"
	@echo 'Finished building: $@'
	@echo ' '

RTT_Demo0.siz: RTT_Demo0.elf
	@echo 'Invoking: ARM Sourcery Windows GNU Print Size'
	arm-none-eabi-size  --format=berkeley RTT_Demo0.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SECONDARY_SIZE)$(OBJS)$(C_DEPS)$(ASM_DEPS)$(SECONDARY_FLASH)$(EXECUTABLES)$(S_UPPER_DEPS) RTT_Demo0.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
