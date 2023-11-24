// DCC Accesory Decode based on the NMRA DCC Accessory Example Code
// For use with an Arduino Uno or Mega
// The output pins will not drive the snap switch
// The output pins are meant to drive the inputs of a solid state relay board.
// Example Relay Board: SainSmart 8-Channel 5V Solid State Relay Module Board for Arduino Uno Duemilanove MEGA2560 MEGA1280 ARM DSP PIC

#include <NmraDcc.h>
NmraDcc  Dcc ;
DCC_MSG  Packet ;

// Define the Arduino input Pin number for the DCC Signal 
#define DCC_PIN     2

// Enter the number of DCC accessories / addresses controlled by this Arduino
#define NUMACCESSORIES 11

// Enter the millisecond delay between engaging the relaye and disengaging the relay
#define RELAYTIME 100

struct CVPair
{
  uint16_t  CV;
  uint8_t   Value;
};

typedef struct DCCAccessoryData {
  uint16_t  address;        // User Configurable DCC address
  byte      closepin;       // User Configurable Arduino pin to close the switch
  byte      openpin;        // User Configurable Arduino pin to throw the switch
  uint8_t   direction;       // Internal use DCC state of accessory, 1=on, 0=off
  uint8_t   previousDirection;  // Internal use DCC previous state of accessory, 1=on, 0=off
  uint8_t   isSwitch;  // Internal use DCC previous state of accessory, 1=on, 0=off
};

// initialize the array of accessories
DCCAccessoryData accessory[NUMACCESSORIES];

CVPair FactoryDefaultCVs [] =
{
  {CV_ACCESSORY_DECODER_ADDRESS_LSB, DEFAULT_ACCESSORY_DECODER_ADDRESS & 0xFF},
  {CV_ACCESSORY_DECODER_ADDRESS_MSB, DEFAULT_ACCESSORY_DECODER_ADDRESS >> 8},
};

uint8_t FactoryDefaultCVIndex = 0;

void notifyCVResetFactoryDefault()
{
  // Make FactoryDefaultCVIndex non-zero and equal to num CV's to be reset 
  // to flag to the loop() function that a reset to Factory Defaults needs to be done
  FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs)/sizeof(CVPair);
};

const int DccAckPin = 3 ;

// This function is called by the NmraDcc library when a DCC ACK needs to be sent
// Calling this function should cause an increased 60ma current drain on the power supply for 6ms to ACK a CV Read 
void notifyCVAck(void)
{
  Serial.println("notifyCVAck") ;
  
  digitalWrite( DccAckPin, HIGH );
  delay( 6 );  
  digitalWrite( DccAckPin, LOW );
}

// Uncomment to print all DCC Packets
//#define NOTIFY_DCC_MSG
#ifdef  NOTIFY_DCC_MSG
void notifyDccMsg( DCC_MSG * Msg)
{
  Serial.print("notifyDccMsg: ") ;
  for(uint8_t i = 0; i < Msg->Size; i++)
  {
    Serial.print(Msg->Data[i], HEX);
    Serial.write(' X ');
  }
  Serial.println();
}
#endif

// This function is called whenever a normal DCC Turnout Packet is received and we're in Board Addressing Mode
void notifyDccAccTurnoutBoard( uint16_t BoardAddr, uint8_t OutputPair, uint8_t Direction, uint8_t OutputPower )
{
  Serial.print("notifyDccAccTurnoutBoard: ") ;
  Serial.print(BoardAddr,DEC) ;
  Serial.print(',');
  Serial.print(OutputPair,DEC) ;
  Serial.print(',');
  Serial.print(Direction,DEC) ;
  Serial.print(',');
  Serial.println(OutputPower, HEX) ;
}

// This function is called whenever a normal DCC Turnout Packet is received and we're in Output Addressing Mode
void notifyDccAccTurnoutOutput( uint16_t Addr, uint8_t Direction, uint8_t OutputPower )
{
  // loop through the
  for(int i=0; i<NUMACCESSORIES; i++) {
    if(accessory[i].address == Addr){
      if(accessory[i].isSwitch){
        Serial.print("changing switch ");
        Serial.println(accessory[i].address);
        if(1==Direction && accessory[i].previousDirection != Direction){
          Serial.print("notifyDccAccTurnoutOutput: ") ;
          Serial.print(Addr,DEC) ;
          Serial.print(',');
          Serial.print(Direction,DEC) ;
          Serial.print(',');
          Serial.println(accessory[i].previousDirection);
          digitalWrite(accessory[i].closepin, 1);
          delay(RELAYTIME);
          digitalWrite(accessory[i].closepin, 0);
          accessory[i].previousDirection = Direction;
        } else if(0==Direction && accessory[i].previousDirection != Direction){
          Serial.print("notifyDccAccTurnoutOutput: ") ;
          Serial.print(Addr,DEC) ;
          Serial.print(',');
          Serial.print(Direction,DEC) ;
          Serial.print(',');
          Serial.println(accessory[i].previousDirection);
          digitalWrite(accessory[i].openpin, 1);
          delay(RELAYTIME);
          digitalWrite(accessory[i].openpin, 0);
          accessory[i].previousDirection = Direction;
        }
      }
      else{
        Serial.print("changing light ");
        Serial.println(accessory[i].address);
        Serial.print("notifyDccAccTurnoutOutput: ") ;
        Serial.print(Addr,DEC) ;
        Serial.print(',');
        Serial.print(Direction,DEC) ;
        Serial.print(',');
        Serial.println(accessory[i].previousDirection);
        digitalWrite(accessory[i].closepin, Direction);
      }
    }
  }  
}

