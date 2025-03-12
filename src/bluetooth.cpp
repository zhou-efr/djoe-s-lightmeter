#include <Arduino.h>
#include "bluetooth.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class DjoesServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer);
    void onDisconnect(BLEServer *pServer);
};

void initBluetooth(
    BLECharacteristicCallbacks* SelectCallbacks,
    BLECharacteristicCallbacks* ISOCallbacks,
    BLECharacteristicCallbacks* ShutterCallbacks,
    BLECharacteristicCallbacks* ApertureCallbacks
) {
    BLEDevice::init(DEVICE_NAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new DjoesServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pSelectedCharacteristic = pService->createCharacteristic(
        SELECTED_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pSelectedCharacteristic->setCallbacks(SelectCallbacks);

    pISOCharacteristic = pService->createCharacteristic(
        ISO_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pISOCharacteristic->setCallbacks(ISOCallbacks);

    pShutterCharacteristic = pService->createCharacteristic(
        SHUTTER_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pShutterCharacteristic->setCallbacks(ShutterCallbacks);

    pApertureCharacteristic = pService->createCharacteristic(
        APERTURE_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pApertureCharacteristic->setCallbacks(ApertureCallbacks);

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}
