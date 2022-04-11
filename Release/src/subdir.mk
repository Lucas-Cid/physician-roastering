################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Area.cpp \
../src/CsvHandler.cpp \
../src/Date.cpp \
../src/DateHandler.cpp \
../src/Physician.cpp \
../src/RestrictedShift.cpp \
../src/RosteringInput.cpp \
../src/Shift.cpp \
../src/SoftConstraint.cpp \
../src/Solution.cpp \
../src/main.cpp \
../src/roastering.cpp 

OBJS += \
./src/Area.o \
./src/CsvHandler.o \
./src/Date.o \
./src/DateHandler.o \
./src/Physician.o \
./src/RestrictedShift.o \
./src/RosteringInput.o \
./src/Shift.o \
./src/SoftConstraint.o \
./src/Solution.o \
./src/main.o \
./src/roastering.o 

CPP_DEPS += \
./src/Area.d \
./src/CsvHandler.d \
./src/Date.d \
./src/DateHandler.d \
./src/Physician.d \
./src/RestrictedShift.d \
./src/RosteringInput.d \
./src/Shift.d \
./src/SoftConstraint.d \
./src/Solution.d \
./src/main.d \
./src/roastering.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DIL_STD -DNDEBUG -DILOUSEMT -DILM_REENTRANT -I/opt/ibm/ILOG/CPLEX_Studio1210/cpoptimizer/include -I/opt/ibm/ILOG/CPLEX_Studio1210/concert/include -I/usr/include/python3.8 -I/home/lucas/vcpkg/installed/x64-linux/include -O3 -pedantic -Wall -Wextra -c -fmessage-length=0 -fstrict-aliasing -fexceptions -frounding-math  -Wno-long-long -m64 -Wno-ignored-attributes -std=c++11 -I/usr/include/python3.8 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


