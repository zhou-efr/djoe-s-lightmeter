//
// Created by Killian Zhou on 14/04/2024.
//

#include "views.h"

void setup_fonts(){
    if (!LittleFS.begin()) {
        Serial.println("Flash FS initialisation failed!");
        while (1) yield(); // Stay here twiddling thumbs waiting
    }
    Serial.println("\nFlash FS available!");

    bool font_missing = false;
    if (!LittleFS.exists("/PoppinsRegular8.vlw")) font_missing = true;
    if (!LittleFS.exists("/PoppinsRegular10.vlw")) font_missing = true;

    if (font_missing)
    {
        Serial.println("\nFont missing in Flash FS, did you upload it?");
        while(1) yield();
    }
    else Serial.println("\nFonts found OK.");
}

void lightmeter_screen_title(TFT_eSPI* tft, String battery_level){    
    tft->fillScreen(TFT_LM_DARKGREY);
    tft->setTextColor(TFT_LM_OFF_WHITE, TFT_LM_DARKGREY);
    tft->loadFont(POPPINS_REGULAR_10, LittleFS);

    String title = "Djo's Light Meter - " + battery_level + "%";
    // int string_width = tft->textWidth(title) / 2;

    int x = 3, y = 3;
    tft->setCursor(x, y);
    tft->println(title);
}

void lightmeter_simple_indicator(
    TFT_eSPI* tft,
    bool is_active,
    bool is_selected,
    int x, int y
){
    int temp_x = x;
    int temp_y = y;
    int radius = 3;
    int border = 1;


    tft->fillSmoothCircle(
        temp_x+border+radius, 
        temp_y+border+radius, 
        radius+border, 
        is_selected ? TFT_LM_ORANGE : TFT_LM_OFF_WHITE, 
        TFT_LM_DARKGREY
    );

    tft->fillSmoothCircle(
        temp_x+border+radius, 
        temp_y+border+radius, 
        radius, 
        is_active ? TFT_LM_ORANGE : TFT_LM_DARKGREY, 
        is_selected ? TFT_LM_ORANGE : TFT_LM_OFF_WHITE
    );
}

void lightmeter_label(
    TFT_eSPI* tft,
    String label,
    bool is_active,
    bool is_selected,
    int x, int y
){
    tft->fillRect(x, y, 100, 30, TFT_LM_DARKGREY);
    lightmeter_simple_indicator(tft, is_active, is_selected, x, y);
    tft->loadFont(POPPINS_REGULAR_8, LittleFS);
    tft->setTextColor(TFT_LM_OFF_WHITE, TFT_LM_DARKGREY);
    tft->setCursor(x+12, y+1);
    tft->println(label);
}

void lightmeter_simple_slider(
    TFT_eSPI* tft,
    int percentage,
    bool is_selected,
    int x, int y,
    int w, int h
    )
{   
    int32_t border = 1;
    int32_t border_radius = 4;

    tft->drawSmoothRoundRect(
        x, y, 
        border_radius, border_radius, 
        w+2*border, h+2*border,
        is_selected ? TFT_LM_ORANGE : TFT_LM_OFF_WHITE,
        TFT_LM_DARKGREY
    );

    int32_t bar_thickness = h/4;
    int32_t spacing_x = bar_thickness;
    int32_t bar_width = w - 2*spacing_x;
    int32_t pos_x = x + border + spacing_x;
    int32_t pos_y = (y + (h + 2*border)/2) - bar_thickness/2;
    
    tft->fillSmoothRoundRect(
        pos_x, pos_y,
        bar_width, bar_thickness,
        border_radius, 
        TFT_LM_OFF_WHITE,
        TFT_LM_DARKGREY
    );

    int indicator_radius = 3;
    int indicator_x = x + (((bar_width-3*indicator_radius) * percentage) / 100) + 2*indicator_radius;
    int indicator_y = y + border + bar_thickness/2 + indicator_radius;

    // Test if the anti-aliasing doesn't add a border to the bar
    tft->fillSmoothCircle(
        indicator_x, 
        indicator_y, 
        indicator_radius, 
        TFT_LM_OFF_WHITE, 
        TFT_LM_DARKGREY
    );
}

