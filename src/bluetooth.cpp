#include <Arduino.h>
#include "bluetooth.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer;
BLECharacteristic* pSelectedCharacteristic;
BLECharacteristic* pISOCharacteristic;
BLECharacteristic* pShutterCharacteristic;
BLECharacteristic* pApertureCharacteristic;

void initBluetooth(
    BLECharacteristicCallbacks* SelectCallbacks,
    BLECharacteristicCallbacks* ISOCallbacks,
    BLECharacteristicCallbacks* ShutterCallbacks,
    BLECharacteristicCallbacks* ApertureCallbacks
) {
    Serial.println("Initialisation du BT");
    BLEDevice::init(DEVICE_NAME);
    pServer = BLEDevice::createServer();

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

    Serial.println("Demarrage du service");
    pService->start();
    Serial.println("Advertising");
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("BT initialisation end");
}
