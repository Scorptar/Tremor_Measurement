# Description

Tremor Measurement using ADXL362 accelerometers controlled by an Arduino (ATmega328p) + data acquisition on PC through serial port (USB).

# Application

Tremor Measurement is a low-budget solution to measure tremor in the medical field (Parkinson,...)

# Principle

Tremor diseases typically make 1-30 Hz signal measured using accelerometers (1 to 5 sensors)

# Software

TM.py is runned on the PC : the program make a 30,60,90,120,150 seconds analysis and output an average FFT from the different sensors. Then, the user can generate a PDF with the results.

# Installation

1) Install a Python compiler (such as Pycharm for example)
2) Install Arduino software
3) In your ...\Documents\Arduino\libraries folder, put the ADXL362 library folder
4) Depending the number of accelerometer, put the right .ino code in your Arduino
5) On your PC, create a directory called "Images" in the folder where TM.py is
6) Lauch TM.py
7) Done.
