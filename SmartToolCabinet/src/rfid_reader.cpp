// rfid_reader.cpp
#include "rfid_reader.h"

RFIDReader rfid;

RFIDReader::RFIDReader() 
    : mfrc522(RFID_SS_PIN, RFID_RST_PIN),
      last_uid(0) {
}

void RFIDReader::init() {
    Serial.println("初始化RFID模块...");
    
    SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
    mfrc522.PCD_Init();
    mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
    
    byte version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    Serial.printf("RFID模块版本: 0x%02X\n", version);
    
    if (version == 0x00 || version == 0xFF) {
        Serial.println("RFID模块未检测到!");
    } else {
        Serial.println("RFID模块初始化完成");
    }
}

bool RFIDReader::readUID(uint8_t* uid) {
    if (!mfrc522.PICC_IsNewCardPresent()) return false;
    if (!mfrc522.PICC_ReadCardSerial()) return false;
    
    memcpy(uid, mfrc522.uid.uidByte, 4);
    
    last_uid = ((uint32_t)uid[0] << 24) |
               ((uint32_t)uid[1] << 16) |
               ((uint32_t)uid[2] << 8) |
               ((uint32_t)uid[3]);
    
    mfrc522.PICC_HaltA();
    return true;
}