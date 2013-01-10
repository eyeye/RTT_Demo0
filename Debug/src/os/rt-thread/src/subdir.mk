################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/os/rt-thread/src/clock.c \
../src/os/rt-thread/src/device.c \
../src/os/rt-thread/src/idle.c \
../src/os/rt-thread/src/ipc.c \
../src/os/rt-thread/src/irq.c \
../src/os/rt-thread/src/kservice.c \
../src/os/rt-thread/src/mem.c \
../src/os/rt-thread/src/memheap.c \
../src/os/rt-thread/src/mempool.c \
../src/os/rt-thread/src/module.c \
../src/os/rt-thread/src/object.c \
../src/os/rt-thread/src/scheduler.c \
../src/os/rt-thread/src/slab.c \
../src/os/rt-thread/src/thread.c \
../src/os/rt-thread/src/timer.c 

OBJS += \
./src/os/rt-thread/src/clock.o \
./src/os/rt-thread/src/device.o \
./src/os/rt-thread/src/idle.o \
./src/os/rt-thread/src/ipc.o \
./src/os/rt-thread/src/irq.o \
./src/os/rt-thread/src/kservice.o \
./src/os/rt-thread/src/mem.o \
./src/os/rt-thread/src/memheap.o \
./src/os/rt-thread/src/mempool.o \
./src/os/rt-thread/src/module.o \
./src/os/rt-thread/src/object.o \
./src/os/rt-thread/src/scheduler.o \
./src/os/rt-thread/src/slab.o \
./src/os/rt-thread/src/thread.o \
./src/os/rt-thread/src/timer.o 

C_DEPS += \
./src/os/rt-thread/src/clock.d \
./src/os/rt-thread/src/device.d \
./src/os/rt-thread/src/idle.d \
./src/os/rt-thread/src/ipc.d \
./src/os/rt-thread/src/irq.d \
./src/os/rt-thread/src/kservice.d \
./src/os/rt-thread/src/mem.d \
./src/os/rt-thread/src/memheap.d \
./src/os/rt-thread/src/mempool.d \
./src/os/rt-thread/src/module.d \
./src/os/rt-thread/src/object.d \
./src/os/rt-thread/src/scheduler.d \
./src/os/rt-thread/src/slab.d \
./src/os/rt-thread/src/thread.d \
./src/os/rt-thread/src/timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/os/rt-thread/src/%.o: ../src/os/rt-thread/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -D__RAM_MODE__ -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\lib\lpc177x_8x\Core\CMSIS\Include" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread\components\drivers\include\drivers" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread\components\drivers\include" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread\config" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\lib\lpc177x_8x\Core\Device\NXP\LPC177x_8x\Include" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\lib\lpc177x_8x\Drivers\include" -I"E:\workspaces\Juno_Gnu\RTT_Demo0\src\os\rt-thread\include" -O0 -ffunction-sections -fdata-sections -Wall -std=gnu99 -funsigned-bitfields -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=cortex-m3 -mthumb -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


