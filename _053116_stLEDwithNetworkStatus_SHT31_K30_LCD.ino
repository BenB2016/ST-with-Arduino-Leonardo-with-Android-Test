//*****************************************************************************
/// @file
/// @brief
///   Arduino SmartThings Shield LED Example with Network Status
/// @note
///
/// TKLCD with Leonardo CPU and attached LCD
/// SHT31 Humidity and Temperature sensor on I2C address: 0x44
/// K30 CO2 Sensor on I2C Address: 0x68
/// Hard wired the Arduino Smart Things Shield to TKLCD Hardware Serial Port
/// Sketch uses 19,826 bytes (69%) of program storage space. Maximum is 28,672 bytes.
/// Global variables use 1,319 bytes (44%) of dynamic memory, leaving 1,241 bytes for
/// local variables. Maximum is 2,560 bytes.
///
///
//*****************************************************************************
#include <SoftwareSerial.h>   //TODO need to set due to some weird wire language linker, should we absorb this whole library into smartthings
#include <SmartThings.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <TKLCD.h>
#include "Adafruit_SHT31.h"
#include <avr/pgmspace.h>
Adafruit_SHT31 sht31 = Adafruit_SHT31();

TKLCD_Local lcd = TKLCD_Local(); // when programming a TKLCD module itself

//*****************************************************************************
// Pin Definitions    | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                    V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
#define PIN_LED         5

// this transmit to phone 05/31/16 4:53 pm
#define PIN_THING_RX    0 // RX for Leonardo on TKLCD
#define PIN_THING_TX    1 // TX for Leonardo on TKLCD

//*****************************************************************************
// Global Variables   | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                    V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
SmartThingsCallout_t messageCallout;    // call out function forward decalaration
SmartThings smartthing(PIN_THING_RX, PIN_THING_TX, messageCallout);  // constructor

bool isDebugEnabled;    // enable or disable debug in this example
int stateLED;           // state to track last set value of LED
int stateNetwork;       // state of the network
int co2Addr = 0x68;
int ti = 0;
int hi = 0;
String humiditySHT31Message;
String temperatureSHT31Message;
String k30CO2Message;
String message;
String strti;
String strhi;
String strco2;

//*****************************************************************************
// Local Functions  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                  V V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
void on()
{
  stateLED = 1;                 // save state as 1 (on)
  digitalWrite(PIN_LED, HIGH);  // turn LED on
  smartthing.shieldSetLED(0, 0, 2);
  smartthing.send("on");        // send message to cloud
}

//*****************************************************************************
void off()
{
  stateLED = 0;                 // set state to 0 (off)
  digitalWrite(PIN_LED, LOW);   // turn LED off
  smartthing.shieldSetLED(0, 0, 0);
  smartthing.send("off");       // send message to cloud
}

//*****************************************************************************
void setNetworkStateLED()
{
  SmartThingsNetworkState_t tempState = smartthing.shieldGetLastNetworkState();
  if (tempState != stateNetwork)
  {
    switch (tempState)
    {
      case STATE_NO_NETWORK:
        if (isDebugEnabled) Serial.println("NO_NETWORK");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      case STATE_JOINING:
        if (isDebugEnabled) Serial.println("JOINING");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      case STATE_JOINED:
        if (isDebugEnabled) Serial.println("JOINED");
        smartthing.shieldSetLED(0, 0, 0); // off
        break;
      case STATE_JOINED_NOPARENT:
        if (isDebugEnabled) Serial.println("JOINED_NOPARENT");
        smartthing.shieldSetLED(2, 0, 2); // purple
        break;
      case STATE_LEAVING:
        if (isDebugEnabled) Serial.println("LEAVING");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      default:
      case STATE_UNKNOWN:
        if (isDebugEnabled) Serial.println("UNKNOWN");
        smartthing.shieldSetLED(0, 2, 2); // blue
        break;
    }
    stateNetwork = tempState;
  }
}

//*****************************************************************************
// API Functions    | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
//                  V V V V V V V V V V V V V V V V V V V V V V V V V V V V V V
//*****************************************************************************
void setup() {

  int co2_value = 777;
  // setup default state of global variables
  isDebugEnabled = true;
  stateLED = 0;                 // matches state of hardware pin set below
  stateNetwork = STATE_JOINED;  // set to joined to keep state off if off
  setNetworkStateLED;

  // setup hardware pins
  pinMode(PIN_LED, OUTPUT);     // define PIN_LED as an output
  digitalWrite(PIN_LED, LOW);   // set value to LOW (off) to match stateLED=0

  lcd.begin();
  lcd.clear();
  lcd.setCursor(0, 0);
  Wire.begin ();
  sht31.begin(0x44);   // Set to 0x45 for alternate i2c addr

  if (isDebugEnabled)
  { // setup debug serial port
    Serial.begin(9600);         // setup serial with a baud rate of 9600
    //  while (!Serial);     // will pause Leonardo, etc until one clicks to open the serial console
    // also will not proceed with stand alone LCD without plugging in USB, so cannot use
    // for stand alone operation

    Serial.println("setup..");  // print out 'setup..' on start
    setNetworkStateLED;
  }
}

