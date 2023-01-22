################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/components/finsh/cmd.c \
../src/components/finsh/finsh_port.c \
../src/components/finsh/msh.c \
../src/components/finsh/msh_file.c \
../src/components/finsh/msh_parse.c \
../src/components/finsh/shell.c 

OBJS += \
./src/components/finsh/cmd.o \
./src/components/finsh/finsh_port.o \
./src/components/finsh/msh.o \
./src/components/finsh/msh_file.o \
./src/components/finsh/msh_parse.o \
./src/components/finsh/shell.o 

C_DEPS += \
./src/components/finsh/cmd.d \
./src/components/finsh/finsh_port.d \
./src/components/finsh/msh.d \
./src/components/finsh/msh_file.d \
./src/components/finsh/msh_parse.d \
./src/components/finsh/shell.d 


# Each subdirectory must supply rules for building sources it contributes
src/components/finsh/%.o: ../src/components/finsh/%.c src/components/finsh/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -march=armv7e-m -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 -munaligned-access -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wuninitialized -Wall -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -DGD32F30X_HD -DARM_MATH_CM4 -DUSE_STDPERIPH_DRIVER -DHXTAL_VALUE=8000000 -I"../src/cmsis" -I"../src/stdperiph/Include" -I"../src/rtthread/include" -I"../src/RTOS2/Include" -I"../src/RTOS2/RTX/Config" -I"../src/RTOS2/RTX/Include" -I"../src/RTOS2/RTX/Source" -I"../src/bsp" -I"../src/bsp/driver" -I"../src/components/rtt" -I"../src/components/finsh" -I"../src/components/rtc" -I"../src/components/spi" -I"../src/components/misc" -I"../src/components/ipc" -I"../src/components/can" -I"../src/components/sfud" -I"../src/components" -I"../src/application" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


