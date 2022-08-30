#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../usb/host/dwc2.c \
../usb/host/ehci-hcd.c \
../usb/host/ohci-hcd.c

OBJS += \
./usb/host/dwc2.o \
./usb/host/ehci-hcd.o \
./usb/host/ohci-hcd.o

C_DEPS += \
./usb/host/dwc2.d \
./usb/host/ehci-hcd.d \
./usb/host/ohci-hcd.d

# Each subdirectory must supply rules for building sources it contributes
usb/host/%.o: ../usb/host/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -I"../usb/include" -I"../libc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

