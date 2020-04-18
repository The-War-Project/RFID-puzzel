#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

//DEBUG message
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print (x)
#else
#define DEBUG_PRINT(x)
#endif

#ifdef DEBUG
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINTLN(x)
#endif

const uint32_t NUM_READERS = 8;        //Adjust number as needed 1 to 4

//Hardware GPIO pin assignments
const uint32_t PN532_SCK_PIN =  13;
const uint32_t PN532_MISO_PIN = 12;
const uint32_t PN532_MOSI_PIN = 11;
const uint32_t PN532_SS1_PIN =  3;
const uint32_t PN532_SS2_PIN =  4;
const uint32_t PN532_SS3_PIN =  5;
const uint32_t PN532_SS4_PIN =  6;
const uint32_t PN532_SS5_PIN = 7;
const uint32_t PN532_SS6_PIN = 8;
const uint32_t PN532_SS7_PIN = 9;
const uint32_t PN532_SS8_PIN = 10;

//The tags that will allow device to open
const uint32_t ACCEPTED_TAG_ID1 = 3872679160;
const uint32_t ACCEPTED_TAG_ID2 = 3584511166;
const uint32_t ACCEPTED_TAG_ID3 = 3584511167;
const uint32_t ACCEPTED_TAG_ID4 = 0;
const uint32_t ACCEPTED_TAG_ID5 = 3872679160;
const uint32_t ACCEPTED_TAG_ID6 = 3872679160;
const uint32_t ACCEPTED_TAG_ID7 = 3872679160;
const uint32_t ACCEPTED_TAG_ID8 = 3872679160;

//PWM values for device locked (LOW) and device opened (HIGH)
const uint32_t PWM_LOW_VAL = 0;
const uint32_t PWM_HIGH_VAL = 255;

const uint32_t DELAY_AMT = 1000;
bool isLocked = true; // Init system state as locked

//An array of the chip select pins for each RFID reader
const uint32_t SS_PINS[NUM_READERS] = {PN532_SS1_PIN, PN532_SS2_PIN, PN532_SS3_PIN, PN532_SS4_PIN, PN532_SS5_PIN, PN532_SS6_PIN, PN532_SS7_PIN, PN532_SS8_PIN};  

//An array of the accepted RFID tags for each RFID reader
const uint32_t NFC_ACCEPTED_TAGS[NUM_READERS] = {ACCEPTED_TAG_ID1, ACCEPTED_TAG_ID2, ACCEPTED_TAG_ID3, ACCEPTED_TAG_ID4, ACCEPTED_TAG_ID5, ACCEPTED_TAG_ID6, ACCEPTED_TAG_ID7, ACCEPTED_TAG_ID8};   

//An array of RFID readers
Adafruit_PN532 *nfcReaders[NUM_READERS];


/**************************************************************************
 * Function: setup
 * Inputs:   None
 * Outputs:  None
 **************************************************************************/
void setup(void) {
  Serial.begin(115200);
  DEBUG_PRINTLN(F("Welcome to the Escape Room RFID Gadget v0.0.1!"));

  for (int x = 0; x < NUM_READERS; x++)
  {
    nfcReaders[x] = new Adafruit_PN532(PN532_SCK_PIN, PN532_MISO_PIN, PN532_MOSI_PIN, SS_PINS[x]);
    nfcReaders[x]->begin();
    //delay(DELAY_AMT*3);
    
    uint32_t versiondata = 9999;
    while (versiondata == 9999) {
      versiondata = nfcReaders[x]->getFirmwareVersion();
      DEBUG_PRINT(F("Didn't find PN53x board #"));
      DEBUG_PRINTLN(x);
    }
    
    // Got ok data, print it out!
    DEBUG_PRINT(F("Found chip PN5"));
    DEBUG_PRINTLN(String((versiondata >> 24) & 0xFF, HEX));
    DEBUG_PRINT(F("Firmware ver. "));
    DEBUG_PRINT(String((versiondata >> 16) & 0xFF, DEC));
    DEBUG_PRINT('.');
    DEBUG_PRINTLN(String((versiondata >> 8) & 0xFF, DEC));

    nfcReaders[x]->SAMConfig();
    nfcReaders[x]->setPassiveActivationRetries(1);
  }
      //Device is locked and ready to go!
      DEBUG_PRINTLN(F("SUCCESS! System initialized. Waiting for RFID tags..."));
}



/**************************************************************************
 * Function: loop
 * Inputs:   None
 * Outputs:  None
 **************************************************************************/
void loop(void) {
  if (isLocked == true)
  {
    uint32_t readNfcTags[NUM_READERS];
    uint32_t countOfMatchingTags = 0;

    for (int w = 0; w < NUM_READERS; w++)
    {
      readNfcTags[w] = 0;
    }

    for (int x = 0; x < NUM_READERS; x++) {
      uint8_t success;
      uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
      uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
      uint32_t cardid = 0;

      DEBUG_PRINT("READER #");
      DEBUG_PRINTLN(x);

      success = nfcReaders[x]->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

      if (success) {
        // Display some basic information about the card
        DEBUG_PRINT(F("Found an ISO14443A card with "));
        //DEBUG_PRINT(F("  UID Value: "));
        //nfcReaders[x]->PrintHex(uid, uidLength);

        if (uidLength == 4)
        {
          // We probably have a Mifare Classic card ...
          cardid = uid[0];
          cardid <<= 8;
          cardid |= uid[1];
          cardid <<= 8;
          cardid |= uid[2];
          cardid <<= 8;
          cardid |= uid[3];
          //DEBUG_PRINTLN(F("Seems to be a Mifare Classic card."));
        }
      }

      DEBUG_PRINT(F("tag ID#"));
      DEBUG_PRINTLN(cardid);
      readNfcTags[x] = cardid;
      delay(DELAY_AMT);
    }

    for (int z = 0; z < NUM_READERS; z++) {
      if (readNfcTags[z] == NFC_ACCEPTED_TAGS[z])
      {
        countOfMatchingTags++;
      }
    }
    /*
    if (countOfMatchingTags == NUM_READERS) {
      activateRelay();
    }*/
  }
}

/**************************************************************************
 * Function: activateRelay
 * Inputs:   None
 * Outputs:  None
 **************************************************************************/




void setDeviceOutputs()
{
  //Warning the device is ready to lock
  DEBUG_PRINTLN("Device locking in...");
  for (int x = 3; x >= 0; x--)  
  {
    //digitalWrite(OPEN_LED_PIN, HIGH);
    //digitalWrite(CLOSED_LED_PIN, HIGH);
    delay(DELAY_AMT);
    //digitalWrite(OPEN_LED_PIN, LOW);
    //digitalWrite(CLOSED_LED_PIN, LOW);
    DEBUG_PRINTLN(x);
  }

  //Initial state of outputs set
  //analogWrite(PWM_OUT_PIN, PWM_LOW_VAL);
  //digitalWrite(OPEN_LED_PIN, LOW);
  //digitalWrite(CLOSED_LED_PIN, HIGH);
}