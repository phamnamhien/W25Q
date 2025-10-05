################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../W25Q/w25q.c 

OBJS += \
./W25Q/w25q.o 

C_DEPS += \
./W25Q/w25q.d 


# Each subdirectory must supply rules for building sources it contributes
W25Q/%.o W25Q/%.su W25Q/%.cyclo: ../W25Q/%.c W25Q/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F107xC -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/Documents/GitHub/phamnamhien/W25Q/W25Q" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-W25Q

clean-W25Q:
	-$(RM) ./W25Q/w25q.cyclo ./W25Q/w25q.d ./W25Q/w25q.o ./W25Q/w25q.su

.PHONY: clean-W25Q

