#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <BH1750.h>

#include <BLEUtils.h>

#include "views.h"
#include "lux.h"
#include <bluetooth.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite ISO_sprite = TFT_eSprite(&tft);

BH1750 lightMeter;

uint32_t lastUpdate = 0;

const int ISOs_len = 11;
double ISOs[ISOs_len] = {25, 50, 80, 100, 200, 400, 500, 800, 1600, 3200, 6400};

const int Ts_len = 17;
double Ts[Ts_len] = {0.0, 2.0, 1.0, 0.5, 0.25, 0.125, 0.067, 0.033, 0.0166666667, 0.008, 0.004, 0.002, 0.001, 0.0005, 0.00025, 0.000125, 0.0000625};
String Ts_str[Ts_len] = {"B", "2", "1", "1/2", "1/4", "1/8", "1/15", "1/30", "1/60", "1/125", "1/250", "1/500", "1/1000", "1/2000", "1/4000", "1/8000", "1/16000"};

const int As_len = 26;
double As[As_len] = {0.5, 1.0, 1.4, 1.8, 1.9, 2, 2.8, 3.5, 4, 4.5, 5.6, 6.2, 7.3, 8, 9.5, 10, 11, 12, 14, 16, 18, 20, 22, 25, 29, 32};
String As_str[As_len] = {"F0.5", "F1", "F1.4", "F1.8", "F1.9", "F2", "F2.8", "F3.5", "F4", "F4.5", "F5.6", "F6.3", "F7.1", "F8", "F9.5", "F10", "F11", "F12", "F14", "F16", "F18", "F20", "F22", "F25", "F29", "F32"};

enum {
    UNKNOWN,
    WAIT,
    UP,
    DOWN,
    ENTER,
    RIGHT,
    LEFT,
    HELP
} State_Type;

typedef struct {
    int ActState;
    int NextState;
    int static_parameter; // 0 - shutter, 1 - aperture, ~~2 - ISO~~
    int selected_parameter; // 0 - shutter, 1 - aperture, ~~2 - ISO~~
    int aperture_index;
    int shutter_index;
    int ISO_index;
    int old_aperture_index;
    int old_shutter_index;
    int old_ISO_index;
} State_Struct;

State_Struct State;
String Serialcurrentline = "";

double nearestNominal(double value, double nominalValues[], int size);
String nearestNominalString(double value, double nominalValues[], String nominalStrings[], int size);
int nearestNominalIndex(double value, double nominalValues[], int size);
void InitStateMachine();
// void ChangeState();
void HandleSerial();
void HandleStates();
void HandleCommands(String Cmd);

class SelectCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value == "aperture") {
            State.static_parameter = 1;
        }
        if (value == "shutter") {
            State.static_parameter = 0;
        }
    }
};

class ISOCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        State.ISO_index = nearestNominalIndex(std::stod(value), ISOs, ISOs_len);
    }
};

class ShutterCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        State.shutter_index = nearestNominalIndex(std::stod(value), Ts, Ts_len);
    }
};

class ApertureCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        State.aperture_index = nearestNominalIndex(std::stod(value), As, As_len);
    }
};

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println();
  Serial.print(__FILE__);
  Serial.println();

  InitStateMachine();

  tft.init();
  setup_fonts();
  tft.setRotation(3);

  lightmeter_load_screen(
    &tft, 
    &ISO_sprite,
    "100",
    Ts_str[State.shutter_index],
    As_str[State.aperture_index],
    String(ISOs[State.ISO_index]),
    int((State.shutter_index*100)/Ts_len),
    int((State.aperture_index*100)/As_len),
    int((State.ISO_index*100)/ISOs_len)
    );
  
  Wire.begin(LM_SDA, LM_SCL);
  lightMeter.begin();

  SelectCallback* SelectCallbacks = new SelectCallback();
  ISOCallback* ISOCallbacks = new ISOCallback();
  ShutterCallback* ShutterCallbacks = new ShutterCallback();
  ApertureCallback* ApertureCallbacks = new ApertureCallback();

  initBluetooth(
      SelectCallbacks,
      ISOCallbacks,
      ShutterCallbacks,
      ApertureCallbacks
  );
}

