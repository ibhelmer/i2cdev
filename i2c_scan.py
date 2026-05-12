from machine import Pin, I2C
from time import sleep
# Device address 0x42
# Reg 0 = status of LED2
# Reg 1 = control of LED2
# Reg 3 = Device ID 47

i2c = I2C(0, scl=Pin(20), sda=Pin(19))
print(i2c.scan())
i2c.writeto(0x42,bytearray((3)))
print(i2c.readfrom(0x42,1))

while (1==1):
   i2c.writeto(0x42,bytearray((0,1)))
   print(i2c.readfrom(0x42,1))
   sleep(0.1)
   i2c.writeto(0x42,bytearray((0,0)))
   print(i2c.readfrom(0x42,1))
   sleep(0.1)
   




