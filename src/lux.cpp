#include <Arduino.h>
#include "lux.h"

// pow(16,2)/pow(2, log2(30/C) + log2(200/100))

double getEv(double lux){
  return 3.3*log10(lux)-1;
}

double getEiso(double Ev, double iso){
  return Ev + log2(iso/100);
}

double getT(double lux, double A, double iso){
  double Ev = getEv(lux);
  double Eiso = getEiso(Ev, iso);
  // Serial.println("Compute aperture : ");
  // Serial.println("Ev : " + String(Ev,4));
  // Serial.println("Eiso : " + String(Eiso,4));
  // Serial.println("A : " + String(A,4));
  // Serial.println("ISO : " + String(iso,4));
  return double(pow(A,2)/pow(2, Eiso));
}

double getA(double lux, double t, double iso){
  double Ev = getEv(lux);
  double Eiso = getEiso(Ev, iso);
  // Serial.println("Compute aperture : ");
  // Serial.println("Ev : " + String(Ev,4));
  // Serial.println("Eiso : " + String(Eiso,4));
  // Serial.println("t : " + String(t,4));
  // Serial.println("ISO : " + String(iso,4));
  return double(sqrt(pow(2, Eiso) * t));
}