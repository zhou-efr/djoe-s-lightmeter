#ifndef LIGHTMETER_VIEWS_H
#define LIGHTMETER_VIEWS_H

#include <TFT_eSPI.h>
#include <Arduino.h>

#include <FS.h>
#include <LittleFS.h>
#define FlashFS LittleFS

void setup_fonts();
void lightmeter_screen_title(TFT_eSPI* tft, String battery_level);
void lightmeter_label(
    TFT_eSPI* tft,
    String label,
    bool is_active,
    bool is_selected,
    int x, int y
);
void lightmeter_simple_slider(
    TFT_eSPI* tft,
    int percentage,
    bool is_selected,
    int x, int y,
    int w, int h
);
void lightmeter_complex_slider(
    TFT_eSPI* tft,
    int percentage,
    bool is_selected,
    int x, int y,
    int w, int h
);
void lightmeter_load_screen(
    TFT_eSPI* tft,
    TFT_eSprite* ISO_sprite,
    String battery_level,
    int selected, int staticparam,
    String shutter_speed, String aperture, String ISO,
    int shutter_percentage, int aperture_percentage, int ISO_percentage
);
void lightmeter_update_screen(
    TFT_eSPI* tft,
    TFT_eSprite* ISO_sprite,
    int changed_parameter,
    bool is_selected, int is_active, 
    String value, int percentage
);
void lightmeter_update_selection(
    TFT_eSPI* tft,
    TFT_eSprite* ISO_sprite,
    int selected, int active,
    int t_percentage,
    int a_percentage,
    int iso_percentage,
    String ISO
);

#endif //LIGHTMETER_VIEWS_H
