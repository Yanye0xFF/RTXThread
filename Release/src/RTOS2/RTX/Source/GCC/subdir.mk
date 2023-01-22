################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/RTOS2/RTX/Source/GCC/irq_armv7m.S 

OBJS += \
./src/RTOS2/RTX/Source/GCC/irq_armv7m.o 

S_UPPER_DEPS += \
./src/RTOS2/RTX/Source/GCC/irq_armv7m.d 


# Each subdirectory must supply rules for building sources it contributes
src/RTOS2/RTX/Source/GCC/%.o: ../src/RTOS2/RTX/Source/GCC/%.S src/RTOS2/RTX/Source/GCC/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -march=armv7e-m -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 -munaligned-access -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wuninitialized -Wall -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -x assembler-with-cpp -I"../src/RTOS2/RTX/Config" -I"../src/RTOS2/RTX/Include" -I"../src/bsp" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


