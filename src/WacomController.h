#ifndef _WACOM_CONTROLLER_H_
#define _WACOM_CONTROLLER_H_
#include <Arduino.h>
#include <USBHost_t36.h>



// From USBHost WacomController
class WacomController : public USBHIDInput, public BTHIDInput {
public:
  WacomController(USBHost &host) {
    init();
  }

  // Has there been a new event from the tablet
  bool available() {
    return digitizerEvent;
  }

  // WHat type of event
  // TOUCH - For those tablets that support using finger(s)
  // PEN - Pen Input
  // FRAME - Buttons or other controls on edges of tablet
  typedef enum {NONE= 0, MOUSE, TOUCH, PEN, FRAME} event_type_t;
  event_type_t eventType() {return event_type_;}

  // Clear the tablet data for the last event
  void digitizerDataClear();

  // Methods for Touch and Pen

  // Number of touches (1 for Pen)
  int getTouchCount() {return touch_count_;}

  // Retrieve the X for the give touch index (defaults to first)
  int getX(uint8_t index = 0) {
    return (index < MAX_TOUCH)? touch_x_[index] : 0xffff;
  }

  // Retrieve the Y for the give touch index (defaults to first)
  int getY(uint8_t index = 0) {
    return (index < MAX_TOUCH)? touch_y_[index] : 0xffff ;
  }

  // O
  uint32_t getPenButtons() {
    return buttons;
  }


  uint16_t getPenPressure() { return pen_pressure_;  }  
  uint16_t getPenDistance() { return pen_distance_; }
  int16_t getPenTiltX() { return pen_tilt_x_; }
  int16_t getPenTiltY() { return pen_tilt_y_; }

  uint16_t getFrameWheel() { return side_wheel_; }
  bool getFrameWheelButton() { return side_wheel_button_;}
  uint16_t getFrameTouchButtons() { return side_touch_buttons_; }
  uint16_t getFrameButtons() { return side_press_buttons_; }
    
  

  int getWheel() {
    return wheel;
  }
  int getWheelH() {
    return wheelH;
  }
  int getAxis(uint32_t index) {
    return (index < (sizeof(digiAxes) / sizeof(digiAxes[0]))) ? digiAxes[index] : 0;
  }

  void debugPrint(bool fOn) {
    debugPrint_ = fOn;
  }
  bool debugPrint() {
    return debugPrint_;
  }
  enum {MAX_TOUCH = 16};
protected:
  virtual hidclaim_t claim_collection(USBHIDParser *driver, Device_t *dev, uint32_t topusage);
  virtual void hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax);
  virtual void hid_input_data(uint32_t usage, int32_t value);
  virtual void hid_input_end();
  virtual void disconnect_collection(Device_t *dev);
	virtual bool hid_process_control(const Transfer_t *transfer);
  virtual bool hid_process_in_data(const Transfer_t *transfer);

  typedef struct {
	uint16_t idVendor;
    uint16_t idProduct; // Product ID
    uint16_t x_max;
    uint16_t y_max;
    uint16_t pressure_max;
    uint16_t distance_max;
	uint8_t report_id;
	uint8_t report_value;
    uint16_t type;
    uint16_t x_resolution;
    uint16_t y_resolution;
    uint8_t touch_max;
  } tablet_info_t;

  static const tablet_info_t s_tablets_info[];
  
private:
  void init();
  uint32_t wacom_equivalent_usage(uint32_t usage);
  bool decodeBamboo_PT(const uint8_t *buffer, uint16_t len);
  bool decodeIntuosHT(const uint8_t *buffer, uint16_t len);
  bool decodeIntuos5(const uint8_t *buffer, uint16_t len);
  bool decodeIntuos4(const uint8_t *buffer, uint16_t len);
  bool decodeH640P(const uint8_t *buffer, uint16_t len);
  bool decodeIntuos4100(const uint8_t *buffer, uint16_t len);
  
  enum {BUFFER_SIZE = 100};
  uint8_t buffer_[BUFFER_SIZE];

  void maybeSendSetupControlPackets();
  uint8_t getDescString(uint32_t bmRequestType, uint32_t bRequest, uint32_t wValue, uint32_t wIndex,
    uint16_t length, uint8_t *buffer );
  uint8_t getParameters(uint32_t bmRequestType, uint32_t bRequest, uint32_t wValue, 
	uint32_t wIndex, uint16_t length, uint8_t *buffer );

  uint8_t convertBufferToAscii();

  volatile uint8_t control_packet_pending_state_ = 0;
  uint8_t ignore_count_ = 2; // hack ignore a few if unexpected type

  inline uint16_t __get_unaligned_be16(const uint8_t *p)
  {
	return p[0] << 8 | p[1];
  }

  inline uint16_t __get_unaligned_le16(const uint8_t *p)
  {
	return p[0] | p[1] << 8;
  }

  uint8_t collections_claimed = 0;
  volatile bool digitizerEvent = false;
  volatile uint8_t hid_input_begin_count_ = 0;
  volatile uint8_t digiAxes_index_ = 0;  
  uint32_t buttons = 0;
  int touch_x_[MAX_TOUCH];
  int touch_y_[MAX_TOUCH];
  int touch_count_ = 0;
  uint16_t pen_pressure_ = 0;
  uint16_t pen_distance_ = 0;
  int16_t pen_tilt_x_ = 0;
  int16_t pen_tilt_y_ = 0;
  uint16_t side_wheel_;
  bool side_wheel_button_;
  uint16_t side_touch_buttons_;
  uint16_t side_press_buttons_;
  int wheel = 0;
  int wheelH = 0;
  int digiAxes[16];
  bool debugPrint_ = true;
  bool sendSetupPacket_ = true;
  USBHIDParser *driver_ = nullptr;
  uint16_t idProduct_;
  uint8_t tablet_info_index_ = 0xff;
  event_type_t event_type_ = NONE;
};
#endif
