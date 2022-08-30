#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../L610/L610_Init.c \
../L610/L610_TCP.c

OBJS += \
./L610/L610_Init.o \
./L610/L610_TCP.o

C_DEPS += \
./L610/L610_Init.d \
./L610/L610_TCP.d

# Each subdirectory must supply rules for building sources it contributes
L610/%.o: ../L610/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	E:/Longson/loongide/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_NONE  -O0 -g -Wall -c -fmessage-length=0 -pipe  -I"../" -I"../include" -I"../core/include" -I"../core/mips" -I"../ls1x-drv/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

