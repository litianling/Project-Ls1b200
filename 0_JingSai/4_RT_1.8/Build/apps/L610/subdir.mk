#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../apps/L610/L610_app_TCP.c \
../apps/L610/L610_Check.c \
../apps/L610/L610_Ctrl.c \
../apps/L610/L610_Init.c \
../apps/L610/L610_ShortMessage.c \
../apps/L610/L610_TCP.c \
../apps/L610/L610_Telephone.c \
../apps/L610/L610_TXcloud.c \
../apps/L610/LBS_Position.c \
../apps/L610/pdu.c

OBJS += \
./apps/L610/L610_app_TCP.o \
./apps/L610/L610_Check.o \
./apps/L610/L610_Ctrl.o \
./apps/L610/L610_Init.o \
./apps/L610/L610_ShortMessage.o \
./apps/L610/L610_TCP.o \
./apps/L610/L610_Telephone.o \
./apps/L610/L610_TXcloud.o \
./apps/L610/LBS_Position.o \
./apps/L610/pdu.o

C_DEPS += \
./apps/L610/L610_app_TCP.d \
./apps/L610/L610_Check.d \
./apps/L610/L610_Ctrl.d \
./apps/L610/L610_Init.d \
./apps/L610/L610_ShortMessage.d \
./apps/L610/L610_TCP.d \
./apps/L610/L610_Telephone.d \
./apps/L610/L610_TXcloud.d \
./apps/L610/LBS_Position.d \
./apps/L610/pdu.d

# Each subdirectory must supply rules for building sources it contributes
apps/L610/%.o: ../apps/L610/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -I"../usb/include" -I"../libc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

