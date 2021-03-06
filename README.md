# BMBurner
BMBurner is a Arduino based EPROM Programmer that can Read/Write/Verify Chips (27C256 and SST 27SF512)

# Downloads Links :
- [Windows Tool (BMBurner.exe)][] to Read/Write/Verify Chips
- [Project Files][] to Update BMBurner or to Create your own

# Downloads Drivers Links :
- Driver for CH340 Chipset (BMBurner) : [CH340 Driver][]
- Driver for FTDI Chipset (Moates) : [FTDI Driver][]

# How to Update the BMBurner :
- Download and Install the [CH340 Driver][] if not already done
- Download and Install the [Arduino Software][] if not already done
1. Download and Open the [Project Files][] of BMBurner
2. Copy the folder 'Arduino_BMBurn' into 'Documents\Arduino\libraries'
- (exemple : C:\Users\YOUR_USERNAME\Documents\Arduino\libraries\Arduino_BMBurn)
3. Open Arduino Software
4. Load the Project 'Arduino_BMBurn'
5. Select 'Arduino Nano' in 'Tools/Board'
6. Select 'Atmega328P' in 'Tools/Processor'
7. Select the BMBurner Serial Port in 'Tools/Port'
8. Clic on Upload!

# Compatible Chips List :
- 27C32 (Read-Only)
- 2732A (Read-Only)
- 27C64 (Read-Only)
- 27C128 (Read-Only)
- 27C256 (Read-Only)
- 27C512 (Read-Only)
- 27SF256 (Read-Only)
- 27SF512
- 28C64
- 28C256
- 29C256
- W27C512
- W27E512

# BMBurner Windows Tool Functions :

*** Image may differ from the updated tool ***

![alt tag](https://github.com/bouletmarc/BMBurner/blob/master/Wiring/Arduino_Windows_GUI.png)

- Load File to Buffer
- Save Buffer to File
- Read Chip
- Program Chip
- Verify Chip
- Blank Check
- Erase Chip
- Edit Reading/Writing Chip addressing
- Edit Reading/Writing Buffer addressing
- Much Mores...

# Wiring Diagram (if you want to make your own) :

![alt tag](https://github.com/bouletmarc/BMBurner/blob/master/Wiring/Wiring_Diagram.png)

# Requirements (if you want to make your own) :
- Windows XP|Vista|7|8|10 (.net framework 3.5 installed)
- 1x Arduino Nano/Uno (any arduino with atmega328 as microcontroller should work with same pinouts)
- 1x 28Pin ZIF Socket
- 2x 1K Resistors
- 6x 22K Resistors
- 3x 0.1uF Capacitors
- 2x 74HC595 (or other replacement)
- 2x 2N4401 (or other replacement)
- 2x 2N3906 (or other replacement)
- 1x Step-Up Converter (usually input 5v and output 12v, it produce 12v out of a 5v signal)

# Donations :

You can donate to me using [This Link][].

[This Link]: <https://www.paypal.me/bouletmarc>
[Project Files]: <https://github.com/bouletmarc/BMBurner/archive/master.zip>
[Windows Tool (BMBurner.exe)]: <https://github.com/bouletmarc/BMBurner/raw/master/BMBurner.exe>
[Arduino Software]: <https://www.arduino.cc/en/main/software>
[CH340 Driver]: <https://sparks.gogo.co.nz/assets/_site_/downloads/CH34x_Install_Windows_v3_4.zip>
[FTDI Driver]: <http://www.ftdichip.com/Drivers/CDM/CDM21228_Setup.zip>

