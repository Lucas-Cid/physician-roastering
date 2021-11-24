################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Area.cpp \
../src/Physician.cpp \
../src/Shift.cpp \
../src/main.cpp 

OBJS += \
./src/Area.o \
./src/Physician.o \
./src/Shift.o \
./src/main.o 

CPP_DEPS += \
./src/Area.d \
./src/Physician.d \
./src/Shift.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DIL_STD -DNDEBUG -DILOUSEMT -DILM_REENTRANT -I/opt/ibm/ILOG/CPLEX_Studio1210/cpoptimizer/include -I/opt/ibm/ILOG/CPLEX_Studio1210/concert/include -O3 -pedantic -Wall -Wextra -c -fmessage-length=0 -fstrict-aliasing -fexceptions -frounding-math  -Wno-long-long -m64 -Wno-ignored-attributes -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


