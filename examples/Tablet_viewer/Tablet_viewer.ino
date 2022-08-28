//=============================================================================
// Simple test viewer app for the WacomController object
// This sketch as well as all others in this library will only work with the
// Teensy 3.6 and the T4.x boards.
//
// The default display is the ILI9341, which uses the ILI9341_t3n library,
// which is located at:
//    ili9341_t3n that can be located: https://github.com/KurtE/ILI9341_t3n
//
// Alternate display ST7735 or ST7789 using the ST7735_t3 library which comes
// with Teensyduino.
//
// Default pins
//   8 = RST
//   9 = D/C
//  10 = CS
//
// This example is in the public domain
//=============================================================================

//#define USE_ST77XX // define this if you wish to use one of these displays.
#define USE_KURTS_MICROMOD_BOARD

#include "USBHost_t36.h"
#include "WacomController.h"

#ifdef USE_ST77XX
#include <ST7735_t3.h>
#include <st7735_t3_font_Arial.h>
#include <ST7789_t3.h>
#define BLACK ST77XX_BLACK
#define WHITE ST77XX_WHITE
#define YELLOW ST77XX_YELLOW
#define GREEN ST77XX_GREEN
#define RED ST77XX_RED
#define CENTER ST7789_t3::CENTER
#else
#include <ILI9341_t3n.h>
#include <ili9341_t3n_font_Arial.h>
#define BLACK ILI9341_BLACK
#define WHITE ILI9341_WHITE
#define YELLOW ILI9341_YELLOW
#define GREEN ILI9341_GREEN
#define RED ILI9341_RED
#define CENTER ILI9341_t3n::CENTER

#endif
#define LIGHTGREY 0xC618   /* 192, 192, 192 */
#define DARKGREY 0x7BEF    /* 128, 128, 128 */
#define BLUE 0x001F        /*   0,   0, 255 */

//=============================================================================
// Connection configuration of ILI9341 LCD TFT
//=============================================================================
#if defined(ARDUINO_TEENSY_MICROMOD) && defined(USE_KURTS_MICROMOD_BOARD)
#define TFT_RST 31  // 8
#define TFT_DC 9
#define TFT_CS 32  //10
#else
#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10
#endif


#ifdef USE_ST77XX
// define which one you are using
//DMAMEM uint16_t frame_buffer[ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT];
// Note there are other options, like defining MOSI, SCK...
//ST7735_t3 tft = ST7735_t3(cs, dc, rst);
// For 1.54" TFT with ST7789
ST7789_t3 tft = ST7789_t3(TFT_CS, TFT_DC, TFT_RST);
#else
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);
#endif
//=============================================================================
// USB Host Ojbects
//=============================================================================
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBHIDParser hid3(myusb);
USBHIDParser hid4(myusb);
USBHIDParser hid5(myusb);
WacomController digi1(myusb);


// Lets only include in the lists The most top level type devices we wish to show information for.
//USBDriver *drivers[] = {&keyboard1, &keyboard2, &joystick};
USBDriver *drivers[] = { &hub1, &hub2, &hid1, &hid2, &hid3, &hid4, &hid5 };

#define CNT_DEVICES (sizeof(drivers) / sizeof(drivers[0]))
//const char * driver_names[CNT_DEVICES] = {"KB1", "KB2", "Joystick(device)"};
const char *driver_names[CNT_DEVICES] = { "Hub1", "Hub2", "HID1", "HID2", "HID3", "HID4", "HID5" };
//bool driver_active[CNT_DEVICES] = {false, false, false};
bool driver_active[CNT_DEVICES] = { false, false, false, false, false };

// Lets also look at HID Input devices
USBHIDInput *hiddrivers[] = { &digi1 };
#define CNT_HIDDEVICES (sizeof(hiddrivers) / sizeof(hiddrivers[0]))
const char *hid_driver_names[CNT_HIDDEVICES] = { "tablet" };

bool hid_driver_active[CNT_HIDDEVICES] = { false };

//=============================================================================
// Other state variables.
//=============================================================================

