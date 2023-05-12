#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>

#define DEBUG

// Define SS pins
#define SS_PIN_1 2
#define SS_PIN_2 3
#define SS_PIN_3 4

// Define reset pin
#define RST_PIN 9

// Define pin for re-writing data
#define REWRITE_PIN 6

// Define lock pin
#define LOCK_PIN 5

// Define addresses for keys in EEPROM
int key1_address[3] = {0, 1, 2};
int key2_address[3] = {3, 4, 5};
int key3_address[3] = {6, 7, 8};

// Define arrays for original keys
int key1_original[3];
int key2_original[3];
int key3_original[3];

// Define arrays for keys
int key1[3];
int key2[3];
int key3[3];

// Create instances of the RFID reader
MFRC522 mfrc522[3] = {
    MFRC522(SS_PIN_1, RST_PIN),
    MFRC522(SS_PIN_2, RST_PIN),
    MFRC522(SS_PIN_3, RST_PIN)};

// Forward declarations of the functions
bool checkAllKeysMatch();
void writeToMemory();
void readCardData(int card_number);

// Setup function runs once when the Arduino is powered on or reset
void setup()
{
    // Begin serial communication at 9600 baud rate
    Serial.begin(9600);

    // Set re-write pin as INPUT and lock pin as OUTPUT
    pinMode(REWRITE_PIN, INPUT);
    pinMode(LOCK_PIN, OUTPUT);

    // Begin SPI communication
    SPI.begin();

    // Initialize the RFID reader for each SS pin
    for (int i = 0; i < 3; i++)
    {
        mfrc522[i].PCD_Init();
// Dump version info to serial for debugging
#ifdef DEBUG
        mfrc522[i].PCD_DumpVersionToSerial();
#endif
    }

    // Check if rewrite pin is LOW, if yes, then run write mode
    if (digitalRead(REWRITE_PIN) == LOW)
    {
        writeToMemory();
    }

    // Read original keys from EEPROM
    for (int i = 0; i < 3; i++)
    {
        key1_original[i] = EEPROM.read(key1_address[i]);
        key2_original[i] = EEPROM.read(key2_address[i]);
        key3_original[i] = EEPROM.read(key3_address[i]);
    }
}

// The loop function runs over and over again forever
void loop()
{
    // Continuously check each card for new data
    for (int i = 0; i < 3; i++)
    {
        while (!(mfrc522[i].PICC_IsNewCardPresent() && mfrc522[i].PICC_ReadCardSerial()))
        {
            clearAllData();
        }
        readCardData(i + 1);
        Serial.println("Card " + String(i + 1) + " read");
    }

    // Check if all keys match their original counterparts
    if (checkAllKeysMatch())
    {
        Serial.println("All Cards Matched");
        // If all keys match, unlock for a period of time then lock again
        digitalWrite(LOCK_PIN, HIGH);
        delay(40000);
        digitalWrite(LOCK_PIN, LOW);
    }
    // Clear all data after checking
    clearAllData();
}

// Function to check if all keys match their original counterparts
bool checkAllKeysMatch()
{
    for (int i = 0; i < 3; i++)
    {
        if (key1[i] != key1_original[i] || key2[i] != key2_original[i] || key3[i] != key3_original[i])
        {
            return false;
        }
    }
    return true;
}

// Function to write new key data into EEPROM
void writeToMemory()
{
#ifdef DEBUG
    Serial.println("Write Mode");
#endif

    // Check each card once for new data
    for (int i = 0; i < 3; i++)
    {
        if (mfrc522[i].PICC_IsNewCardPresent() && mfrc522[i].PICC_ReadCardSerial())
        {
            EEPROM.write(key1_address[i], mfrc522[i].uid.uidByte[0]);
            EEPROM.write(key2_address[i], mfrc522[i].uid.uidByte[1]);
            EEPROM.write(key3_address[i], mfrc522[i].uid.uidByte[2]);
            Serial.println("Card " + String(i + 1) + " Done");
        }
    }

#ifdef DEBUG
    Serial.println("Write Mode Done");
#endif
}

// Function to read card data
void readCardData(int card_number)
{
    if (card_number == 1)
    {
        for (int i = 0; i < 3; i++)
        {
            key1[i] = mfrc522[card_number - 1].uid.uidByte[i];
        }
    }
    else if (card_number == 2)
    {
        for (int i = 0; i < 3; i++)
        {
            key2[i] = mfrc522[card_number - 1].uid.uidByte[i];
        }
    }
    else if (card_number == 3)
    {
        for (int i = 0; i < 3; i++)
        {
            key3[i] = mfrc522[card_number - 1].uid.uidByte[i];
        }
    }
}

void clearAllData()
{
    // Halt card reading and clear data for each card reader
    for (int i = 0; i < 3; i++)
    {
        mfrc522[i].PICC_HaltA();
        for (int j = 0; j < 3; j++)
        {
            mfrc522[i].uid.uidByte[j] = 0;
        }
    }

    // Clear stored key data
    for (int i = 0; i < 3; i++)
    {
        key1[i] = 0;
        key2[i] = 0;
        key3[i] = 0;
    }
}
