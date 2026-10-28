// rfid_reader.h
#pragma once
#include <MFRC522.h>
#include <SPI.h>
#include "config.h"

class RFIDReader {
private:
    MFRC522 mfrc522;
    uint32_t last_uid;
    
public:
    RFIDReader();
    void init();
    bool readUID(uint8_t* uid);
    uint32_t getLastUID() { return last_uid; }
    bool verifyTool(uint8_t cabinet_num);
};

extern RFIDReader rfid;