// Save away values for buttons, x, y, wheel, wheelh
#define MAX_TOUCH 4
uint32_t buttons_cur = 0;
int x_cur = 0,
    x_cur_prev = 0,
    y_cur = 0,
    y_cur_prev = 0,
    z_cur = 0;

int wheel_cur = 0;
int wheelH_cur = 0;
int axis_cur[10];

bool BT = 0;

bool show_alternate_view = false;

int user_axis[64];
uint32_t buttons_prev = 0;
uint32_t buttons;

bool g_redraw_all = false;
int16_t y_position_after_device_info = 0;

//=============================================================================
// Setup
//=============================================================================
void setup() {
  Serial1.begin(2000000);
  while (!Serial && millis() < 3000)
    ;  // wait for Arduino Serial Monitor
  Serial.println("\nTablet Viewer");
  if (CrashReport) Serial.print(CrashReport);
  myusb.begin();

#ifdef USE_ST77XX
  // Only uncomment one of these init options.
  // ST7735 - More options mentioned in examples for st7735_t3 library
  //tft.initR(INITR_BLACKTAB); // if you're using a 1.8" TFT 128x160 displays
  //tft.initR(INITR_144GREENTAB); // if you're using a 1.44" TFT (128x128)
  //tft.initR(INITR_MINI160x80);  //if you're using a .96" TFT(160x80)

  // ST7789
  tft.init(240, 240);  // initialize a ST7789 chip, 240x240 pixels
  //tft.init(240, 320);           // Init ST7789 2.0" 320x240
  //tft.init(135, 240);             // Init ST7789 1.4" 135x240
  //tft.init(240, 240, SPI_MODE2);    // clones Init ST7789 240x240 no CS
#else
  tft.begin();
#endif
  // explicitly set the frame buffer
  //  tft.setFrameBuffer(frame_buffer);
  delay(100);
  tft.setRotation(3);  // 180
  delay(100);
  // make sure display is working
  tft.fillScreen(GREEN);
  delay(500);
  tft.fillScreen(YELLOW);
  delay(500);
  tft.fillScreen(RED);
  delay(500);

  tft.fillScreen(BLACK);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.setCursor(CENTER, CENTER);
  tft.println("Waiting for Device...");
  tft.useFrameBuffer(true);
  tft.updateChangedAreasOnly(true);
}


//=============================================================================
// Loop
//=============================================================================
void loop() {
  myusb.Task();

  // Update the display with
  UpdateActiveDeviceInfo();

  // Now lets try displaying Tablet data
  ProcessTabletData();

  if (Serial.available()) {
    while (Serial.read() != -1)
      ;
    switchView();
  }
}


void switchView() {
  show_alternate_view = !show_alternate_view;
  if (show_alternate_view) Serial.println("Switched to simple graphic mode");
  else {
    Serial.println("switched back to Text mode");
  }
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(YELLOW);
  tft.setFont(Arial_12);
  tft.printf("(%x:%x): ", digi1.idVendor(), digi1.idProduct());
  const uint8_t *psz;
  psz = digi1.product();
  if (psz && *psz) tft.print((const char *)psz);
  tft.println();
  tft.updateScreen();
  g_redraw_all = true;
}

//=============================================================================
// ProcessTabletData
//=============================================================================
void ProcessTabletData() {
  digi1.debugPrint(false);
  bool update_screen;
  if (digi1.available()) {
    elapsedMicros em;
    if (show_alternate_view) update_screen = ShowSimpleGraphicScreen();
    else update_screen = showDataScreen();
    if (update_screen) {
      tft.updateScreen();  // update the screen now
      Serial.println(em);
    }
    digi1.digitizerDataClear();
  }
}

