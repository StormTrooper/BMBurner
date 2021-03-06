//####################################################################################################################################
//####################################################################################################################################
//####################################################################################################################################
//
// EEPROM Programmer - code for an Arduino Nano V3.0 && Arduino Uno (anys Atmega328) - Written by BM Devs (Bouletmarc)
//
// This software presents a 115200-8N1 serial port.
// It is made to Read/Write Data from EEPROM directly with the Arduino!
// This program is made to read 28pins Chips such as 27c256, 27sf512
// 
// For more informations visit : https://www.github.com/bouletmarc/BMBurner/
//
//
//---------------------------------------------------------------------------------------------------------------------------------------------------------
// COMMANDS :                                                  |   OUTPUTS :                      |   DESC :
//---------------------------------------------------------------------------------------------------------------------------------------------------------
// 2 + R + "MMSB" + "MSB" + "LSB" + "CS"                       | "B1" + "B2" + "...B256" + "CS"   |   Reads 256 bytes of data from the EEPROM at address "MSB" + "LSB"  ... On Chip Type  2
// 3 + W + "MMSB" + "MSB" + "LSB" + "B1" + "B2" + "..." + "CS" | "O" (0x4F)                       |   Writes 256 bytes of data to the EEPROM at address "MSB" + "LSB", This output 'O' (0x79) for telling 'OK'  ... On Chip Type 3
// 5 + E + "CS"                                                | "O" (0x4F)                       |   Erase all the data on Chip Type 5 (SST)
// V + CS  (V+V)                                               | "V1" + "V2" + "V3"               |   Prints the version bytes of the BMBurner PCB Board for Moates Compatibility (Ex: V5.1.0 = 0x05, 0x01, 0x00)
// V + F                                                       | "V1" + "V2"                      |   Prints the version bytes of the BMBurner Firmware aka Arduino Project Version (Ex: V1.1 = 0x01, 0x01)
//
//**MMSB is always 0, this is only for compatibility with moates softwares on baud 115200, since the all chips arent more than 64KB (0xFFFF) there are no MMSB**
//####################################################################################################################################
//####################################################################################################################################
//####################################################################################################################################

byte BoardVersionBytes[] = {
  5, 1, 0  //Burn2 Version = 5, 15, 70
};

byte FirmwareVersionBytes[] = {
  1, 2
};

//Define chips predefinied model numbers (2-5 is for moates compatibility, 4 is never ever as it should be used for 27SF040 which is not compatible with this project at all)
unsigned int chipType;
#define CHIP27C128    1
#define CHIP27C256    2   //INCLUDED : 29C256, 27SF256
#define CHIP27C32     3   //INCLUDED : 2732A
#define CHIP27SF512   5   //INCLUDED : 27C512, W27E512, W27C512
#define CHIP28C64     6   //INCLUDED : 27C64
#define CHIP28C256    7

// define the IO lines for the data - bus
#define D0 2
#define D1 3
#define D2 4
#define D3 5
#define D4 6
#define D5 7
#define D6 8
#define D7 10

// for high voltage programming supply 
#define VH     11     //Erase Pin (A9)  12v
#define VPP    12     //OE Pin = VPP    12v

// shiftOut part
#define DS     A0
#define LATCH  A1
#define CLOCK  A2

// define the IO lines for the eeprom control
#define CE     A3
#define OE     A4
#define A15VPP A5     //A15 on SST, VPP on 27C256
#define A10    13

// direct access to port
#define STROBE_PORT PORTC
#define STROBE_DS      0
#define STROBE_LATCH   1
#define STROBE_CLOCK   2
#define STROBE_CE      3
#define STROBE_OE      4
#define STROBE_WE      5

//a buffer for bytes to burn
#define BUFFERSIZE 256
byte buffer[BUFFERSIZE + 1]; // +1 for checksum

//command buffer for parsing commands
int COMMANDSIZE = 2;    //Commands are set automatically when detecting commands
byte cmdbuf[262];

//flag set if we are on the first write pass
boolean firstWritePass = true;

//Last Address (used for sending high/low output at only 1 specific location on the 74HC595
unsigned int Last_Address = 0;

//###############################################################
//###############################################################
//###############################################################

