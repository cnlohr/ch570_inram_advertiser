all : runfromram

TARGET:=broadcaster
TARGET_MCU:=CH570
TARGET_MCU_PACKAGE:=CH570D

ADDITIONAL_C_FILES+=ble.c
LINKER_SCRIPT:=app_in_ram.ld

include ../ch32fun/ch32fun/ch32fun.mk

flash : runfromram

runfromram : $(TARGET).bin
	$(MINICHLINK)/minichlink -kw $(TARGET).bin 0x20000000
	$(MINICHLINK)/minichlink -ks 0x18 0 # Disable auto execution
	$(MINICHLINK)/minichlink -ks 0x04 0x20000000 # Write DATA0 = RAM start
	$(MINICHLINK)/minichlink -ks 0x17 0x002307b1 # Write DATA0 to DPC
	$(MINICHLINK)/minichlink -ks 0x10 0x40000001 -T # Resume Request

clean : cv_clean

