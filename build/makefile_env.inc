#工作区路径,项目路径,编译输出路径(可由make作为参数带入)
export PLAT_PATH=arm

export WORKSPACE_PATH=$(LINUX_PLAT_COMPONENT_LINUX)


export C_DEV_LCD = -DLCD_320_240


export OUTPUT_PATH=$(DRIVER_PATH)
export C_DEV_CPU = -DCPU_EC600U
export PREFIX_PATH=$(OUTPUT_PATH)

export TARGET_PATH=$(OUTPUT_PATH)
export LIBRARY_PATH=$(OUTPUT_PATH)/lib
export OBJECT_PATH=$(OUTPUT_PATH)/obj
export DEPEND_PATH=$(OUTPUT_PATH)/dep



# 编译器




export CC=arm-none-eabi-gcc.exe
export CXX=arm-none-eabi-gcc.exe
export CAR=arm-none-eabi-ar.exe
export LD=arm-none-eabi-gcc.exe
export OBJCOPY=arm-none-eabi-objcopy.exe

LDFLAGS:= -Wl,-gc-sections -std=c99 -fno-exceptions -fno-rtti -fno-threadsafe-statics -mcpu=cortex-a5 -mtune=generic-armv7-a -mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mno-unaligned-access -g -Os -Wall -T $(DRIVER_PATH)/app2_flashimg.ld  -Wl,-Map=$(TARGET_PATH)/dev.map 

export XFLAGS= -ffunction-sections -fdata-sections -std=c99 -fno-exceptions -fno-rtti -fno-threadsafe-statics   -mcpu=cortex-a5 -mtune=generic-armv7-a -mthumb -mfpu=neon-vfpv4 -mfloat-abi=hard -mno-unaligned-access -g -Os -Wall -fno-strict-aliasing -ffunction-sections -fdata-sections
#支持文件的扩展名
EXT=.c .cpp

export CFLAGS=-DCPU_EC600U -D__ARMEL__ -DOSI_LOG_TAG $(C_DEV_LCD) $(C_DEV_CPU)  -DM90_DEV  -DDEV_ET380
export CXXFLAGS=-DCPU_EC600U -D__ARMEL__ -DOSI_LOG_TAG $(C_DEV_LCD) $(C_DEV_CPU) -DM90_DEV  -DDEV_ET380


#export CFLAGS = $(PLAT_DEFINE)
#export CXXFLAGS = $(PLAT_DEFINE)
