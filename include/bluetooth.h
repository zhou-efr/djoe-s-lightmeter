#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

void initBluetooth(
    BLECharacteristicCallbacks* SelectCallbacks,
    BLECharacteristicCallbacks* ISOCallbacks,
    BLECharacteristicCallbacks* ShutterCallbacks,
    BLECharacteristicCallbacks* ApertureCallbacks
);

#endif //BLUETOOTH_H