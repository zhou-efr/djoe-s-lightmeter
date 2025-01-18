#ifndef LIGHTMETER_LUX_H
#define LIGHTMETER_LUX_H

const double C = 320; // constant for lux to EV conversion - 320 for incident light, 250 for reflected light

double getEv(double lux);
double getEiso(double Ev, double iso);
double getT(double lux, double A, double iso);
double getA(double lux, double t, double iso);

#endif //LIGHTMETER_LUX_H
