#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../usb/common/usb.c \
../usb/common/usb_hub.c \
../usb/common/usb_kbd.c \
../usb/common/usb_storage.c

OBJS += \
./usb/common/usb.o \
./usb/common/usb_hub.o \
./usb/common/usb_kbd.o \
./usb/common/usb_storage.o

C_DEPS += \
./usb/common/usb.d \
./usb/common/usb_hub.d \
./usb/common/usb_kbd.d \
./usb/common/usb_storage.d

# Each subdirectory must supply rules for building sources it contributes
usb/common/%.o: ../usb/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	E:/Longson/loongide/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -I"../usb/include" -I"../libc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

