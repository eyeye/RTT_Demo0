################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/system_LPC177x_8x.c 

OBJS += \
./src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/system_LPC177x_8x.o 

C_DEPS += \
./src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/system_LPC177x_8x.d 


# Each subdirectory must supply rules for building sources it contributes
src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/%.o: ../src/lib/lpc177x_8x/Core/Device/NXP/LPC177x_8x/Source/Templates/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -D__RAM_MODE__ -I"D:\workspace_gnu\RTT_Demo0\src\lib\lpc177x_8x\Core\CMSIS\Include" -I"D:\workspace_gnu\RTT_Demo0\src" -I"D:\workspace_gnu\RTT_Demo0\src\hal\inc" -I"D:\workspace_gnu\RTT_Demo0\src\os\rt-thread\config" -I"D:\workspace_gnu\RTT_Demo0\src\lib\lpc177x_8x\Core\Device\NXP\LPC177x_8x\Include" -I"D:\workspace_gnu\RTT_Demo0\src\lib\lpc177x_8x\Drivers\include" -I"D:\workspace_gnu\RTT_Demo0\src\os\rt-thread\include" -O0 -Wall -std=c99 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


