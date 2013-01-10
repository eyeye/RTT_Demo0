################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/GCC/startup_LPC177x_8x.S 

OBJS += \
./src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/GCC/startup_LPC177x_8x.o 

S_UPPER_DEPS += \
./src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/GCC/startup_LPC177x_8x.d 


# Each subdirectory must supply rules for building sources it contributes
src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/GCC/%.o: ../src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/GCC/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -DRAM_MODE=1 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


