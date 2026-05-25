################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../protocol/ \
../protocol/ethercat_slave/


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../../ClearLinkCompatibilityFirmware/Device_Startup/startup_same53.c \
../main.cpp \
../protocol/ethercat_slave/ethercat_slave.cpp


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
startup_same53.o \
main.o \
protocol/ethercat_slave/ethercat_slave.o

OBJS_AS_ARGS +=  \
startup_same53.o \
main.o \
protocol/ethercat_slave/ethercat_slave.o

C_DEPS +=  \
startup_same53.d \
main.d \
protocol/ethercat_slave/ethercat_slave.d

C_DEPS_AS_ARGS +=  \
startup_same53.d \
main.d \
protocol/ethercat_slave/ethercat_slave.d

OUTPUT_FILE_PATH +=EtherCATSlaveFirmware.elf

OUTPUT_FILE_PATH_AS_ARGS +=EtherCATSlaveFirmware.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+=  \
D:/ClearCore-Motion-Stack/ProjectTemplate/LwIP/Release/libLwIP.a \
D:/ClearCore-Motion-Stack/ProjectTemplate/libClearCore/Release/libClearCore.a \
D:/ClearCore-Motion-Stack/ProjectTemplate/LwIP/Release/libLwIP.a

LINKER_SCRIPT_DEP+=  \
../../ClearLinkCompatibilityFirmware/Device_Startup/flash_with_bootloader.ld \
../../ClearLinkCompatibilityFirmware/Device_Startup/flash_without_bootloader.ld


# AVR32/GNU C Compiler
./startup_same53.o: ../../ClearLinkCompatibilityFirmware/Device_Startup/startup_same53.c
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-gcc.exe$(QUOTE)  -x c -mthumb -D__SAME53N19A__ -DNDEBUG -DCLEARCORE -DRESTRICT=__restrict  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAME53_DFP\1.1.118\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\4.5.0\CMSIS\Include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\5.4.0\CMSIS\Core\Include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\libClearCore\inc" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\LwIP\LwIP\src\include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\LwIP\LwIP\port\include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\protocol"  -O3 -mlong-calls -Wall -mcpu=cortex-m4 -c -std=gnu11 --param max-inline-insns-single=50 -MMD -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./main.o: .././main.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAME53N19A__ -DNDEBUG -DCLEARCORE -DRESTRICT=__restrict  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\4.5.0\CMSIS\Include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAME53_DFP\1.1.118\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\5.4.0\CMSIS\Core\Include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\libClearCore\inc" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\LwIP\LwIP\src\include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\LwIP\LwIP\port\include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\protocol"  -O3 -fno-rtti -fno-exceptions -mlong-calls -Wall -mcpu=cortex-m4 -c -std=gnu++11 -fno-threadsafe-statics -nostdlib --param max-inline-insns-single=500 -MMD -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

protocol/ethercat_slave/ethercat_slave.o: ../protocol/ethercat_slave/ethercat_slave.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAME53N19A__ -DNDEBUG -DCLEARCORE -DRESTRICT=__restrict  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\4.5.0\CMSIS\Include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAME53_DFP\1.1.118\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\5.4.0\CMSIS\Core\Include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\libClearCore\inc" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\LwIP\LwIP\src\include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\LwIP\LwIP\port\include" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware" -I"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\protocol"  -O3 -fno-rtti -fno-exceptions -mlong-calls -Wall -mcpu=cortex-m4 -c -std=gnu++11 -fno-threadsafe-statics -nostdlib --param max-inline-insns-single=500 -MMD -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	




# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Linker : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -mthumb -Wl,-Map="EtherCATSlaveFirmware.map" --specs=rdimon.specs -Wl,--start-group -lm -larm_cortexM4lf_math -lLwIP -lClearCore -lLwIP  -Wl,--end-group -L"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\ClearLinkCompatibilityFirmware\Device_Startup" -L"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\4.5.0\CMSIS\Lib\GCC" -L"D:\ClearCore-Motion-Stack\ProjectTemplate\EtherCATSlaveFirmware\..\LwIP\Release" -L"D:\ClearCore-Motion-Stack\ProjectTemplate\libClearCore\Release" -L"D:\ClearCore-Motion-Stack\ProjectTemplate\LwIP\Release"  -Wl,--gc-sections -mcpu=cortex-m4 -Tflash_with_bootloader.ld -mfloat-abi=hard -mfpu=fpv4-sp-d16  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O binary "EtherCATSlaveFirmware.elf" "EtherCATSlaveFirmware.bin"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "EtherCATSlaveFirmware.elf" "EtherCATSlaveFirmware.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O binary "EtherCATSlaveFirmware.elf" "EtherCATSlaveFirmware.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objdump.exe" -h -S "EtherCATSlaveFirmware.elf" > "EtherCATSlaveFirmware.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature  "EtherCATSlaveFirmware.elf" "EtherCATSlaveFirmware.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-size.exe" "EtherCATSlaveFirmware.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "EtherCATSlaveFirmware.elf" "EtherCATSlaveFirmware.a" "EtherCATSlaveFirmware.hex" "EtherCATSlaveFirmware.bin" "EtherCATSlaveFirmware.lss" "EtherCATSlaveFirmware.eep" "EtherCATSlaveFirmware.map" "EtherCATSlaveFirmware.srec"
	