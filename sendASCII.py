import getch
import serial

port = '/dev/tty.usbmodem1411'


ard = serial.Serial(port,9600,timeout=5)
time.sleep(2) # wait for Arduino

print("Type Away! Spaces become newlines; * mirrors the x direction, and $ resets")

while True:
    char = getch.getche()
    ard.write(bytes(char))
    
