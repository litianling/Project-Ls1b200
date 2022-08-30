#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../kernel-samples/dynmem_sample.c \
../kernel-samples/event_sample.c \
../kernel-samples/idlehook_sample.c \
../kernel-samples/interrupt_sample.c \
../kernel-samples/mailbox_sample.c \
../kernel-samples/memp_sample.c \
../kernel-samples/msgq_sample.c \
../kernel-samples/mutex_sample.c \
../kernel-samples/priority_inversion.c \
../kernel-samples/producer_consumer.c \
../kernel-samples/push_box.c \
../kernel-samples/read_disk.c \
../kernel-samples/scheduler_hook.c \
../kernel-samples/semaphore_sample.c \
../kernel-samples/thread_sample.c \
../kernel-samples/timer_sample.c \
../kernel-samples/timeslice_sample.c

OBJS += \
./kernel-samples/dynmem_sample.o \
./kernel-samples/event_sample.o \
./kernel-samples/idlehook_sample.o \
./kernel-samples/interrupt_sample.o \
./kernel-samples/mailbox_sample.o \
./kernel-samples/memp_sample.o \
./kernel-samples/msgq_sample.o \
./kernel-samples/mutex_sample.o \
./kernel-samples/priority_inversion.o \
./kernel-samples/producer_consumer.o \
./kernel-samples/push_box.o \
./kernel-samples/read_disk.o \
./kernel-samples/scheduler_hook.o \
./kernel-samples/semaphore_sample.o \
./kernel-samples/thread_sample.o \
./kernel-samples/timer_sample.o \
./kernel-samples/timeslice_sample.o

C_DEPS += \
./kernel-samples/dynmem_sample.d \
./kernel-samples/event_sample.d \
./kernel-samples/idlehook_sample.d \
./kernel-samples/interrupt_sample.d \
./kernel-samples/mailbox_sample.d \
./kernel-samples/memp_sample.d \
./kernel-samples/msgq_sample.d \
./kernel-samples/mutex_sample.d \
./kernel-samples/priority_inversion.d \
./kernel-samples/producer_consumer.d \
./kernel-samples/push_box.d \
./kernel-samples/read_disk.d \
./kernel-samples/scheduler_hook.d \
./kernel-samples/semaphore_sample.d \
./kernel-samples/thread_sample.d \
./kernel-samples/timer_sample.d \
./kernel-samples/timeslice_sample.d

# Each subdirectory must supply rules for building sources it contributes
kernel-samples/%.o: ../kernel-samples/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -I"../usb/include" -I"../libc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