void loop(){
    HandleSerial();

    uint16_t interval = 1000;
    if (millis() - lastUpdate >= interval)
    {
    uint16_t lux = lightMeter.readLightLevel();
    // Serial.print("Light: ");
    // Serial.print(lux);
    // Serial.println(" lx");


    if (State.static_parameter == 0){
        double shutter = getT(lux, As[State.aperture_index], ISOs[State.ISO_index]);
        // Serial.println("Aperture (static): " + String(As[State.aperture_index],4) + 
        // " ; ISO (static): " + String(ISOs[State.ISO_index],4) + 
        // " ; Shutter: " + nearestNominalString(shutter, Ts, Ts_str, Ts_len));

        State.old_shutter_index = State.shutter_index;
        State.shutter_index = nearestNominalIndex(shutter, Ts, Ts_len);

        if (State.old_shutter_index != State.shutter_index){
            lightmeter_update_screen(
                &tft, 
                &ISO_sprite, 
                0, 
                (State.selected_parameter == 0),
                true,
                Ts_str[State.shutter_index],
                int((State.shutter_index*100)/Ts_len)
            );
        }
    }

    if (State.static_parameter == 1){
        double aperture = getA(lux, Ts[State.shutter_index], ISOs[State.ISO_index]);
        // Serial.println("Aperture: " + nearestNominalString(aperture, As, As_str, As_len) + 
        // " ; ISO (static): " + String(ISOs[State.ISO_index],4) + 
        // " ; Shutter (static): " + String(Ts[State.shutter_index],4));
        
        State.old_aperture_index = State.aperture_index;
        State.aperture_index = nearestNominalIndex(aperture, As, As_len);

        if (State.old_aperture_index != State.aperture_index){
            lightmeter_update_screen(
                &tft, 
                &ISO_sprite, 
                1, 
                (State.selected_parameter == 1),
                true,
                As_str[State.aperture_index],
                int((State.aperture_index*100)/As_len)
            );
        }
    }


    lastUpdate = millis();
    // Serial.println("-----------");
    }

    HandleStates();
}

void InitStateMachine() {
    State.ActState = WAIT;
    State.NextState = UNKNOWN;
    State.static_parameter = 1;
    State.selected_parameter = 0;
    State.aperture_index = 18;
    State.shutter_index = 9;
    State.ISO_index = 4;
}

void HandleSerial() {
    while (Serial.available()) {
        char c = Serial.read();
        bool CR = (c == char(10));
        if (c >= ' ') {
            Serialcurrentline += c;
        };
        if (CR)      {
            HandleCommands(Serialcurrentline);
            Serialcurrentline = "";
        };
    }
}

// void ChangeState() {
//     if (!State.NextState == UNKNOWN) {
//         State.ActState = State.NextState;
//         State.NextState = UNKNOWN;
//     } else State.ActState = WAIT;
// }