// This function is called whenever a DCC Signal Aspect Packet is received
void notifyDccSigOutputState( uint16_t Addr, uint8_t State)
{
  Serial.print("notifyDccSigOutputState: ") ;
  Serial.print(Addr,DEC) ;
  Serial.print(',');
  Serial.println(State, HEX) ;
}

void setup()
{
  Serial.begin(9600);
  uint8_t maxWaitLoops = 255;
  while(!Serial && maxWaitLoops--)
    delay(20);
      
  // Configure the DCC CV Programing ACK pin for an output
  pinMode( DccAckPin, OUTPUT );
  // pinMode(4, OUTPUT);
  // pinMode(5, OUTPUT);
  // pinMode(6, OUTPUT);
  // pinMode(7, OUTPUT);
  
  // Assign DCC Address and Output pins for each decoder
  // This could possibly be accomplished in a loop statement if your DCC addresses and output pins are sequential.
  accessory[0].address = 122;
  accessory[0].openpin = 22; // Arduino pin
  accessory[0].closepin = 23; // Arduino pin
  accessory[0].direction = HIGH;
  accessory[0].previousDirection = LOW;
  accessory[0].isSwitch = HIGH;

  accessory[1].address = 123;
  accessory[1].openpin = 24; // Arduino pin
  accessory[1].closepin = 25; // Arduino pin
  accessory[1].direction = HIGH;
  accessory[1].previousDirection = LOW;
  accessory[1].isSwitch = HIGH;

  accessory[2].address = 124;
  accessory[2].openpin = 26; // Arduino pin
  accessory[2].closepin = 27; // Arduino pin
  accessory[2].direction = HIGH;
  accessory[2].previousDirection = LOW;
  accessory[2].isSwitch = HIGH;

  accessory[3].address = 125;
  accessory[3].openpin = 28; // Arduino pin
  accessory[3].closepin = 29; // Arduino pin
  accessory[3].direction = HIGH;
  accessory[3].previousDirection = LOW;
  accessory[3].isSwitch = HIGH;

  accessory[4].address = 126;
  accessory[4].openpin = 30; // Arduino pin
  accessory[4].closepin = 31; // Arduino pin
  accessory[4].direction = HIGH;
  accessory[4].previousDirection = LOW;
  accessory[4].isSwitch = HIGH;

  accessory[5].address = 127;
  accessory[5].openpin = 32; // Arduino pin
  accessory[5].closepin = 33; // Arduino pin
  accessory[5].direction = HIGH;
  accessory[5].previousDirection = LOW;
  accessory[5].isSwitch = HIGH;

  accessory[6].address = 128;
  accessory[6].openpin = 34; // Arduino pin
  accessory[6].closepin = 35; // Arduino pin
  accessory[6].direction = HIGH;
  accessory[6].previousDirection = LOW;
  accessory[6].isSwitch = HIGH;

  accessory[7].address = 129;
  accessory[7].openpin = 36; // Arduino pin
  accessory[7].closepin = 37; // Arduino pin
  accessory[7].direction = HIGH;
  accessory[7].previousDirection = LOW;
  accessory[7].isSwitch = HIGH;

  accessory[8].address = 130;
  accessory[8].openpin = 38; // Arduino pin
  accessory[8].closepin = 39; // Arduino pin
  accessory[8].direction = HIGH;
  accessory[8].previousDirection = LOW;
  accessory[8].isSwitch = HIGH;

  accessory[9].address = 131;
  accessory[9].openpin = 40; // Arduino pin
  accessory[9].closepin = 41; // Arduino pin
  accessory[9].direction = HIGH;
  accessory[9].previousDirection = LOW;
  accessory[9].isSwitch = HIGH;

  accessory[10].address = 88;
  accessory[10].openpin = 42; // Arduino pin
  accessory[10].closepin = 42; // Arduino pin
  accessory[10].direction = HIGH;
  accessory[10].previousDirection = LOW;
  accessory[10].isSwitch = HIGH;

  Serial.println("read accessory settings begin: ");
  for(int i=0; i<NUMACCESSORIES; i++) {
    // write all conditions to the monitor
    Serial.println("--------------------");
    Serial.print("accessory setting: ");
    Serial.println(i);
    Serial.print("accessory address: ");
    Serial.println(accessory[i].address);
    Serial.print("accessory openpin:");
    Serial.println(accessory[i].openpin);
    Serial.print("accessory closepin:");
    Serial.println(accessory[i].closepin);
    Serial.print("accessory direction:"); 
    Serial.println(accessory[i].direction);
    Serial.print("accessory previousDirection:");
    Serial.println(accessory[i].previousDirection);
    
    pinMode(accessory[i].closepin, OUTPUT);
    pinMode(accessory[i].openpin, OUTPUT);
  }  
    Serial.println("end reading accessory settings and setting pin modes");
  // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up
  // Many Arduino Cores now support the digitalPinToInterrupt() function that makes it easier to figure out the
  // Interrupt Number for the Arduino Pin number, which reduces confusion. 
#ifdef digitalPinToInterrupt
  Dcc.pin(DCC_PIN, 0);
#else
  Dcc.pin(0, DCC_PIN, 1);
#endif
  
  // Call the main DCC Init function to enable the DCC Receiver
  Dcc.init( MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0 );

  Serial.println("Init Done");
}

void loop()
{
  // You MUST call the NmraDcc.process() method frequently from the Arduino loop() function for correct library operation
  Dcc.process();
  
  if( FactoryDefaultCVIndex && Dcc.isSetCVReady())
  {
    FactoryDefaultCVIndex--; // Decrement first as initially it is the size of the array 
    Dcc.setCV( FactoryDefaultCVs[FactoryDefaultCVIndex].CV, FactoryDefaultCVs[FactoryDefaultCVIndex].Value);
  }
}
