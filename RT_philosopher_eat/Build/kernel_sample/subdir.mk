#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../kernel_sample/event_sample.c \
../kernel_sample/mailbox_sample.c \
../kernel_sample/memp_sample.c \
../kernel_sample/msgq_sample.c \
../kernel_sample/mutex_sample.c \
../kernel_sample/semaphore_sample.c \
../kernel_sample/thread_sample.c \
../kernel_sample/timer_sample.c \
../kernel_sample/timeslice_sample.c

OBJS += \
./kernel_sample/event_sample.o \
./kernel_sample/mailbox_sample.o \
./kernel_sample/memp_sample.o \
./kernel_sample/msgq_sample.o \
./kernel_sample/mutex_sample.o \
./kernel_sample/semaphore_sample.o \
./kernel_sample/thread_sample.o \
./kernel_sample/timer_sample.o \
./kernel_sample/timeslice_sample.o

C_DEPS += \
./kernel_sample/event_sample.d \
./kernel_sample/mailbox_sample.d \
./kernel_sample/memp_sample.d \
./kernel_sample/msgq_sample.d \
./kernel_sample/mutex_sample.d \
./kernel_sample/semaphore_sample.d \
./kernel_sample/thread_sample.d \
./kernel_sample/timer_sample.d \
./kernel_sample/timeslice_sample.d

# Each subdirectory must supply rules for building sources it contributes
kernel_sample/%.o: ../kernel_sample/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	C:/LoongIDE/mips-2015.05/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