void HandleStates() {
    String changed = "";
    int changed_percentage = 0;
    int previous_static_parameter = -1;
    switch (State.ActState) {
        case HELP:
            Serial.println("Help test");
            State.ActState = WAIT;
            break;
        case DOWN:
            State.selected_parameter = (State.selected_parameter == 0) ? 2 : State.selected_parameter-1;
            lightmeter_update_selection(
                &tft,
                &ISO_sprite,
                State.selected_parameter,
                State.static_parameter,
                int((State.shutter_index*100)/Ts_len),
                int((State.aperture_index*100)/As_len),
                int((State.ISO_index*100)/ISOs_len),
                String(ISOs[State.ISO_index])
            );
            State.ActState = WAIT;
            break;
        case UP:
            State.selected_parameter = (State.selected_parameter == 2) ? 0 : State.selected_parameter+1;
            lightmeter_update_selection(
                &tft,
                &ISO_sprite,
                State.selected_parameter,
                State.static_parameter,
                int((State.shutter_index*100)/Ts_len),
                int((State.aperture_index*100)/As_len),
                int((State.ISO_index*100)/ISOs_len),
                String(ISOs[State.ISO_index])
            );
            State.ActState = WAIT;
            break;
        case ENTER:
            previous_static_parameter = State.static_parameter;
            State.static_parameter = State.static_parameter ? 0 : 1;
            lightmeter_update_selection(
                &tft,
                &ISO_sprite,
                State.selected_parameter,
                State.static_parameter,
                int((State.shutter_index*100)/Ts_len),
                int((State.aperture_index*100)/As_len),
                int((State.ISO_index*100)/ISOs_len),
                String(ISOs[State.ISO_index])
            );
            lightmeter_update_screen(
                &tft, 
                &ISO_sprite, 
                previous_static_parameter, 
                false,
                State.selected_parameter == previous_static_parameter,
                previous_static_parameter ? As_str[State.aperture_index] : Ts_str[State.shutter_index],
                previous_static_parameter ? int((State.aperture_index*100)/As_len) : int((State.shutter_index*100)/Ts_len)
            );
            State.ActState = WAIT;
            break;
        case LEFT:
            if (State.selected_parameter == 0) {
                State.shutter_index = (State.shutter_index == 0) ? 0 : State.shutter_index-1;
                changed = Ts_str[State.shutter_index];
                changed_percentage = int((State.shutter_index*100)/Ts_len);
            }
            if (State.selected_parameter == 1) {
                State.aperture_index = (State.aperture_index == 0) ? 0 : State.aperture_index-1;
                changed = As_str[State.aperture_index];
                changed_percentage = int((State.aperture_index*100)/As_len);

            }
            if (State.selected_parameter == 2) {
                State.ISO_index = (State.ISO_index == 0) ? 0 : State.ISO_index-1;
                changed = String(ISOs[State.ISO_index]);
                changed_percentage = int((State.ISO_index*100)/ISOs_len);
            }
            lightmeter_update_screen(
                &tft, 
                &ISO_sprite, 
                State.selected_parameter, 
                true,
                State.static_parameter == State.selected_parameter,
                changed,
                changed_percentage
            );
            State.ActState = WAIT;
            break;
        case RIGHT:
            if (State.selected_parameter == 0) {
                State.shutter_index = (State.shutter_index == Ts_len-1) ? Ts_len-1 : State.shutter_index+1;
                changed = Ts_str[State.shutter_index];
                changed_percentage = int((State.shutter_index*100)/Ts_len);
            }
            if (State.selected_parameter == 1) {
                State.aperture_index = (State.aperture_index == As_len-1) ? As_len-1 : State.aperture_index+1;
                changed = As_str[State.aperture_index];
                changed_percentage = int((State.aperture_index*100)/As_len);

            }
            if (State.selected_parameter == 2) {
                State.ISO_index = (State.ISO_index == ISOs_len-1) ? ISOs_len-1 : State.ISO_index+1;
                changed = String(ISOs[State.ISO_index]);
                changed_percentage = int((State.ISO_index*100)/ISOs_len);
            }
            lightmeter_update_screen(
                &tft, 
                &ISO_sprite, 
                State.selected_parameter, 
                true,
                State.static_parameter == State.selected_parameter,
                changed,
                changed_percentage
            );
            State.ActState = WAIT;
            break;
        default :      
            State.ActState = WAIT;
            break;
    }
}

void HandleCommands(String Cmd) {
    bool unknown = true;
    if (Cmd.startsWith(F("Help"))) {
        State.ActState = HELP;
        unknown = false;
    };
    if (Cmd.startsWith(F("Up"))) {
        State.ActState = UP;
        unknown = false;
    };
    if (Cmd.startsWith(F("Down"))) {
        State.ActState = DOWN;
        unknown = false;
    };
    if (Cmd.startsWith(F("Enter"))) {
        State.ActState = ENTER;
        unknown = false;
    };
    if (Cmd.startsWith(F("Right"))) {
        State.ActState = RIGHT;
        unknown = false;
    };
    if (Cmd.startsWith(F("Left"))) {
        State.ActState = LEFT;
        unknown = false;
    };
    if (unknown) {
        Serial.print (Cmd);
        Serial.println(F(" is unknown! For Help look at the source code <3"));
    };
}

double nearestNominal(double value, double nominalValues[], int size){
    if (value < nominalValues[0]){
        return nominalValues[0];
    }
    for (int i = 0; i < size-1; i++){
        if (nominalValues[i] <= value && value < nominalValues[i+1]) {
            return nominalValues[i];
        }
    }
    return nominalValues[size-1];
}

String nearestNominalString(double value, double nominalValues[], String nominalStrings[], int size){
    if (value < nominalValues[0]){
        return nominalStrings[0];
    }
    for (int i = 0; i < size-1; i++){
        if (nominalValues[i] <= value && value < nominalValues[i+1]) {
            return nominalStrings[i];
        }
    }
    return nominalStrings[size-1];
}

int nearestNominalIndex(double value, double nominalValues[], int size){
    if (value < nominalValues[0]){
        return 0;
    }
    for (int i = 0; i < size-1; i++){
        if (nominalValues[i] <= value && value < nominalValues[i+1]) {
            return i;
        }
    }
    return size-1;
}