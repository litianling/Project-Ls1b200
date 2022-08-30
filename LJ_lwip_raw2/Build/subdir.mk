#
# Auto-Generated file. Do not edit!
#

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../main.c \
../lwip_test.c \
../tcp_client.c \
../tcp_server.c \
../udp_client.c \
../udp_server.c

OBJS += \
./main.o \
./lwip_test.o \
./tcp_client.o \
./tcp_server.o \
./udp_client.o \
./udp_server.o

C_DEPS += \
./main.d \
./lwip_test.d \
./tcp_client.d \
./tcp_server.d \
./udp_client.d \
./udp_server.d

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: SDE Lite C Compiler'
	E:/Longson/loongide/LoongIDE/mips-2011.03/bin/mips-sde-elf-gcc.exe -mips32 -G0 -EL -msoft-float -DLS1B -DOS_NONE  -O0 -g -Wall -c -fmessage-length=0 -pipe -I"../" -I"../include" -I"../core/include" -I"../core/mips" -I"../ls1x-drv/include" -I"../lwIP-1.4.1/include" -I"../lwIP-1.4.1/include/ipv4" -I"../lwIP-1.4.1/port/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