///////////////////////////////////////////////////////////////////
// Function : int readCO2()
// Returns : CO2 Value upon success, 0 upon checksum failure
// Assumes : - Wire library has been imported successfully.
// - LED is connected to IO pin D5
// - CO2 sensor address is defined in co2_addr
///////////////////////////////////////////////////////////////////
int readCO2()
{
  int co2_value = 0;
  // We will store the CO2 value inside this variable.
  digitalWrite(5, HIGH);
  // On most Arduino platforms this pin is used as an indicator light.
  //////////////////////////
  /* Begin Write Sequence */
  //////////////////////////
  Wire.beginTransmission(co2Addr);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x08);
  Wire.write(0x2A);

  Wire.endTransmission();
  /////////////////////////
  /* End Write Sequence. */
  /////////////////////////
  /*
  We wait 10ms for the sensor to process our command.
  The sensors's primary duties are to accurately
  measure CO2 values. Waiting 10ms will ensure the
  data is properly written to RAM
  */
  delay(10);
  /////////////////////////
  /* Begin Read Sequence */
  /////////////////////////
  /*
  Since we requested 2 bytes from the sensor we must
  read in 4 bytes. This includes the payload, checksum,
  and command status byte.
  */
  Wire.requestFrom(co2Addr, 4);
  byte i = 0;
  byte buffer[4] = {0, 0, 0, 0};
  /*
  Wire.available() is not nessessary. Implementation is obscure but we leave
  it in here for portability and to future proof our code
  */
  while (Wire.available())
  {
    buffer[i] = Wire.read();
    i++;
  }
  ///////////////////////
  /* End Read Sequence */
  ///////////////////////
  /*
  Using some bitwise manipulation we will shift our buffer
  into an integer for general consumption
  */
  co2_value = 0;
  co2_value |= buffer[1] & 0xFF;
  co2_value = co2_value << 8;
  co2_value |= buffer[2] & 0xFF;
  byte sum = 0; //Checksum Byte
  sum = buffer[0] + buffer[1] + buffer[2]; //Byte addition utilizes overflow

  if (sum == buffer[3])
  {
    // Success!
    digitalWrite(5, LOW);
    return co2_value;
  }
  else
  {
    // Failure!
    /*
    Checksum failure can be due to a number of factors,
    fuzzy electrons, sensor busy, etc.
    */
    digitalWrite(5, LOW);
    return 0;
  }
}


//*****************************************************************************
void loop() {
  float t = sht31.readTemperature();
  float tF = (t * 9 / 5) + 32;
  float h = sht31.readHumidity();

  ti = int(tF); // degrees Fahrenheit
  hi = int(h); // % humidity

  String strti = String(ti);
  String strhi = String(hi);

  // LCD print to second line of 16 x 2
  lcd.setCursor(0, 1);

  if (ti < 100)
  {
    // if Temperature less than 100 deg F add an extra space
    lcd.print("TEMP "); // first 5 characters
  }
  else
  {
    // if Temperature  greater than 100 deg F del an extra space
    lcd.print("TEMP"); // first 4 characters
  }
  lcd.print(ti); // temperature value
  lcd.print((char)223); // degree symbol

  if (hi < 10)
  {
    // if Humidity less than 10 % add an extra space
    lcd.print("F RH  "); // first 5 characters
  }
  else
   {
    // if Humidity Greater than 10 % del an extra space
    lcd.print("F RH ");
   }
 
    lcd.print(hi);
    lcd.print("%");

    delay(400);

    int co2Value = readCO2();
    delay (200);
    String strco2 = String(co2Value);

    lcd.setCursor(0, 0);
    if (co2Value < 1000)
    {
      // if CO2 PPM count less than 1000 add an extra space
      lcd.print("  CO2  "); // first 7 characters
    }
    else
    {
      // if CO2 PPM count greater than 1000 del an extra space
      lcd.print("  CO2 "); // first 6 characters
    }
    lcd.print(co2Value);
    lcd.print(" PPM  ");

 
   
   // run smartthing logic


    smartthing.run();

 //   Serial.println(" ");
//    Serial.println("In the loop");
    //    digitalWrite(PIN_LED, HIGH);   // set value to LOW (off) to match stateLED=1
    //    delay(500);
    //    digitalWrite(PIN_LED, LOW);   // set value to LOW (off) to match stateLED=0
    //    delay(3000);
    

    Serial.println("Reporting data...");
    Serial.println("Messages to be delivered to SmartThings");
 
    
    // Report Temperature, Humidity from the SHT31, and CO2 Level in PPM form the K30

    String humiditySHT31Message = "A";
    humiditySHT31Message.concat(strhi);
    smartthing.send(humiditySHT31Message);
    Serial.println(humiditySHT31Message);
    //    Serial.println(strhi);
   delay(500);

    String temperatureSHT31Message = "B";
    temperatureSHT31Message.concat(strti);
    smartthing.send(temperatureSHT31Message);
    Serial.println(temperatureSHT31Message);
    //    Serial.println(strti);

   delay(500);
   
    String k30CO2Message = "C";
    k30CO2Message.concat(strco2);
    smartthing.send(k30CO2Message);
    Serial.println(k30CO2Message);
    //   Serial.println(strco2);

    setNetworkStateLED();
    delay(9000);
    //Serial.println(F(" "));

   }

  //*****************************************************************************
  void messageCallout(String message)
  {
    // if debug is enabled print out the received message
    if (isDebugEnabled)
    {
      Serial.print("Received message: '");
      Serial.print(message);
      Serial.println("' ");
    }

    // if message contents equals to 'on' then call on() function
    // else if message contents equals to 'off' then call off() function
    if (message.equals("on"))
    {
      on();
    }
    else if (message.equals("off"))
    {
      off();
    }
    else if (message.equals("poll"))
    {
    //reportData();
    // hello();
    Serial.println("Poll Request ");
    Serial.println(" ");
    }
  }

  //*****************************************************************************