void lightmeter_complex_slider(
    TFT_eSPI* tft,
    int percentage,
    bool is_selected,
    int x, int y,
    int w, int h
){
    int32_t border = 1;
    int32_t border_radius = 2;

    tft->drawSmoothRoundRect(
        x-border, y-border, 
        border_radius, border_radius, 
        w+2*border, h+2*border,
        is_selected ? TFT_LM_ORANGE : TFT_LM_OFF_WHITE,
        TFT_LM_DARKGREY
    );

    int32_t graduation_bar_thickness = w/12;
    int32_t large_graduation_width = w-6;
    int32_t small_graduation_width = w-12;
    int32_t spacing_y = graduation_bar_thickness;

    for(int i = 1; i < 10; i++){
        int32_t bar_width = (i == 9 || i == 1 || i == 5) ? large_graduation_width : small_graduation_width;
        
        int32_t pos_x = x + border + w/2 - bar_width/2;
        int32_t pos_y = y + (i * (h - 2*spacing_y - 2*border))/9;
        
        tft->fillSmoothRoundRect(
            pos_x, pos_y,
            bar_width, graduation_bar_thickness,
            border_radius, 
            TFT_LM_OFF_WHITE,
            TFT_LM_DARKGREY
        );
    }

    int32_t bar_thickness = w/12;
    spacing_y = bar_thickness;
    int32_t bar_height = h - 2*spacing_y;
    int32_t pos_x = x + w/2 + border - bar_thickness/2;
    int32_t pos_y = y + border + h/2 - bar_height/2;
    
    tft->fillSmoothRoundRect(
        pos_x, pos_y,
        bar_thickness, bar_height,
        border_radius, 
        TFT_LM_OFF_WHITE,
        TFT_LM_DARKGREY
    );

    int indicator_w = w-4;
    int indicator_h = 4;
    int indicator_border = 1;
    int indicator_radius = 4;

    int indicator_x = x + border + 1;
    int indicator_y = y + border + spacing_y + ((bar_height-indicator_h)*percentage)/100;
    
    tft->fillSmoothRoundRect(
        indicator_x, indicator_y,
        indicator_w+2*indicator_border, indicator_h+2*indicator_border,
        indicator_radius, 
        TFT_LM_OFF_WHITE,
        TFT_LM_DARKGREY
    );

    tft->fillSmoothRoundRect(
        indicator_x+indicator_border, indicator_y+indicator_border,
        indicator_w, indicator_h,
        indicator_radius, 
        TFT_LM_ORANGE,
        TFT_LM_OFF_WHITE
    );
}

void lightmeter_load_screen(
    TFT_eSPI* tft,
    TFT_eSprite* ISO_sprite,
    String battery_level,
    String shutter_speed, String aperture, String ISO,
    int shutter_percentage, int aperture_percentage, int ISO_percentage
){

  
  lightmeter_screen_title(tft, battery_level);

  lightmeter_label(tft, "Shutter - " + shutter_speed, false, true, 3, 23);

  lightmeter_simple_slider(tft, shutter_percentage, true, 3, 35, 100, 8);
  
  lightmeter_label(tft, "Aperture - " + aperture, true, false, 3, 53);

  lightmeter_simple_slider(tft, aperture_percentage, false, 3, 65, 100, 8);

  tft->setPivot(124, 80);

  int string_width = tft->textWidth("ISO - " + ISO);

  ISO_sprite->setColorDepth(16);
  ISO_sprite->createSprite(3+8+3+string_width+3, 10);
  ISO_sprite->setPivot(0, 10);
  ISO_sprite->fillSprite(TFT_LM_DARKGREY);
  lightmeter_label(ISO_sprite, "ISO - " + ISO, false, false, 5, 1);
  ISO_sprite->pushRotated(-90);

  lightmeter_complex_slider(tft, ISO_percentage, false, 131, 3, 24, 70);
}

void lightmeter_update_screen(
    TFT_eSPI* tft,
    TFT_eSprite* ISO_sprite,
    int changed_parameter,
    bool is_selected, int is_active, 
    String value, int percentage
){
    switch (changed_parameter)
    {
    case 0:
        lightmeter_label(tft, "Shutter - " + value, is_active, is_selected, 3, 23);
        lightmeter_simple_slider(tft, percentage, is_selected, 3, 35, 100, 8);
        break;
    case 1:
        lightmeter_label(tft, "Aperture - " + value, is_active, is_selected, 3, 53);
        lightmeter_simple_slider(tft, percentage, is_selected, 3, 65, 100, 8);
        break;
    case 2:
        ISO_sprite->fillSprite(TFT_LM_DARKGREY);
        lightmeter_label(ISO_sprite, "ISO - " + value, is_active, is_selected, 5, 1);
        ISO_sprite->pushRotated(-90);
        lightmeter_complex_slider(tft, percentage, is_selected, 131, 3, 24, 70);
        break;
    }
}   

void lightmeter_update_selection(
    TFT_eSPI* tft,
    TFT_eSprite* ISO_sprite,
    int selected, int active,
    int t_percentage,
    int a_percentage,
    int iso_percentage,
    String ISO
){
    Serial.println("Selected : " + String(selected) + " ; Active : " + String(active));
    lightmeter_simple_indicator(tft, active==0, selected==0, 3, 23);
    lightmeter_simple_slider(tft, t_percentage, selected==0, 3, 35, 100, 8);
    lightmeter_simple_indicator(tft, active==1, selected==1, 3, 53);
    lightmeter_simple_slider(tft, a_percentage, selected==1, 3, 65, 100, 8);
    ISO_sprite->fillSprite(TFT_LM_DARKGREY);
    lightmeter_label(ISO_sprite, "ISO - " + ISO, active==2, selected==2, 5, 1);
    ISO_sprite->pushRotated(-90);
    lightmeter_complex_slider(tft, iso_percentage, selected==2, 131, 3, 24, 70);
}