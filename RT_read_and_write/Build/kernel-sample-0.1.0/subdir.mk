#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../kernel-sample-0.1.0/dynmem_sample.c \
../kernel-sample-0.1.0/event_sample.c \
../kernel-sample-0.1.0/idlehook_sample.c \
../kernel-sample-0.1.0/interrupt_sample.c \
../kernel-sample-0.1.0/mailbox_sample.c \
../kernel-sample-0.1.0/memp_sample.c \
../kernel-sample-0.1.0/msgq_sample.c \
../kernel-sample-0.1.0/mutex_sample.c \
../kernel-sample-0.1.0/priority_inversion.c \
../kernel-sample-0.1.0/producer_consumer.c \
../kernel-sample-0.1.0/scheduler_hook.c \
../kernel-sample-0.1.0/semaphore_sample.c \
../kernel-sample-0.1.0/thread_sample.c \
../kernel-sample-0.1.0/timer_sample.c \
../kernel-sample-0.1.0/timeslice_sample.c

OBJS += \
./kernel-sample-0.1.0/dynmem_sample.o \
./kernel-sample-0.1.0/event_sample.o \
./kernel-sample-0.1.0/idlehook_sample.o \
./kernel-sample-0.1.0/interrupt_sample.o \
./kernel-sample-0.1.0/mailbox_sample.o \
./kernel-sample-0.1.0/memp_sample.o \
./kernel-sample-0.1.0/msgq_sample.o \
./kernel-sample-0.1.0/mutex_sample.o \
./kernel-sample-0.1.0/priority_inversion.o \
./kernel-sample-0.1.0/producer_consumer.o \
./kernel-sample-0.1.0/scheduler_hook.o \
./kernel-sample-0.1.0/semaphore_sample.o \
./kernel-sample-0.1.0/thread_sample.o \
./kernel-sample-0.1.0/timer_sample.o \
./kernel-sample-0.1.0/timeslice_sample.o

C_DEPS += \
./kernel-sample-0.1.0/dynmem_sample.d \
./kernel-sample-0.1.0/event_sample.d \
./kernel-sample-0.1.0/idlehook_sample.d \
./kernel-sample-0.1.0/interrupt_sample.d \
./kernel-sample-0.1.0/mailbox_sample.d \
./kernel-sample-0.1.0/memp_sample.d \
./kernel-sample-0.1.0/msgq_sample.d \
./kernel-sample-0.1.0/mutex_sample.d \
./kernel-sample-0.1.0/priority_inversion.d \
./kernel-sample-0.1.0/producer_consumer.d \
./kernel-sample-0.1.0/scheduler_hook.d \
./kernel-sample-0.1.0/semaphore_sample.d \
./kernel-sample-0.1.0/thread_sample.d \
./kernel-sample-0.1.0/timer_sample.d \
./kernel-sample-0.1.0/timeslice_sample.d

# Each subdirectory must supply rules for building sources it contributes
kernel-sample-0.1.0/%.o: ../kernel-sample-0.1.0/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2015.05/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

