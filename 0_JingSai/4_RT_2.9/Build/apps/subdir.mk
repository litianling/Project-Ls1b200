#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../apps/air_quality.c \
../apps/human_sensor.c \
../apps/mouse_draw.c \
../apps/push_box.c \
../apps/read_disk.c \
../apps/set_color.c \
../apps/SG180.c \
../apps/temperature.c

OBJS += \
./apps/air_quality.o \
./apps/human_sensor.o \
./apps/mouse_draw.o \
./apps/push_box.o \
./apps/read_disk.o \
./apps/set_color.o \
./apps/SG180.o \
./apps/temperature.o

C_DEPS += \
./apps/air_quality.d \
./apps/human_sensor.d \
./apps/mouse_draw.d \
./apps/push_box.d \
./apps/read_disk.d \
./apps/set_color.d \
./apps/SG180.d \
./apps/temperature.d

# Each subdirectory must supply rules for building sources it contributes
apps/%.o: ../apps/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	D:/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_RTTHREAD  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../RTT4/include" -I"../RTT4/port/include" -I"../RTT4/port/mips" -I"../RTT4/components/finsh" -I"../RTT4/components/dfs/include" -I"../RTT4/components/drivers/include" -I"../RTT4/components/libc/time" -I"../RTT4/bsp-ls1x" -I"../ls1x-drv/include" -I"../usb/include" -I"../libc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

