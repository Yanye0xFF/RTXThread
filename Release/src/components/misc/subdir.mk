################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/components/misc/pin.c \
../src/components/misc/rt_adc.c \
../src/components/misc/rt_dac.c \
../src/components/misc/rt_inputcapture.c \
../src/components/misc/rt_pwm.c 

OBJS += \
./src/components/misc/pin.o \
./src/components/misc/rt_adc.o \
./src/components/misc/rt_dac.o \
./src/components/misc/rt_inputcapture.o \
./src/components/misc/rt_pwm.o 

C_DEPS += \
./src/components/misc/pin.d \
./src/components/misc/rt_adc.d \
./src/components/misc/rt_dac.d \
./src/components/misc/rt_inputcapture.d \
./src/components/misc/rt_pwm.d 


# Each subdirectory must supply rules for building sources it contributes
src/components/misc/%.o: ../src/components/misc/%.c src/components/misc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -march=armv7e-m -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 -munaligned-access -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wuninitialized -Wall -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -DGD32F30X_HD -DARM_MATH_CM4 -DUSE_STDPERIPH_DRIVER -DHXTAL_VALUE=8000000 -I"../src/cmsis" -I"../src/stdperiph/Include" -I"../src/rtthread/include" -I"../src/RTOS2/Include" -I"../src/RTOS2/RTX/Config" -I"../src/RTOS2/RTX/Include" -I"../src/RTOS2/RTX/Source" -I"../src/bsp" -I"../src/bsp/driver" -I"../src/components/rtt" -I"../src/components/finsh" -I"../src/components/rtc" -I"../src/components/spi" -I"../src/components/misc" -I"../src/components/ipc" -I"../src/components/can" -I"../src/components/sfud" -I"../src/components" -I"../src/application" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


