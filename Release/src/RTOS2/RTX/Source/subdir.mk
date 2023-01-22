################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/RTOS2/RTX/Source/rtx_delay.c \
../src/RTOS2/RTX/Source/rtx_evflags.c \
../src/RTOS2/RTX/Source/rtx_evr.c \
../src/RTOS2/RTX/Source/rtx_kernel.c \
../src/RTOS2/RTX/Source/rtx_lib.c \
../src/RTOS2/RTX/Source/rtx_mempool.c \
../src/RTOS2/RTX/Source/rtx_msgqueue.c \
../src/RTOS2/RTX/Source/rtx_mutex.c \
../src/RTOS2/RTX/Source/rtx_semaphore.c \
../src/RTOS2/RTX/Source/rtx_system.c \
../src/RTOS2/RTX/Source/rtx_thread.c \
../src/RTOS2/RTX/Source/rtx_timer.c 

OBJS += \
./src/RTOS2/RTX/Source/rtx_delay.o \
./src/RTOS2/RTX/Source/rtx_evflags.o \
./src/RTOS2/RTX/Source/rtx_evr.o \
./src/RTOS2/RTX/Source/rtx_kernel.o \
./src/RTOS2/RTX/Source/rtx_lib.o \
./src/RTOS2/RTX/Source/rtx_mempool.o \
./src/RTOS2/RTX/Source/rtx_msgqueue.o \
./src/RTOS2/RTX/Source/rtx_mutex.o \
./src/RTOS2/RTX/Source/rtx_semaphore.o \
./src/RTOS2/RTX/Source/rtx_system.o \
./src/RTOS2/RTX/Source/rtx_thread.o \
./src/RTOS2/RTX/Source/rtx_timer.o 

C_DEPS += \
./src/RTOS2/RTX/Source/rtx_delay.d \
./src/RTOS2/RTX/Source/rtx_evflags.d \
./src/RTOS2/RTX/Source/rtx_evr.d \
./src/RTOS2/RTX/Source/rtx_kernel.d \
./src/RTOS2/RTX/Source/rtx_lib.d \
./src/RTOS2/RTX/Source/rtx_mempool.d \
./src/RTOS2/RTX/Source/rtx_msgqueue.d \
./src/RTOS2/RTX/Source/rtx_mutex.d \
./src/RTOS2/RTX/Source/rtx_semaphore.d \
./src/RTOS2/RTX/Source/rtx_system.d \
./src/RTOS2/RTX/Source/rtx_thread.d \
./src/RTOS2/RTX/Source/rtx_timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/RTOS2/RTX/Source/%.o: ../src/RTOS2/RTX/Source/%.c src/RTOS2/RTX/Source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -march=armv7e-m -mthumb -mlittle-endian -mfloat-abi=hard -mfpu=fpv4-sp-d16 -munaligned-access -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wuninitialized -Wall -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -DGD32F30X_HD -DARM_MATH_CM4 -DUSE_STDPERIPH_DRIVER -DHXTAL_VALUE=8000000 -I"../src/cmsis" -I"../src/stdperiph/Include" -I"../src/rtthread/include" -I"../src/RTOS2/Include" -I"../src/RTOS2/RTX/Config" -I"../src/RTOS2/RTX/Include" -I"../src/RTOS2/RTX/Source" -I"../src/bsp" -I"../src/bsp/driver" -I"../src/components/rtt" -I"../src/components/finsh" -I"../src/components/rtc" -I"../src/components/spi" -I"../src/components/misc" -I"../src/components/ipc" -I"../src/components/can" -I"../src/components/sfud" -I"../src/components" -I"../src/application" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


