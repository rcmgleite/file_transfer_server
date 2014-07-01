################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/source/server.c \
../src/source/server_conn_utils.c \
../src/source/server_utils.c 

OBJS += \
./src/source/server.o \
./src/source/server_conn_utils.o \
./src/source/server_utils.o 

C_DEPS += \
./src/source/server.d \
./src/source/server_conn_utils.d \
./src/source/server_utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/source/%.o: ../src/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/rafael/Desktop/rafael/C/proj_redes_server/src/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