//=============================================================================
// Show using raw data output.
//=============================================================================
bool showDataScreen() {
  int16_t tilt_x_cur = 0,
          tilt_y_cur = 0;
  uint16_t pen_press_cur = 0,
           pen_dist_cur = 0;
  int touch_x_cur[4] = { 0, 0, 0, 0 },
      touch_y_cur[4] = { 0, 0, 0, 0 };
  bool touch = false;

  if (g_redraw_all) {
    // Lets display the titles.
    int16_t x;
    tft.getCursor(&x, &y_position_after_device_info);
    tft.setTextColor(YELLOW);
    tft.printf("Buttons:\nPen (X, Y):\nTouch (X, Y):\n\nPress/Dist:\nTiltXY:\nFPress:\nFTouch:\nAxis:");
    g_redraw_all = false;
  }

  int touch_count = digi1.getTouchCount();
  int touch_index;
  uint16_t buttons_bin = digi1.getPenButtons();
  Serial.println(buttons_bin);
  bool pen_button[4];
  bool button_touched[8];
  bool button_pressed[8];
  Serial.printf("Digitizer: ");
  for (int i = 0; i < 4; i++) {
    pen_button[i] = (buttons_bin >> i) & 0x1;
    Serial.printf("Pen_Btn%d:%d ", i, pen_button[i]);
  }

  bool something_changed = false;

  //buttons are saved within one byte for all the 8 buttons, here is a decoding example
  uint16_t button_touch_bin = digi1.getFrameTouchButtons();
  uint16_t button_press_bin = digi1.getFrameButtons();
  for (int i = 0; i < 8; i++) {
    button_touched[i] = (button_touch_bin >> i) & 0x1;
    button_pressed[i] = (button_press_bin >> i) & 0x1;
    Serial.printf("Btn%d: T:%d P:%d ", i, button_touched[i], button_pressed[i]);
  }

  WacomController::event_type_t evt = digi1.eventType();
  switch (evt) {
    case WacomController::TOUCH:
      Serial.print(" Touch:");
      for (touch_index = 0; touch_index < touch_count; touch_index++) {
        Serial.printf(" (%d, %d)", digi1.getX(touch_index), digi1.getY(touch_index));
        touch_x_cur[touch_index] = digi1.getX(touch_index);
        touch_y_cur[touch_index] = digi1.getY(touch_index);
      }
      touch = true;
      break;
    case WacomController::PEN:
      Serial.printf(" Pen: (%d, %d) Pressure: %u Distance: %u", digi1.getX(), digi1.getY(),
                    digi1.getPenPressure(), digi1.getPenDistance());
      Serial.printf(" TiltX: %d TiltY: %d", digi1.getPenTiltX(), digi1.getPenTiltY());
      x_cur = digi1.getX();
      y_cur = digi1.getY();
      pen_press_cur = digi1.getPenPressure();
      pen_dist_cur = digi1.getPenDistance();
      touch = false;
      break;
    case WacomController::FRAME:
      {
        //wheel data 0-71
        Serial.printf(" Whl: %d ", digi1.getFrameWheel());
        //wheel button binary, no touch functionality
        Serial.printf(" WhlBtn: %d ", digi1.getFrameWheelButton());
        something_changed = true;
      }
      break;
    default:
      Serial.printf(",  X = %u, Y = %u", digi1.getX(), digi1.getY());
      Serial.print(",  Pressure: = ");
      Serial.print(",  wheel = ");
      Serial.print(digi1.getWheel());
      Serial.print(",  wheelH = ");
      Serial.print(digi1.getWheelH());
      break;
  }

  Serial.println();
  
  if (buttons_bin != buttons_cur) {
    buttons_cur = buttons_bin;
    something_changed = true;
  }
  if (x_cur != x_cur_prev) {
    x_cur_prev = x_cur;
    something_changed = true;
  }
  if (y_cur != y_cur_prev) {
    y_cur_prev = y_cur;
    something_changed = true;
  }
  if (digi1.getPenPressure() != pen_press_cur) {
    pen_press_cur = digi1.getPenPressure();
    something_changed = true;
  }
  if (digi1.getPenDistance() != pen_dist_cur) {
    pen_dist_cur = digi1.getPenDistance();
    something_changed = true;
  }
  if (digi1.getPenTiltX() != tilt_x_cur) {
    tilt_x_cur = digi1.getPenTiltX();
    something_changed = true;
  }
  if (digi1.getPenTiltY() != tilt_y_cur) {
    tilt_y_cur = digi1.getPenTiltY();
    something_changed = true;
  }
  if (touch) {
    for (touch_index = 0; touch_index < touch_count; touch_index++) {
      touch_x_cur[touch_index] = digi1.getX(touch_index);
      touch_y_cur[touch_index] = digi1.getY(touch_index);
    }
    something_changed = true;
  }
  // BUGBUG:: play with some Axis...
  for (uint8_t i = 0; i < 10; i++) {
    int axis = digi1.getAxis(i);
    if (axis != axis_cur[i]) {
      axis_cur[i] = axis;
      something_changed = true;
    }
  }

  if (something_changed) {
#define TABLET_DATA_X 100
    int16_t x, y2;
    unsigned char line_space = Arial_12.line_space;
    tft.setTextColor(WHITE, BLACK);
    //tft.setTextDatum(BR_DATUM);
    int16_t y = y_position_after_device_info;
    tft.setCursor(TABLET_DATA_X, y);
    for (int i = 0; i < 4; i++) {
      tft.printf("(%d) ", pen_button[i]);
    }
    tft.getCursor(&x, &y2);
    tft.fillRect(x, y2, 320, line_space, BLACK);

    y += line_space;
    if (touch == false) {
      tft.setCursor(TABLET_DATA_X, y);
      tft.fillRect(x, y2, 320, line_space, BLACK);
      tft.printf("(%d, %d)", x_cur, y_cur);
    }
    y += line_space;
    if (touch == true) {
      tft.setCursor(TABLET_DATA_X, y);
      tft.fillRect(TABLET_DATA_X, y, tft.width(), line_space, BLACK);
      for (int i = 0; i < touch_count; i++) {
        if (i == 2) {
          y += line_space;
          tft.setCursor(TABLET_DATA_X, y);
          tft.fillRect(x, y2, 320, line_space, BLACK);
        }
        tft.printf("(%d, %d) ", touch_x_cur[i], touch_y_cur[i]);
      }
    }
    if (touch_count < 2) {
      y += line_space;
      tft.setCursor(TABLET_DATA_X, y);
    }
    y += line_space;
    tft.setCursor(TABLET_DATA_X, y);
    tft.fillRect(x, y2, 320, line_space, BLACK);
    tft.printf("%d, %d", pen_press_cur, pen_dist_cur);
    y += line_space;
    tft.fillRect(x, y2, 320, line_space, BLACK);
    tft.setCursor(TABLET_DATA_X, y);
    tft.printf("(%d, %d)", tilt_x_cur, tilt_y_cur);

    y += line_space;
    tft.fillRect(TABLET_DATA_X, y, tft.width(), line_space, BLACK);
    if (evt == WacomController::FRAME) {
      tft.setCursor(TABLET_DATA_X, y);
      for (int i = 0; i < 4; i++) {
        tft.printf("(%d) ", button_pressed[i]);
      }
    }
    y += line_space;
    tft.fillRect(TABLET_DATA_X, y, tft.width(), line_space, BLACK);
    if (evt == WacomController::FRAME) {
      tft.setCursor(TABLET_DATA_X, y);
      for (int i = 0; i < 4; i++) {
        tft.printf("(%d) ", button_touched[i]);
      }
    }
    //y += line_space; OutputNumberField(TABLET_DATA_X, y, wheel_cur, 320);
    //y += line_space; OutputNumberField(TABLET_DATA_X, y, wheelH_cur, 320);
    /*
    // Output other Axis data
    for (uint8_t i = 0; i < 9; i += 3) {
      y += line_space;
      OutputNumberField(TABLET_DATA_X, y, axis_cur[i], 75);
      OutputNumberField(TABLET_DATA_X + 75, y, axis_cur[i + 1], 75);
      OutputNumberField(TABLET_DATA_X + 150, y, axis_cur[i + 2], 75);
    }
*/
  }
  return something_changed;
}

