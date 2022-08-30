#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../kernel_samples/event_sample.c \
../kernel_samples/timer_sample.c \
../kernel_samples/semaphore_sample.c \
../kernel_samples/mutex_sample.c \
../kernel_samples/producer_consumer.c \
../kernel_samples/scheduler_hook.c \
../kernel_samples/interrupt_sample.c \
../kernel_samples/mailbox_sample.c \
../kernel_samples/smoker.c

OBJS += \
./kernel_samples/event_sample.o \
./kernel_samples/timer_sample.o \
./kernel_samples/semaphore_sample.o \
./kernel_samples/mutex_sample.o \
./kernel_samples/producer_consumer.o \
./kernel_samples/scheduler_hook.o \
./kernel_samples/interrupt_sample.o \
./kernel_samples/mailbox_sample.o \
./kernel_samples/smoker.o

C_DEPS += \
./kernel_samples/event_sample.d \
./kernel_samples/timer_sample.d \
./kernel_samples/semaphore_sample.d \
./kernel_samples/mutex_sample.d \
./kernel_samples/producer_consumer.d \
./kernel_samples/scheduler_hook.d \
./kernel_samples/interrupt_sample.d \
./kernel_samples/mailbox_sample.d \
./kernel_samples/smoker.d

# Each subdirectory must supply rules for building sources it contributes
kernel_samples/%.o: ../kernel_samples/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	C:/LoongIDE/mips-2015.05/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

