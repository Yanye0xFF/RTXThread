################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bsp/driver/drv_adc.c \
../src/bsp/driver/drv_can.c \
../src/bsp/driver/drv_dac.c \
../src/bsp/driver/drv_dwt.c \
../src/bsp/driver/drv_mem.c \
../src/bsp/driver/drv_pin.c \
../src/bsp/driver/drv_rtc.c \
../src/bsp/driver/drv_spi.c \
../src/bsp/driver/drv_timer.c \
../src/bsp/driver/pre_init.c 

OBJS += \
./src/bsp/driver/drv_adc.o \
./src/bsp/driver/drv_can.o \
./src/bsp/driver/drv_dac.o \
./src/bsp/driver/drv_dwt.o \
./src/bsp/driver/drv_mem.o \
./src/bsp/driver/drv_pin.o \
./src/bsp/driver/drv_rtc.o \
./src/bsp/driver/drv_spi.o \
./src/bsp/driver/drv_timer.o \
./src/bsp/driver/pre_init.o 

C_DEPS += \
./src/bsp/driver/drv_adc.d \
./src/bsp/driver/drv_can.d \
./src/bsp/driver/drv_dac.d \
./src/bsp/driver/drv_dwt.d \
./src/bsp/driver/drv_mem.d \
./src/bsp/driver/drv_pin.d \
./src/bsp/driver/drv_rtc.d \
./src/bsp/driver/drv_spi.d \
./src/bsp/driver/drv_timer.d \
./src/bsp/driver/pre_init.d 


# Each subdirectory must supply rules for building sources it contributes
src/bsp/driver/%.o: ../src/bsp/driver/%.c src/bsp/driver/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -march=armv7e-m -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 -munaligned-access -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wuninitialized -Wall -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -DGD32F30X_HD -DARM_MATH_CM4 -DUSE_STDPERIPH_DRIVER -DHXTAL_VALUE=8000000 -I"../src/cmsis" -I"../src/stdperiph/Include" -I"../src/rtthread/include" -I"../src/RTOS2/Include" -I"../src/RTOS2/RTX/Config" -I"../src/RTOS2/RTX/Include" -I"../src/RTOS2/RTX/Source" -I"../src/bsp" -I"../src/bsp/driver" -I"../src/components/rtt" -I"../src/components/finsh" -I"../src/components/rtc" -I"../src/components/spi" -I"../src/components/misc" -I"../src/components/ipc" -I"../src/components/can" -I"../src/components/sfud" -I"../src/components" -I"../src/application" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