void setup() {
  //define the shiftOut Pins as output
  pinMode(DS, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
 
  //define the boost pins as output (take care that they are LOW)
  digitalWrite(VH, LOW);
  pinMode(VH, OUTPUT);
  digitalWrite(VPP, LOW);
  pinMode(VPP, OUTPUT);

  //define the EEPROM Pins as output (take care that they are HIGH)
  digitalWrite(OE, HIGH);
  pinMode(OE, OUTPUT);
  digitalWrite(CE, HIGH);
  pinMode(CE, OUTPUT);
  digitalWrite(A15VPP, HIGH);
  pinMode(A15VPP, OUTPUT);
  digitalWrite(A10, HIGH);
  pinMode(A10, OUTPUT);

  //set speed of serial connection
  Serial.begin(115200);
  //Serial.begin(921600); //doesnt works, need 14.7456mhz crystal
  //Serial.begin(1000000);
}

void loop() {
  readCommand();
  if (cmdbuf[0] == 'V' && cmdbuf[1] == 'V') Serial.write(BoardVersionBytes, 3);
  if (cmdbuf[0] == 'V' && cmdbuf[1] == 'F') Serial.write(FirmwareVersionBytes, 2);
  if (cmdbuf[1] == 'R') Read();
  if (cmdbuf[1] == 'W') Write();
  if (cmdbuf[0] == '5' && cmdbuf[1] == 'E') EraseSST();
  //if (cmdbuf[0] == 'S' && cmdbuf[1] == 0  && cmdbuf[2] == 'S') Serial.write(0x4F); //moates commands for 921.6k baudrates
}

//###############################################################
// Serial Bytes functions
//###############################################################

void readCommand() {
  for(int i=0; i< COMMANDSIZE;i++) cmdbuf[i] = 0;
  int idx = 0;
  COMMANDSIZE = 2;
  
  do {
    if(Serial.available()) cmdbuf[idx++] = Serial.read();
    if (cmdbuf[1] == 'R') COMMANDSIZE = 6;
    if (cmdbuf[1] == 'W') COMMANDSIZE = 262;
    if (cmdbuf[0] == '5' && cmdbuf[1] == 'E') COMMANDSIZE = 3;
    //if (cmdbuf[0] == 'S' && cmdbuf[1] == 0) COMMANDSIZE = 3;  //moates commands for 921.6k baudrates
  }
  while (idx < (COMMANDSIZE));
}

void ChecksumThis() {
    byte num = 0;
    for (int i = 0; i < BUFFERSIZE; i++)
        num = (byte) (num + buffer[i]);
    buffer[256] = num;
}

//###############################################################
// Commands functions
//###############################################################

void Read() {
  //Get Parameters
  chipType = (int) cmdbuf[0] - 48;
  long addr = ((long) cmdbuf[3] * 256) + (long) cmdbuf[4];
  Last_Address = addr;
  
  //Read
  read_start();
  for (int x = 0; x < BUFFERSIZE; ++x) {
    buffer[x] = read_byte(addr+x);
    delayMicroseconds(100);
  }
  read_end();

  //return Array+Checksum to say it passed
  ChecksumThis();
  Serial.write(buffer, sizeof(buffer));
}

void Write() {
  //Get Parameters
  chipType = (int) cmdbuf[0] - 48;
  long addr = ((long) cmdbuf[3] * 256) + (long) cmdbuf[4];
  Last_Address = addr;

  //Write
  write_start();
  for (int x = 0; x < BUFFERSIZE; ++x) fast_write(addr + x, cmdbuf[x+5]);
  write_end();

  //return 0x4F (79) to say it passed
  Serial.write(0x4F);
}

void EraseSST() {
  //chipType = (int) cmdbuf[0] - 48;
  chipType = 5;
  set_ce(HIGH);
  set_oe(HIGH);
  set_vh(HIGH);
  set_vpp(HIGH);
  delay(1);
  
  //erase pulse
  set_ce(LOW);
  delay(350);
  set_ce(HIGH);
  delayMicroseconds(1);

  //Turning Off
  set_vh(LOW);
  set_vpp(LOW);
  delayMicroseconds(1);
  
  //return 0x4F (79) to say it passed
  Serial.write(0x4F);
}

//###############################################################
// COMMANDS SUBS functions
//###############################################################

void read_start() {
  data_bus_input();
  //enable chip select
  set_ce(LOW);
  //enable output
  set_oe(LOW);

  //Set VPP to Low/High (27C2128, 27C256)
  if (chipType == CHIP27C128) {
      digitalWrite(A15VPP, LOW);
      Set_Output_At(15, HIGH);
  }
  else if (chipType == CHIP27C256 || chipType == CHIP28C64) {
      digitalWrite(A15VPP, HIGH);   //the 28C64 doesnt care but the 27C64 is VPP
  }
  else if (chipType == CHIP27C32) {
      digitalWrite(A15VPP, LOW);  //normally used for A15/VPP, this pin is not used on 24pin chips
      Set_Output_At(15, LOW);     //normally used for A14, this pin is not used on 24pin chips
      Set_Output_At(14, HIGH); //**** normally used for A13, this pin is now VCC (5v) ****
      Set_Output_At(13, LOW);     //normally used for A12, this pin is not used on 24pin chips
      delayMicroseconds(5);
  }
}

void read_end() {
  //disable output
  set_oe(HIGH);
  //disable chip select
  set_ce(HIGH);
  
  //Set VPP to Low/High (27C2128, 27C256)
  if (chipType == CHIP27C128) Set_Output_At(15, LOW);
  else if (chipType == CHIP27C256)  digitalWrite(A15VPP, LOW);
}  

inline byte read_byte(unsigned int address)
{
  set_address_bus(address);
  return read_data_bus();
}
 
void write_start() {
  firstWritePass = true;
  //disable output
  set_oe(HIGH);
  set_vpp(HIGH);
  //Set VPP to low on 29C256 (not 27C256/27SF256 as its read only)
  if (chipType == CHIP27C256){digitalWrite(A15VPP, LOW); delayMicroseconds(5);}
  
  data_bus_output();
}

void write_end() {
  set_vpp(LOW);
  data_bus_input();
}

inline void fast_write(unsigned int address, byte data)
{
  if (chipType == CHIP28C64 || chipType == CHIP28C256) {
    //this function uses /DATA polling to get the end of the page write cycle. This is much faster than waiting 10ms
    static unsigned int lastAddress = 0;
    static byte lastData = 0;
    
    //enable chip select
    set_ce(LOW);

    //data poll
    if (((lastAddress ^ address) & 0xFFC0 || chipType == CHIP28C64) && !firstWritePass)
    {
      unsigned long startTime = millis();

      //poll data until data matches
      data_bus_input();
      set_oe(LOW);

      //set timeout here longer than JBurn timeout
      while(lastData != read_data_bus()) {
        if (millis() - startTime > 3000) return false;
      }
      
      set_oe(HIGH);
      delayMicroseconds(1);
      data_bus_output();
    }

    //set address and data for write
    set_address_bus(address);
    write_data_bus(data);
    delayMicroseconds(1);
 
    //strobe write
    set_we(LOW);
    set_we(HIGH);
    //disable chip select
    set_ce(HIGH);

    lastAddress = address;
    lastData = data;
    firstWritePass = false;
  }
  else 
  {
    set_address_bus(address);
    write_data_bus(data);
    delayMicroseconds(1);
  
    //programming pulse
    set_ce(LOW);
    delayMicroseconds(100); // for W27E512, works for 27SF512 also (but 27SF512 should be 20ms)
    set_ce(HIGH);
    delayMicroseconds(1);
  }
}

//###############################################################
// DATA BUS functions
//###############################################################

void data_bus_input() {
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
}

void data_bus_output() {
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
}

byte read_data_bus()
{
 byte b = 0;
  if (digitalRead(D0) == HIGH) b |= 1;
  if (digitalRead(D1) == HIGH) b |= 2;
  if (digitalRead(D2) == HIGH) b |= 4;
  if (digitalRead(D3) == HIGH) b |= 8;
  if (digitalRead(D4) == HIGH) b |= 16;
  if (digitalRead(D5) == HIGH) b |= 32;
  if (digitalRead(D6) == HIGH) b |= 64;
  if (digitalRead(D7) == HIGH) b |= 128;
  return(b);
}

inline void write_data_bus(byte data)
{
  digitalWrite(D0, data & 1);
  digitalWrite(D1, data & 2);
  digitalWrite(D2, data & 4);
  digitalWrite(D3, data & 8);
  digitalWrite(D4, data & 16);
  digitalWrite(D5, data & 32);
  digitalWrite(D6, data & 64);
  digitalWrite(D7, data & 128);
}

//###############################################################
// FAST SWIFT functions
//###############################################################
inline void set_address_bus(unsigned int address)
{
  byte hi, low;
  hi = (address >> 8);
  low = address & 0xff;
  //A14 become WE on 28C64 && 28C256, make sure dont use the output that were generally used for A14
  if (chipType == CHIP28C64 || chipType == CHIP28C256) bitSet(hi, 6); //set ouput 7 on hi byte
  else if (chipType == CHIP27C32) {
    bitClear(hi, 6);  //set ouput 7 on hi byte
    bitSet(hi, 5);    //set ouput 6 on hi byte, this is now VCC
    bitClear(hi, 4);  //set ouput 5 on hi byte
  }
  
  ApplyShiftAt(hi, low);
  
  digitalWrite(A10, (address & 1024)?HIGH:LOW );
  if (chipType == CHIP28C64 || chipType == CHIP28C256) digitalWrite(A15VPP, (address & 16384)?HIGH:LOW);  //A15/VPP become A14 on 28C64 && 28C256
  else if (chipType == CHIP27SF512) digitalWrite(A15VPP, (address & 32768)?HIGH:LOW);
}

inline void Set_Output_At(unsigned int Position, bool IsHigh)
{
  byte hi, low;
  hi = (Last_Address >> 8);
  low = Last_Address & 0xff;
  if (Position >= 8) {
    if (IsHigh) bitSet(hi, Position - 8);
    else  bitClear(hi, Position - 8);
  }
  else {
    if (IsHigh) bitSet(low, Position);
    else  bitClear(low, Position);
  }
  ApplyShiftAt(hi, low);
}

void ApplyShiftAt(byte hi, byte low)
{
  fastShiftOut(hi);
  fastShiftOut(low);
  //strobe latch line
  bitSet(STROBE_PORT,STROBE_LATCH);
  bitClear(STROBE_PORT,STROBE_LATCH);
  delayMicroseconds(1);
}

void fastShiftOut(byte data) {
  //clear
  bitClear(STROBE_PORT,STROBE_DS);

  //Loop for the 8x outputs
  for (int i=7; i>=0; i--) {
    //clear clock pin
    bitClear(STROBE_PORT,STROBE_CLOCK);

    //Enable/Disable pin Output
    if (bitRead(data,i) == 1) bitSet(STROBE_PORT,STROBE_DS); 
    else  bitClear(STROBE_PORT,STROBE_DS);
    
    //register shifts bits on upstroke of clock pin  
    bitSet(STROBE_PORT,STROBE_CLOCK);
    //clear after shift to prevent bleed through
    bitClear(STROBE_PORT,STROBE_DS);
  }
  
  //stop shifting
  bitClear(STROBE_PORT,STROBE_CLOCK);
}

int GetAddress(unsigned int Position)
{
  int Address = 0;
  if (Position == 1) Address = 1;
  if (Position == 2) Address = 2;
  if (Position == 3) Address = 4;
  if (Position == 4) Address = 8;
  if (Position == 5) Address = 16;
  if (Position == 6) Address = 32;
  if (Position == 7) Address = 64;
  if (Position == 8) Address = 128;
  if (Position == 9) Address = 256;
  if (Position == 10) Address = 512;
  if (Position == 11) Address = 1024; //This pin is not wired to anything on the BMBurner, refer to pin D13 on arduino instead
  if (Position == 12) Address = 2048;
  if (Position == 13) Address = 4096;
  if (Position == 14) Address = 8192;
  if (Position == 15) Address = 16384;
  if (Position == 16) Address = 32768; //This pin is not wired to anything on the BMBurner, refer to pin A5 on arduino instead
  return Address;
}

//###############################################################
// PINS functions
//###############################################################

//**attention, this line is LOW - active**
inline void set_oe (byte state)
{
  digitalWrite(OE, state);
}
 
//**attention, this line is LOW - active**
inline void set_ce (byte state)
{
  digitalWrite(CE, state);
}

//**attention, this line is LOW - active**
inline void set_we (byte state)
{
  if (chipType == CHIP28C64 || chipType == CHIP28C256) Set_Output_At(15, state);  //output 15 become WE (since there are 8 outputs by 74HC595, its the #7 ouputs on the 2nd 74HC595)
}

//Boost VPP 12V
void set_vpp (byte state)
{
  switch (chipType) {
  case CHIP27SF512:
    digitalWrite(VPP, state);
    break;
  default:
    break;
  }
}

//Boost Erase 12V
void set_vh (byte state)
{
  switch (chipType) {
  case CHIP27SF512:
    digitalWrite(VH, state);
    break;
  default:
    break;
  }
}
