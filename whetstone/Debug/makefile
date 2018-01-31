CFLAGS = -Wall -O0 -g3
INC = -I../../../jtag_eval/basic_bsp/ps7_cortexa9_0/include -I../../common
LD_SRCS += ../../common/lscript.ld
LIB = -L../../../jtag_eval/basic_bsp/ps7_cortexa9_0/lib
SPEC = -specs=../../common/Xilinx.spec
LIBS := -Wl,--start-group,-lxil,-lgcc,-lc,-lm,--end-group

RM := rm -rf


# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
whetstone.elf.size \

# All Target
all: pre-build main-build

# Main-build Target
main-build: whetstone.elf secondary-outputs

C_SRCS += ../../common/platform.c  \
../src/whetstone.c

OBJS += ../../common/platform.o \
../src/whetstone.o

C_DEPS += ../../common/platform.d \
../src/whetstone.d

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc $(CFLAGS) -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard $(INC) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# Tool invocations
whetstone.elf: $(OBJS) $(LD_SRCS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM v7 gcc linker'
	arm-none-eabi-gcc -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -Wl,-build-id=none $(SPEC) -Wl,-T -Wl,$(LD_SRCS) $(LIB) -o "whetstone.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

whetstone.elf.size: whetstone.elf
	@echo 'Invoking: ARM v7 Print Size'
	arm-none-eabi-size whetstone.elf  |tee "whetstone.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES) $(OBJS) $(S_UPPER_DEPS) $(C_DEPS) $(ELFSIZE) whetstone.elf Xilinx.spec
	-@echo ' '

pre-build:
	-a9-linaro-pre-build-step
	-@echo ' '

secondary-outputs: $(ELFSIZE)

.PHONY: all clean dependents
.SECONDARY: main-build pre-build

-include ../makefile.targets
