################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/stdperiph/Source/gd32f30x_adc.c \
../src/stdperiph/Source/gd32f30x_bkp.c \
../src/stdperiph/Source/gd32f30x_can.c \
../src/stdperiph/Source/gd32f30x_dac.c \
../src/stdperiph/Source/gd32f30x_dbg.c \
../src/stdperiph/Source/gd32f30x_dma.c \
../src/stdperiph/Source/gd32f30x_exti.c \
../src/stdperiph/Source/gd32f30x_fmc.c \
../src/stdperiph/Source/gd32f30x_fwdgt.c \
../src/stdperiph/Source/gd32f30x_gpio.c \
../src/stdperiph/Source/gd32f30x_misc.c \
../src/stdperiph/Source/gd32f30x_pmu.c \
../src/stdperiph/Source/gd32f30x_rcu.c \
../src/stdperiph/Source/gd32f30x_rtc.c \
../src/stdperiph/Source/gd32f30x_spi.c \
../src/stdperiph/Source/gd32f30x_timer.c \
../src/stdperiph/Source/gd32f30x_usart.c \
../src/stdperiph/Source/gd32f30x_wwdgt.c 

OBJS += \
./src/stdperiph/Source/gd32f30x_adc.o \
./src/stdperiph/Source/gd32f30x_bkp.o \
./src/stdperiph/Source/gd32f30x_can.o \
./src/stdperiph/Source/gd32f30x_dac.o \
./src/stdperiph/Source/gd32f30x_dbg.o \
./src/stdperiph/Source/gd32f30x_dma.o \
./src/stdperiph/Source/gd32f30x_exti.o \
./src/stdperiph/Source/gd32f30x_fmc.o \
./src/stdperiph/Source/gd32f30x_fwdgt.o \
./src/stdperiph/Source/gd32f30x_gpio.o \
./src/stdperiph/Source/gd32f30x_misc.o \
./src/stdperiph/Source/gd32f30x_pmu.o \
./src/stdperiph/Source/gd32f30x_rcu.o \
./src/stdperiph/Source/gd32f30x_rtc.o \
./src/stdperiph/Source/gd32f30x_spi.o \
./src/stdperiph/Source/gd32f30x_timer.o \
./src/stdperiph/Source/gd32f30x_usart.o \
./src/stdperiph/Source/gd32f30x_wwdgt.o 

C_DEPS += \
./src/stdperiph/Source/gd32f30x_adc.d \
./src/stdperiph/Source/gd32f30x_bkp.d \
./src/stdperiph/Source/gd32f30x_can.d \
./src/stdperiph/Source/gd32f30x_dac.d \
./src/stdperiph/Source/gd32f30x_dbg.d \
./src/stdperiph/Source/gd32f30x_dma.d \
./src/stdperiph/Source/gd32f30x_exti.d \
./src/stdperiph/Source/gd32f30x_fmc.d \
./src/stdperiph/Source/gd32f30x_fwdgt.d \
./src/stdperiph/Source/gd32f30x_gpio.d \
./src/stdperiph/Source/gd32f30x_misc.d \
./src/stdperiph/Source/gd32f30x_pmu.d \
./src/stdperiph/Source/gd32f30x_rcu.d \
./src/stdperiph/Source/gd32f30x_rtc.d \
./src/stdperiph/Source/gd32f30x_spi.d \
./src/stdperiph/Source/gd32f30x_timer.d \
./src/stdperiph/Source/gd32f30x_usart.d \
./src/stdperiph/Source/gd32f30x_wwdgt.d 


# Each subdirectory must supply rules for building sources it contributes
src/stdperiph/Source/%.o: ../src/stdperiph/Source/%.c src/stdperiph/Source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -march=armv7e-m -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 -munaligned-access -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wuninitialized -Wall -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -DGD32F30X_HD -DARM_MATH_CM4 -DUSE_STDPERIPH_DRIVER -DHXTAL_VALUE=8000000 -I"../src/cmsis" -I"../src/stdperiph/Include" -I"../src/rtthread/include" -I"../src/RTOS2/Include" -I"../src/RTOS2/RTX/Config" -I"../src/RTOS2/RTX/Include" -I"../src/RTOS2/RTX/Source" -I"../src/bsp" -I"../src/bsp/driver" -I"../src/components/rtt" -I"../src/components/finsh" -I"../src/components/rtc" -I"../src/components/spi" -I"../src/components/misc" -I"../src/components/ipc" -I"../src/components/can" -I"../src/components/sfud" -I"../src/components" -I"../src/application" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