//=============================================================================
// Show using psuedo graphic screen
//=============================================================================
#define BUTTON_WIDTH 15
#define BUTTON_HEIGHT 30

int g_cnt_pen_buttons; 
int g_cnt_frame_buttons;
int g_button_height;
uint32_t g_pen_buttons_prev;
uint32_t g_frame_buttons_prev;

int tablet_changed_area_x_min = 0;
int tablet_changed_area_x_max = 0;
int tablet_changed_area_y_min = 0;
int tablet_changed_area_y_max = 0;

bool ShowSimpleGraphicScreen() {
  if (g_redraw_all) {
    // Lets display the titles.
    y_position_after_device_info = tft.getCursorY();
    tft.setTextColor(YELLOW);
    g_cnt_pen_buttons = digi1.getCntPenButtons();
    g_cnt_frame_buttons = digi1.getCntFrameButtons();

    if (g_cnt_frame_buttons >= g_cnt_pen_buttons) g_button_height = (tft.height() - y_position_after_device_info) / g_cnt_frame_buttons;
    else g_button_height = (tft.height() - y_position_after_device_info) / g_cnt_pen_buttons;
    if (g_button_height > BUTTON_HEIGHT) g_button_height = BUTTON_HEIGHT;
  }

  int y_start_graphics = y_position_after_device_info;


  //Serial.printf("P:%d F:%d H:%d\n", g_cnt_pen_buttons, g_cnt_frame_buttons, g_button_height);
  // Lets output pen buttons first
  uint32_t buttons = digi1.getPenButtons();
  bool pen_touching = buttons & 1;
  uint16_t x = 5;
  uint16_t y = tft.height() - g_button_height - 2;
  uint8_t index = 0;
  bool buttons_changed = false;

  if (g_redraw_all) { 
    g_pen_buttons_prev = buttons;
    for (index = 0; index < g_cnt_pen_buttons; index++) {
      tft.drawRect(x, y, BUTTON_WIDTH, g_button_height, GREEN);
      if (buttons & 1) tft.fillRect(x + 1, y + 1, BUTTON_WIDTH - 2, g_button_height - 2, BLUE);
      else tft.fillRect(x + 1, y + 1, BUTTON_WIDTH - 2, g_button_height - 2, BLACK);
      buttons >>= 1;
      y -= g_button_height;     
    }
  } else if (buttons != g_pen_buttons_prev) { 
    buttons_changed = true;
    uint32_t buttons_prev = g_pen_buttons_prev;
    g_pen_buttons_prev = buttons;
    for (index = 0; index < g_cnt_pen_buttons; index++) {
      if ((buttons & 1) != (buttons_prev & 1)) {
        if (buttons & 1) tft.fillRect(x + 1, y + 1, BUTTON_WIDTH - 2, g_button_height - 2, BLUE);
        else tft.fillRect(x + 1, y + 1, BUTTON_WIDTH - 2, g_button_height - 2, BLACK);
      }
      buttons >>= 1;
      buttons_prev  >>= 1;
      y -= g_button_height;     
    }
  }

  buttons = digi1.getFrameButtons();
  x += BUTTON_WIDTH + 5;
  y = tft.height() - g_button_height - 2;
  if (g_redraw_all) { 
    //uint32_t butprev = g_frame_buttons_prev
    g_frame_buttons_prev = buttons;
    for (index = 0; index < g_cnt_frame_buttons; index++) {
      tft.drawRect(x, y, BUTTON_WIDTH, g_button_height, GREEN);
      if (buttons & 1) tft.fillRect(x + 1, y + 1, BUTTON_WIDTH - 2, g_button_height - 2, RED);
      else tft.fillRect(x + 1, y + 1, BUTTON_WIDTH - 2, g_button_height - 2, BLACK);
      buttons >>= 1;
      y -= g_button_height;     
    }
  } else if (buttons != g_frame_buttons_prev) { 
    buttons_changed = true;
    uint32_t buttons_prev = g_frame_buttons_prev;
    g_frame_buttons_prev = buttons;
    for (index = 0; index < g_cnt_frame_buttons; index++) {
      if ((buttons & 1) != (buttons_prev & 1)) {
        if (buttons & 1) tft.fillRect(x + 1, y + 1, BUTTON_WIDTH - 2, g_button_height - 2, RED);
        else tft.fillRect(x + 1, y + 1, BUTTON_WIDTH - 2, g_button_height - 2, BLACK);
      }
      buttons >>= 1;
      buttons_prev  >>= 1;
      y -= g_button_height;     
    }
  }    

  // now lets output a rough tablet image:
  x += BUTTON_WIDTH + 5;
  int tab_draw_width = tft.width() - x - 5;
  int tab_draw_height = tft.height() - y_start_graphics - 5;
      
  if (g_redraw_all) tft.fillRect(x, y_start_graphics, tab_draw_width, tab_draw_height, DARKGREY);
  else if (buttons_changed)tft.updateScreen();

  x += 3;
  y_start_graphics += 3;
  tab_draw_width -= 6;
  tab_draw_height -= 6;
  if (g_redraw_all) tft.fillRect(x, y_start_graphics, tab_draw_width, tab_draw_height, LIGHTGREY);
  tft.setClipRect(x, y_start_graphics, tab_draw_width, tab_draw_height);

  // if we changed something in previous run, than fill it back   
  if ((tablet_changed_area_x_min < tft.width()) && !g_redraw_all) 
    tft.fillRect(tablet_changed_area_x_min, tablet_changed_area_y_min, tablet_changed_area_x_max - tablet_changed_area_x_min + 1,
          tablet_changed_area_y_max - tablet_changed_area_y_min + 1, LIGHTGREY);
  
  tablet_changed_area_x_min = tft.width();
  tablet_changed_area_x_max = 0;
  tablet_changed_area_y_min = tft.height();
  tablet_changed_area_y_max = 0;


  WacomController::event_type_t evt = digi1.eventType();
  if (evt == WacomController::PEN) {
    if (pen_touching) {
      int pen_x = digi1.getX();
      int pen_y = digi1.getY();
      int x_in_tablet = map(pen_x, 0, digi1.width(), 0, tab_draw_width) + x;
      int y_in_tablet = map(pen_y, 0, digi1.height(), 0, tab_draw_height) + y_start_graphics;
      //Serial.printf("Pen: (%d, %d) W:%u, H:%u -> (%d, %d)\n", pen_x, pen_y, digi1.width(), digi1.height(), x_in_tablet, y_in_tablet);      
      tft.fillRect(x_in_tablet-1, y_in_tablet-10, 3, 21, BLUE);
      tft.fillRect(x_in_tablet-10, y_in_tablet-1, 21, 3, BLUE);
      if ((x_in_tablet - 10) < tablet_changed_area_x_min) tablet_changed_area_x_min = x_in_tablet - 10;
      if ((x_in_tablet + 10) > tablet_changed_area_x_max) tablet_changed_area_x_max = x_in_tablet + 10;
      if ((y_in_tablet - 10) < tablet_changed_area_y_min) tablet_changed_area_y_min = y_in_tablet - 10;
      if ((y_in_tablet + 10) > tablet_changed_area_y_max) tablet_changed_area_y_max = y_in_tablet + 10;
    }
    
  } else if (evt == WacomController::TOUCH) {
    uint8_t touch_count = digi1.getTouchCount();
    for (uint8_t i = 0; i < touch_count; i++) {
      int finger_x = digi1.getX(i);
      int finger_y = digi1.getY(i);
      int x_in_tablet = map(finger_x, 0, digi1.touchWidth(), 0, tab_draw_width) + x;
      int y_in_tablet = map(finger_y, 0, digi1.touchHeight(), 0, tab_draw_height) + y_start_graphics;
      //Serial.printf("Pen: (%d, %d) W:%u, H:%u -> (%d, %d)\n", finger_x, finger_y, digi1.width(), digi1.height(), x_in_tablet, y_in_tablet);      
      tft.fillRect(x_in_tablet-1, y_in_tablet-10, 3, 21, RED);
      tft.fillRect(x_in_tablet-10, y_in_tablet-1, 21, 3, RED);
      if ((x_in_tablet - 10) < tablet_changed_area_x_min) tablet_changed_area_x_min = x_in_tablet - 10;
      if ((x_in_tablet + 10) > tablet_changed_area_x_max) tablet_changed_area_x_max = x_in_tablet + 10;
      if ((y_in_tablet - 10) < tablet_changed_area_y_min) tablet_changed_area_y_min = y_in_tablet - 10;
      if ((y_in_tablet + 10) > tablet_changed_area_y_max) tablet_changed_area_y_max = y_in_tablet + 10;      
    }
    
  }
  tft.setClipRect();
  g_redraw_all = false;

  return true;
}

