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
	arm-none-eabi-gcc -D__RAM_MODE__ -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\lib\lpc177x_8x\Core\CMSIS\Include" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread\components\drivers\include\drivers" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread\components\drivers\include" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread\config" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\lib\lpc177x_8x\Core\Device\NXP\LPC177x_8x\Include" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\lib\lpc177x_8x\Drivers\include" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread\include" -O0 -ffunction-sections -fdata-sections -Wall -std=gnu99 -funsigned-bitfields -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


