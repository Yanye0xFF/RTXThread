################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/application/collection/arraylist.c \
../src/application/collection/ringarray.c 

OBJS += \
./src/application/collection/arraylist.o \
./src/application/collection/ringarray.o 

C_DEPS += \
./src/application/collection/arraylist.d \
./src/application/collection/ringarray.d 


# Each subdirectory must supply rules for building sources it contributes
src/application/collection/%.o: ../src/application/collection/%.c src/application/collection/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -march=armv7e-m -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 -munaligned-access -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wuninitialized -Wall -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -DGD32F30X_HD -DARM_MATH_CM4 -DUSE_STDPERIPH_DRIVER -DHXTAL_VALUE=8000000 -I"../src/cmsis" -I"../src/stdperiph/Include" -I"../src/rtthread/include" -I"../src/RTOS2/Include" -I"../src/RTOS2/RTX/Config" -I"../src/RTOS2/RTX/Include" -I"../src/RTOS2/RTX/Source" -I"../src/bsp" -I"../src/bsp/driver" -I"../src/components/rtt" -I"../src/components/finsh" -I"../src/components/rtc" -I"../src/components/spi" -I"../src/components/misc" -I"../src/components/ipc" -I"../src/components/can" -I"../src/components/sfud" -I"../src/components" -I"../src/application" -I"../src/application/algorithm" -I"../src/application/collection" -I"../src/thirdparty/lsm6dsl" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