//=============================================================================
// UpdateActiveDeviceInfo
//=============================================================================
void UpdateActiveDeviceInfo() {
  // First see if any high level devices
  for (uint8_t i = 0; i < CNT_DEVICES; i++) {
    if (*drivers[i] != driver_active[i]) {
      if (driver_active[i]) {
        Serial.printf("*** Device %s - disconnected ***\n", driver_names[i]);
        driver_active[i] = false;

      } else {
        g_redraw_all = true;
        Serial.printf("*** Device %s %x:%x - connected ***\n", driver_names[i], drivers[i]->idVendor(), drivers[i]->idProduct());
        driver_active[i] = true;
        tft.fillScreen(BLACK);  // clear the screen.
        tft.setCursor(0, 0);
        tft.setTextColor(YELLOW);
        tft.setFont(Arial_12);
        tft.printf("Device %s %x:%x\n", driver_names[i], drivers[i]->idVendor(), drivers[i]->idProduct());

        const uint8_t *psz = drivers[i]->manufacturer();
        if (psz && *psz) tft.printf("  manufacturer: %s\n", psz);
        psz = drivers[i]->product();
        if (psz && *psz) tft.printf("  product: %s\n", psz);
        psz = drivers[i]->serialNumber();
        if (psz && *psz) tft.printf("  Serial: %s\n", psz);
        tft.updateScreen();  // update the screen now
      }
    }
  }
  // Then Hid Devices
  for (uint8_t i = 0; i < CNT_HIDDEVICES; i++) {
    if (*hiddrivers[i] != hid_driver_active[i]) {
      if (hid_driver_active[i]) {
        Serial.printf("*** HID Device %s - disconnected ***\n", hid_driver_names[i]);
        hid_driver_active[i] = false;
        if (!digi1) {
          tft.fillScreen(BLACK);
          tft.setTextColor(YELLOW);
          tft.setTextSize(2);
          tft.setCursor(CENTER, CENTER);
          tft.println("Waiting for Device...");
          tft.updateScreen();  // update the screen now
        }
      } else {
        g_redraw_all = true;
        Serial.printf("*** HID Device %s %x:%x - connected ***\n", hid_driver_names[i], hiddrivers[i]->idVendor(), hiddrivers[i]->idProduct());
        hid_driver_active[i] = true;
        tft.fillScreen(BLACK);  // clear the screen.
        tft.setCursor(0, 0);
        tft.setTextColor(YELLOW);
        tft.setFont(Arial_12);
        tft.printf("HID Device %s %x:%x\n", hid_driver_names[i], hiddrivers[i]->idVendor(), hiddrivers[i]->idProduct());

        const uint8_t *psz = hiddrivers[i]->manufacturer();
        if (psz && *psz) tft.printf("  manufacturer: %s\n", psz);
        psz = hiddrivers[i]->product();
        if (psz && *psz) tft.printf("  product: %s\n", psz);
        psz = hiddrivers[i]->serialNumber();
        if (psz && *psz) tft.printf("  Serial: %s\n", psz);
        tft.updateScreen();  // update the screen now
      }
    }
  }
}

//=============================================================================
// OutputNumberField
//=============================================================================
void OutputNumberField(int16_t x, int16_t y, int val, int16_t field_width) {
  int16_t x2, y2;
  tft.setCursor(x, y);
  tft.print(val, DEC);
  tft.getCursor(&x2, &y2);
  tft.fillRect(x2, y, field_width - (x2 - x), Arial_12.line_space, BLACK);
}


void MaybeSetupTextScrollArea() {
  if (g_redraw_all) {
    BT = 0;
    g_redraw_all = false;
  }
  if (BT == 0) {
    tft.enableScroll();
    tft.setScrollTextArea(20, 70, 280, 140);
    tft.setScrollBackgroundColor(GREEN);
    tft.setFont(Arial_11);
    tft.setTextColor(BLACK);
    tft.setCursor(20, 70);
    BT = 1;
  }
}