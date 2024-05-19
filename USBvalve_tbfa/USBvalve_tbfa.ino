/*********************************************************************

  USBvalve TryBreakFixAgain MOD (Trafficlight)
  
  Original written by Cesare Pizzi
  https://github.com/cecio/USBvalve
  It´s a nice Guy!

  All changes can be found via tbfa and TryBreakFixAgain
  
*********************************************************************/

/*********************************************************************

  USBvalve
  
  written by Cesare Pizzi
  This project extensively reuse code done by Adafruit and TinyUSB. 
  Please support them!

*********************************************************************/

/*********************************************************************
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  Copyright (c) 2019 Ha Thach for Adafruit Industries
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/


// Uncomment one ore more defines to compile the MODS you want to use

/* 
 * #define TBFA_TRAFFIC = TrafficLight Mod
 * #define TBFA_LIPO = Pimoroni Pico LiPo Battery Mod
 * #define TBFA_SDLOG = SD-CARD Logger Mod
 * #define TBFA_ONBORDLED = Use Onbord LED on Pico
 * #define TBFA_ONBORDLEDW = Use Onbord LED on PicoW
 * #define TBFA_ESP01SRV = Use ESP01 as Webserver
 */

//#define TBFA_TRAFFIC
//#define TBFA_LIPO
//#define TBFA_SDLOG
//#define TBFA_ONBORDLED
//#define TBFA_ONBORDLEDW
//#define TBFA_ESP01SRV

#include <pio_usb.h>
#include "Adafruit_TinyUSB.h"
#include <XxHash_arduino.h>
#include <pico/stdlib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if defined(TBFA_ONBORDLED)
// LED Pin. If solid GREEN everything is OK, otherwise it will be put OFF
#define LED_PIN   25
#endif

// START TryBreakFixAgain MODS

/* TryBreakFixAgain Language MOD
 * Using Json for later modifications
 * definition in setup
 * rebuilded the printout()
 * variables, functions ... use the prefix tbfa_
 */

#include <ArduinoJson.h>
JsonDocument tbfa_dictonary;
char bufferx[200];

#if defined(TBFA_TRAFFIC)
/* TryBreakFixAgain TrafficLight MOD
 * Using a Ws2812b DOT (https://www.amazon.com/dp/B088K8DVMQ) ON Pin 10 (GPIO7)
 * Powerd on 3.3V at 5v the brightness is too high for me
 *
 * All TrafficLight Functions at the end of file.
 * config in setup()
 */

#include <NeoPixelConnect.h>
#define tbfa_PixelPin 7 // PixelData Pin
#define tbfa_PixelNum 1 // Pixel count
NeoPixelConnect pixels(tbfa_PixelPin, tbfa_PixelNum, pio0, 1);
//Set and Protect LED Status
boolean tbfa_lightbreak = false;
int tbfa_lightstatus = 0;
#endif

#if defined(TBFA_LIPO)
/* TryBreakFixAgain LiPo MOD
 * Using a Pimoroni Pico LiPo https://shop.pimoroni.com/products/pimoroni-pico-lipo 
 * Battery status on GPIO29 /  GPIO24 status if USB is connected or not
 * change tbfa_full_battery and tbfa_empty_battery to your Battery
 * Using a LP402025 +3.7V 150mAh https://shop.pimoroni.com/products/lipo-battery-pack?variant=20429081991
 */
const int tbfa_batPin = 29;
const int tbfa_usbPower = 24;
int tbfa_batValue = 0;
float tbfa_conversion_factor = 3 * 3.3 / 65535;
float tbfa_full_battery = 4.2;
float tbfa_empty_battery = 3.4;
int tbfa_charging = 0;
int tbfa_batcount = 0;
boolean tbfa_jumpbat = false;
#endif


#if defined(TBFA_SDLOG)
/* TryBreakFixAgain SD-CARD Logger MOD
 * Using a Waveshare Micro SD Board https://www.amazon.com/dp/B00KM6WO0Q
 */
const int _MISO = 16;  // AKA SPI RX
const int _MOSI = 19;  // AKA SPI TX
const int _SCK = 18;
const int _CS = 17;
#include <SD.h>
File dataLog;
#endif

// END TryBreakFixAgain MODS

//
// BADUSB detector section
//

/*
 * Requirements:
 * - [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB) library
 * - 2 consecutive GPIOs: D+ is defined by HOST_PIN_DP (gpio2), D- = D+ +1 (gpio3)
 * - CPU Speed must be either 120 or 240 Mhz. Selected via "Menu -> CPU Speed"
 */

#define HOST_PIN_DP 14      // Pin used as D+ for host, D- = D+ + 1
#define LANGUAGE_ID 0x0407  // English

// USB Host object
Adafruit_USBH_Host USBHost;

// END of BADUSB detector section


#define I2C_ADDRESS 0x3C  // 0X3C+SA0 - 0x3C or 0x3D
#define RST_PIN -1        // Define proper RST_PIN if required.
#define OLED_WIDTH  128
#define OLED_HEIGHT 32    // 64 or 32 depending on the OLED

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, RST_PIN);


// Define the dimension of RAM DISK. We have a "real" one (for which
// a real array is created) and a "fake" one, presented to the OS
#define DISK_BLOCK_NUM 0x150
#define FAKE_DISK_BLOCK_NUM 0x800
#define DISK_BLOCK_SIZE 0x200
#include "ramdisk.h"

Adafruit_USBD_MSC usb_msc;

// Eject button to demonstrate medium is not ready e.g SDCard is not present
// whenever this button is pressed and hold, it will report to host as not ready
#if defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS) || defined(ARDUINO_NRF52840_CIRCUITPLAY)
#define BTN_EJECT 4  // Left Button
bool activeState = true;

#elif defined(ARDUINO_FUNHOUSE_ESP32S2)
#define BTN_EJECT BUTTON_DOWN
bool activeState = true;

#elif defined PIN_BUTTON1
#define BTN_EJECT PIN_BUTTON1
bool activeState = false;
#endif

//
// USBvalve globals
//
#define VERSION "USBvalve-0.18.2 Mod"
boolean readme = false;
boolean autorun = false;
boolean written = false;
boolean deleted = false;
boolean written_reported = false;
boolean deleted_reported = false;
boolean hid_sent = false;
boolean hid_reported = false;

//
// Anti-Detection settings.
//
// Set USB IDs strings and numbers, to avoid possible detections.
// Remember that you can cusotmize FAKE_DISK_BLOCK_NUM as well
// for the same reason. Also DISK_LABEL in ramdisk.h can be changed.
//
// You can see here for inspiration: https://the-sz.com/products/usbid/
//
// Example:
//             0x0951 0x16D5    VENDORID_STR: Kingston   PRODUCTID_STR: DataTraveler
//
#define USB_VENDORID 0x0951               // This override the Pi Pico default 0x2E8A
#define USB_PRODUCTID 0x16D5              // This override the Pi Pico default 0x000A
#define USB_DESCRIPTOR "DataTraveler"     // This override the Pi Pico default "Pico"
#define USB_MANUF "Kingston"              // This override the Pi Pico default "Raspberry Pi"
#define USB_SERIAL "123456789A"           // This override the Pi Pico default. Disabled by default. \
                                          // See "setSerialDescriptor" in setup() if needed
#define USB_VENDORID_STR "Kingston"       // Up to 8 chars
#define USB_PRODUCTID_STR "DataTraveler"  // Up to 16 chars
#define USB_VERSION_STR "1.0"             // Up to 4 chars

#define BLOCK_AUTORUN 102       // Block where Autorun.inf file is saved
#define BLOCK_README 100        // Block where README.txt file is saved
#define MAX_DUMP_BYTES 16       // Used by the dump of the debug facility: do not increase this too much
#define BYTES_TO_HASH 512 * 2   // Number of bytes of the RAM disk used to check consistency
#define BYTES_TO_HASH_OFFSET 7  // Starting sector to check for consistency (FAT_DIRECTORY is 7)

// Burned hash to check consistency
uint valid_hash = 2362816530;

// Core 0 Setup: will be used for the USB mass device functions
void setup() {
  // START TryBreakFixAgain Language definitions
  tbfa_dictonary["selfok"][0] ="[+] Selftest: OK";
  tbfa_dictonary["selfko"][0] ="[!] Selftest: KO";
  tbfa_dictonary["stop"][0] ="[!] Stopping...";
  tbfa_dictonary["readme"][0] ="[!] README (R)";
  tbfa_dictonary["autorun"][0] ="[+] AUTORUN (R)";
  tbfa_dictonary["deleting"][0] = "[!] DELETING";
  tbfa_dictonary["write"][0] ="[!] WRITING";
  tbfa_dictonary["hidsend"][0] ="[!!] HID Sending data";
  tbfa_dictonary["reset"][0] ="[+] RESETTING";
  tbfa_dictonary["hiddev"][0] ="[!!] HID Device";
  tbfa_dictonary["massdev"][0] ="[++] Mass Device";
  tbfa_dictonary["cdcdev"][0] ="[++] CDC Device";
  tbfa_dictonary["version"][0] =VERSION;
  tbfa_dictonary["cardok"][0] ="[+] SD OK";
  tbfa_dictonary["cardko"][0] ="[!] SD ERROR";
  tbfa_dictonary["bigfile"][0] ="[!] BIG LOG";
  tbfa_dictonary["espip"][0] ="";
  // END TryBreakFixAgain Language definitions

#if defined(TBFA_TRAFFIC)
  // START TryBreakFixAgain Trafficlight definitions
  // values as integer 0= Off, 1=Blue , 2=Green , 3=Orange 4=Red
  tbfa_dictonary["selfok"][1] = 2;
  tbfa_dictonary["selfko"][1] = 4;
  tbfa_dictonary["stop"][1] = 4;
  tbfa_dictonary["readme"][1] = 3;
  tbfa_dictonary["autorun"][1] = 2;
  tbfa_dictonary["deleting"][1] = 3;
  tbfa_dictonary["write"][1] =" 3";
  tbfa_dictonary["hidsend"][1] = 4;
  tbfa_dictonary["reset"][1] = 0;
  tbfa_dictonary["hiddev"][1] = 4;
  tbfa_dictonary["massdev"][1] = 2;
  tbfa_dictonary["cdcdev"][1] = 2;
  tbfa_dictonary["version"][1] = 0;
  tbfa_dictonary["cardok"][1] = 0;
  tbfa_dictonary["cardko"][1] = 0;
  tbfa_dictonary["bigfile"][1] = 3;
  tbfa_dictonary["espip"][1] = "1";
  // END TryBreakFixAgain Language definitions
#endif
  // Change all the USB Pico settings
  TinyUSBDevice.setID(USB_VENDORID, USB_PRODUCTID);
  TinyUSBDevice.setProductDescriptor(USB_DESCRIPTOR);
  TinyUSBDevice.setManufacturerDescriptor(USB_MANUF);
  // This could be used to change the serial number as well
  // TinyUSBDevice.setSerialDescriptor(USB_SERIAL);

#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
  // Manual begin() is required on core without built-in support for TinyUSB such as
  // - mbed rp2040
  TinyUSB_Device_Init(0);
#endif

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID(USB_VENDORID_STR, USB_PRODUCTID_STR, USB_VERSION_STR);

  // Set disk size (using the "fake" size)
  usb_msc.setCapacity(FAKE_DISK_BLOCK_NUM, DISK_BLOCK_SIZE);

  // Set the callback functions
  usb_msc.setReadWriteCallback(msc_read_callback, msc_write_callback, msc_flush_callback);

  // Set Lun ready (RAM disk is always ready)
  usb_msc.setUnitReady(true);

#ifdef BTN_EJECT
  pinMode(BTN_EJECT, activeState ? INPUT_PULLDOWN : INPUT_PULLUP);
  usb_msc.setReadyCallback(msc_ready_callback);
#endif

  // Check consistency of RAM FS
  // Add 11 bytes to skip the DISK_LABEL from the hashing
  // The startup of the USB has been moved before initialization of the 
  // screen because sometimes it inserts some delay preventing
  // proper initialization of the mass device
  uint computed_hash;
  computed_hash = XXH32(msc_disk[BYTES_TO_HASH_OFFSET] + 11, BYTES_TO_HASH, 0);
  if (computed_hash == valid_hash) {
      usb_msc.begin();
  }

  // Screen Init
#if RST_PIN >= 0
  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS, RST_PIN);
#else
  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
#endif

  display.setTextSize(1);
  cls();  // Clear display

#if defined(TBFA_ESP01SRV)
  // START TryBreakFixAgain Serial on PIN 8/9
  Serial2.setRX(9);
  Serial2.setTX(8);
  Serial2.begin(115200);
  // END TryBreakFixAgain Serial on PIN 8/9
#endif

#if defined(TBFA_TRAFFIC)
  // START TryBreakFixAgain TrafficLight LED-DEMO
  tbfa_trafficlightstart();
  // END TryBreakFixAgain TrafficLight LED-DEMO
#endif

  // Now outputs the result of the check
  if (computed_hash == valid_hash) {
    printout("selfok");
  } else {
    printout("selfko");
    printout("stop");
    while (1) {
      delay(1000);  // Loop forever
    }
  }

#if defined(TBFA_SDLOG)
  // START TryBreakFixAgain SD-CARD config & check
  SPI.setRX(_MISO);
  SPI.setTX(_MOSI);
  SPI.setSCK(_SCK);
  if (!SD.begin(_CS)) {
    printout("cardko");
    delay(500);
    SerialTinyUSB.println("Card failed, or not present");  
  }else{
    printout("cardok");
    delay(500);
    SerialTinyUSB.println("card initialized.");
    if (SD.exists("datalog.txt")) {
      File fproof;
      fproof=SD.open("datalog.txt");
      if((int)fproof.size() > 5242880){
        printout("bigfile");
        SerialTinyUSB.println("datalog.txt bigger then 5MB.");
      }
      fproof.close();
    }
    tbfa_datalogger("\r\n\r\n----------NEW SESSION----------\r\n\r\n", 0);
  }
  // END TryBreakFixAgain SD-CARD config & check
  
#endif

#if defined(TBFA_ONBORDLED)
  // Set up led PIN
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);
#endif

#if defined(TBFA_ONBORDLEDW)
  // Set up led PIN PicoW
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
#endif

}

// Core 1 Setup: will be used for the USB host functions for BADUSB detector
void setup1() {
  // Set a custom clock (multiple of 12Mhz) to achieve maximum compatibility for HID
  set_sys_clock_khz(144000, true);

  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = HOST_PIN_DP;
  USBHost.configure_pio_usb(1, &pio_cfg);

  // run host stack on controller (rhport) 1
  // Note: For rp2040 pico-pio-usb, calling USBHost.begin() on core1 will have most of the
  // host bit-banging processing works done in core1
  USBHost.begin(1);
}

// Main Core0 loop: managing display
void loop() {

  if (readme == true) {
    printout("readme");
    readme = false;

#if defined(TBFA_ONBORDLED)
    gpio_put(LED_PIN, 0);         // Turn Off LED
#endif
#if defined(TBFA_ONBORDLEDW)
    digitalWrite(LED_BUILTIN, LOW);         // Turn Off LED
#endif

  }

  if (autorun == true) {
    printout("autorun");
    autorun = false;
  }

  if (deleted == true && deleted_reported == false) {
    printout("deleting");
    deleted = false;
    deleted_reported = true;

#if defined(TBFA_ONBORDLED)
    gpio_put(LED_PIN, 0);         // Turn Off LED
#endif
#if defined(TBFA_ONBORDLEDW)
    digitalWrite(LED_BUILTIN, LOW);         // Turn Off LED
#endif

  }

  if (written == true && written_reported == false) {
    printout("write");
    written = false;
    written_reported = true;

#if defined(TBFA_ONBORDLED)
    gpio_put(LED_PIN, 0);         // Turn Off LED
#endif
#if defined(TBFA_ONBORDLEDW)
    digitalWrite(LED_BUILTIN, LOW);         // Turn Off LED
#endif

  }

  if (hid_sent == true && hid_reported == false) {
    printout("hidsend");
    hid_sent = false;
    hid_reported = true;

#if defined(TBFA_ONBORDLED)
    gpio_put(LED_PIN, 0);         // Turn Off LED
#endif
#if defined(TBFA_ONBORDLEDW)
    digitalWrite(LED_BUILTIN, LOW);         // Turn Off LED
#endif

  }

  if (BOOTSEL) {
    printout("reset");
#if defined(TBFA_ESP01SRV)
  // END TryBreakFixAgain Serial on PIN 8/9
    Serial2.println("ESPRESET");
  // END TryBreakFixAgain Serial on PIN 8/9
#endif
    swreset();
  }
#if defined(TBFA_LIPO)
// START TryBreakFixAgain Battery check
  tbfa_batteryPack();
// END TryBreakFixAgain Battery check
#endif
#if defined(TBFA_ESP01SRV)
  if (Serial2.available() > 0) {
    String tempser = Serial2.readString();
    if(tempser.indexOf("IP: ") > -1 ){
      String tempser2= tempser.substring(tempser.indexOf("IP: ") +4 );
      tempser2.trim();
      if(tempser2!=""){
        tbfa_dictonary["espip"][0] = tempser2;
        printout("espip");
        tempser2="";
      }
    } 
  }
  // END TryBreakFixAgain Serial on PIN 8/9
#endif
}

// Main Core1 loop: managing USB Host
void loop1() {
  USBHost.task();
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size).
// This happens only for the "real" size of disk
int32_t msc_read_callback(uint32_t lba, void* buffer, uint32_t bufsize) {

  // Check for README.TXT
  if (lba == BLOCK_README) {
    readme = true;
  }

  // Check for AUTORUN.INF
  if (lba == BLOCK_AUTORUN) {
    autorun = true;
  }

  // We are declaring a bigger size than what is actually allocated, so
  // this is protecting our memory integrity
  if (lba < DISK_BLOCK_NUM - 1) {
    uint8_t const* addr = msc_disk[lba];
    memcpy(buffer, addr, bufsize);
  }

  tbfa_print(String("Read LBA: "));
  tbfa_print(String(lba));
  tbfa_print(String("   Size: "));
  tbfa_println(String(bufsize));
  if (lba < DISK_BLOCK_NUM - 1) {
    hexDump(msc_disk[lba], MAX_DUMP_BYTES);
  }
  tbfa_flush();

  return bufsize;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size).
// This happens only for the "real" size of disk
int32_t msc_write_callback(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {

  // Check for file deletion at Block 7
  // The first char of filename is replaced with 0xE5, we are going
  // to check for it
  if (lba == 7) {
    if (buffer[32] == 0xE5 || buffer[64] == 0xE5 || buffer[160] == 0xE5) {
      deleted = true;
    }
  }

  // This check for writing of space. The LBA > 10 is set to avoid some
  // false positives, in particular on Windows Systems
  if (lba > 10) {
    written = true;
  }

  // We are declaring a bigger size than what is actually allocated, so
  // this is protecting our memory integrity
  if (lba < DISK_BLOCK_NUM - 1) {
    // Writing buffer to "disk"
    uint8_t* addr = msc_disk[lba];
    memcpy(addr, buffer, bufsize);
  }

  tbfa_print(String("Write LBA: "));
  tbfa_print(String(lba));
  tbfa_print(String("   Size: "));
  tbfa_println(String(bufsize));
  if (lba < DISK_BLOCK_NUM - 1) {
    hexDump(msc_disk[lba], MAX_DUMP_BYTES);
  }
  tbfa_flush();

  return bufsize;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_callback(void) {
  // Nothing to do
}

#ifdef BTN_EJECT
// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool msc_ready_callback(void) {
  // button not active --> medium ready
  return digitalRead(BTN_EJECT) != activeState;
}
#endif


void scrollUp(uint8_t pixels) {
  // Read the current content of the display, shift it up by 'pixels' rows
  display.startscrollright(0x00, 0x07); // Dummy values to initiate scroll
  display.stopscroll(); // Immediately stop to manually shift pixels in memory
  // TryBreakFixAgain Mod for Static first Rows set i from 0 to 16
  for (int i = 16; i < display.height() - pixels; i++) {
    for (int j = 0; j < display.width(); j++) {
      uint8_t color = display.getPixel(j, i + pixels);
      display.drawPixel(j, i, color);
    }
  }

  // Clear the freed space after scrolling
  display.fillRect(0, display.height() - pixels, display.width(), pixels, SSD1306_BLACK);
  // Refresh the display to show the changes
  display.display();
}

void checkAndScroll() {
  // Assumes text height of 8 pixels, but check for 16 because newline is not used
  if ((display.getCursorY() + 16) > display.height()) {
    // Scroll up by 8 pixels
    scrollUp(8);
    display.setCursor(0, display.getCursorY() - 8);
  }
}

// START TryBreakFixAgain modifikated printout()
void printout(const char *ctrl)
{
  // TryBreakFixAgain MOD
  // Normal Printout
  if(ctrl != "cls"){
    String str=tbfa_dictonary[ctrl][0];
    checkAndScroll();
    display.println(str);
    tbfa_println(str);
  }else{
    // Printout first 2 Rows on cls();
    display.setCursor(0, 0);
    display.fillRect(0, 0, display.width(), 16, SSD1306_BLACK);
    display.print(String(tbfa_dictonary["version"][0]));
    display.drawLine(0, 10, display.width(), 10, SSD1306_WHITE);
    display.setCursor(0, 16);
  }
  display.display();

#if defined(TBFA_TRAFFIC)
  //START TryBreakFixAgain TrafficLight SETSTATUS
  int tbfa_led=tbfa_dictonary[ctrl][1];
  tbfa_trafficlight(tbfa_led);
  if(tbfa_led == 4){
    tbfa_lightbreak = true;
  }
  if(ctrl == "selfok" || ctrl == "autorun" || ctrl == "espip"){
    delay(2000);
    tbfa_trafficlight(0);
  }
  //END TryBreakFixAgain TrafficLight SETSTATUS 
#endif

}
// END TryBreakFixAgain modifikated printout()


// Clear display
void cls(void) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  // TryBreakFixAgain First Two Rows Mod
  printout("cls");
}


// HexDump
void hexDump(unsigned char* data, size_t size) {
  char asciitab[17];
  size_t i, j;
  asciitab[16] = '\0';

  for (i = 0; i < size; ++i) {

    tbfa_print(String(data[i] >> 4, HEX));
    tbfa_print(String(data[i] & 0x0F, HEX));

    if ((data)[i] >= ' ' && (data)[i] <= '~') {
      asciitab[i % 16] = (data)[i];
    } else {
      asciitab[i % 16] = '.';
    }
    if ((i + 1) % 8 == 0 || i + 1 == size) {
      tbfa_print(" ");
      if ((i + 1) % 16 == 0) {
        tbfa_println(asciitab);
      } else if (i + 1 == size) {
        asciitab[(i + 1) % 16] = '\0';
        if ((i + 1) % 16 <= 8) {
          tbfa_print(" ");
        }
        for (j = (i + 1) % 16; j < 16; ++j) {
          tbfa_print("   ");
        }
        tbfa_print("|  ");
        tbfa_println(String(asciitab));
      }
    }
  }
  tbfa_println("");
}

// Reset the Pico
void swreset() {
  watchdog_enable(1500, 1);
  while (1)
    ;
}

//
// BADUSB detector section
//

static uint8_t const keycode2ascii[128][2] = { HID_KEYCODE_TO_ASCII };

// Invoked when device with hid interface is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
  uint16_t vid, pid;
  const char* protocol_str[] = { "None", "Keyboard", "Mouse" };

  // Read the HID protocol
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  tuh_vid_pid_get(dev_addr, &vid, &pid);

  printout("hiddev");

#if defined(TBFA_ONBORDLED)
    gpio_put(LED_PIN, 0);         // Turn Off LED
#endif
#if defined(TBFA_ONBORDLEDW)
    digitalWrite(LED_BUILTIN, LOW);         // Turn Off LED
#endif

  sprintf(bufferx, "HID device address = %d, instance = %d mounted", dev_addr, instance);
  tbfa_println(bufferx);
  sprintf(bufferx, "VID = %04x, PID = %04x", vid, pid);
  tbfa_println(bufferx);
  sprintf(bufferx, "HID Interface Protocol = %s", protocol_str[itf_protocol]);
  tbfa_println(bufferx);

  if (!tuh_hid_receive_report(dev_addr, instance)) {
    tbfa_println("Error: cannot request to receive report");
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  sprintf(bufferx, "HID device address = %d, instance = %d unmounted", dev_addr, instance);
  tbfa_println("");
  tbfa_println(bufferx);

  // Reset HID sent flag
  hid_sent = false;
  hid_reported = false;
}

// Invoked when received report from device
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {

  static bool kbd_printed = false;
  static bool mouse_printed = false;

  // Used in main loop to write output to OLED
  hid_sent = true;

  // Read the HID protocol
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  switch (itf_protocol) {
    case HID_ITF_PROTOCOL_KEYBOARD:
      if (kbd_printed == false) {
        tbfa_println("HID received keyboard report");
        kbd_printed = true;
        mouse_printed = false;
      }
      process_kbd_report((hid_keyboard_report_t const*)report);
      break;

    case HID_ITF_PROTOCOL_MOUSE:
      if (kbd_printed == false) {
        tbfa_println("HID receive mouse report");
        mouse_printed = true;
        kbd_printed = false;
      }
      process_mouse_report((hid_mouse_report_t const*)report);
      break;

    default:
      // Generic report: for the time being we use kbd for this as well
      process_kbd_report((hid_keyboard_report_t const*)report);
      break;
  }

  if (!tuh_hid_receive_report(dev_addr, instance)) {
    tbfa_println("Error: cannot request to receive report");
  }
}

static inline bool find_key_in_report(hid_keyboard_report_t const* report, uint8_t keycode) {
  for (uint8_t i = 0; i < 6; i++) {
    if (report->keycode[i] == keycode) return true;
  }

  return false;
}

static void process_kbd_report(hid_keyboard_report_t const* report) {
  // Previous report to check key released
  static hid_keyboard_report_t prev_report = { 0, 0, { 0 } };

  for (uint8_t i = 0; i < 6; i++) {
    if (report->keycode[i]) {
      if (find_key_in_report(&prev_report, report->keycode[i])) {
        // Exist in previous report means the current key is holding
      } else {
        // Not existed in previous report means the current key is pressed

        // Check for modifiers. It looks that in specific cases, they are not correctly recognized (probably
        // for timing issues in fast input)
        bool const is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
        uint8_t ch = keycode2ascii[report->keycode[i]][is_shift ? 1 : 0];

        bool const is_gui = report->modifier & (KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_RIGHTGUI);
        if (is_gui == true) tbfa_print("GUI+");

        bool const is_alt = report->modifier & (KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_RIGHTALT);
        if (is_alt == true) tbfa_print("ALT+");

        // Check for "special" keys
        check_special_key(report->keycode[i]);

        // Finally, print out the decoded char
        sprintf(bufferx, "%c", ch);
        tbfa_print(bufferx);
        if (ch == '\r') tbfa_print("\n");  // New line for enter

        fflush(stdout);  // flush right away, else nanolib will wait for newline
      }
    }
  }

  prev_report = *report;
}

static void check_special_key(uint8_t code) {

  if (code == HID_KEY_ARROW_RIGHT) tbfa_print(String("<ARROWRIGHT>"));
  if (code == HID_KEY_ARROW_LEFT) tbfa_print(String("<ARROWLEFT>"));
  if (code == HID_KEY_ARROW_DOWN) tbfa_print(String("<ARROWDOWN>"));
  if (code == HID_KEY_ARROW_UP) tbfa_print(String("<ARROWUP>"));
  if (code == HID_KEY_HOME) tbfa_print(String("<HOME>"));
  if (code == HID_KEY_KEYPAD_1) tbfa_print(String("<KEYPAD_1>"));
  if (code == HID_KEY_KEYPAD_2) tbfa_print(String("<KEYPAD_2>"));
  if (code == HID_KEY_KEYPAD_3) tbfa_print(String("<KEYPAD_3>"));
  if (code == HID_KEY_KEYPAD_4) tbfa_print(String("<KEYPAD_4>"));
  if (code == HID_KEY_KEYPAD_5) tbfa_print(String("<KEYPAD_5>"));
  if (code == HID_KEY_KEYPAD_6) tbfa_print(String("<KEYPAD_6>"));
  if (code == HID_KEY_KEYPAD_7) tbfa_print(String("<KEYPAD_7>"));
  if (code == HID_KEY_KEYPAD_8) tbfa_print(String("<KEYPAD_8>"));
  if (code == HID_KEY_KEYPAD_9) tbfa_print(String("<KEYPAD_9>"));
  if (code == HID_KEY_KEYPAD_0) tbfa_print(String("<KEYPAD_0>"));
  if (code == HID_KEY_F1) tbfa_print(String("<F1>"));
  if (code == HID_KEY_F2) tbfa_print(String("<F2>"));
  if (code == HID_KEY_F3) tbfa_print(String("<F3>"));
  if (code == HID_KEY_F4) tbfa_print(String("<F4>"));
  if (code == HID_KEY_F5) tbfa_print(String("<F5>"));
  if (code == HID_KEY_F6) tbfa_print(String("<F6>"));
  if (code == HID_KEY_F7) tbfa_print(String("<F7>"));
  if (code == HID_KEY_F8) tbfa_print(String("<F8>"));
  if (code == HID_KEY_F9) tbfa_print(String("<F9>"));
  if (code == HID_KEY_F10) tbfa_print(String("<F10>"));
  if (code == HID_KEY_F11) tbfa_print(String("<F11>"));
  if (code == HID_KEY_F12) tbfa_print(String("<F12>"));
  if (code == HID_KEY_PRINT_SCREEN) tbfa_print(String("<PRNT>"));
  if (code == HID_KEY_SCROLL_LOCK) tbfa_print(String("<SCRLL>"));
  if (code == HID_KEY_PAUSE) tbfa_print(String("<PAUSE>"));
  if (code == HID_KEY_INSERT) tbfa_print(String("<INSERT>"));
  if (code == HID_KEY_PAGE_UP) tbfa_print(String("<PAGEUP>"));
  if (code == HID_KEY_DELETE) tbfa_print(String("<DEL>"));
  if (code == HID_KEY_END) tbfa_print(String("<END>"));
  if (code == HID_KEY_PAGE_DOWN) tbfa_print(String("<PAGEDOWN>"));
  if (code == HID_KEY_NUM_LOCK) tbfa_print(String("<ARROWRIGHT>"));
  if (code == HID_KEY_KEYPAD_DIVIDE) tbfa_print(String("<KEYPAD_DIV>"));
  if (code == HID_KEY_KEYPAD_MULTIPLY) tbfa_print(String("<KEYPAD_MUL>"));
  if (code == HID_KEY_KEYPAD_SUBTRACT) tbfa_print(String("<KEYPAD_SUB>"));
  if (code == HID_KEY_KEYPAD_ADD) tbfa_print(String("<KEYPAD_ADD>"));
  if (code == HID_KEY_KEYPAD_DECIMAL) tbfa_print(String("<KEYPAD_DECIMAL>"));
}

static void process_mouse_report(hid_mouse_report_t const* report) {
  static hid_mouse_report_t prev_report = { 0 };

  //------------- button state  -------------//
  uint8_t button_changed_mask = report->buttons ^ prev_report.buttons;
  if (button_changed_mask & report->buttons) {
        sprintf(bufferx, "MOUSE: %c%c%c ",
                         report->buttons & MOUSE_BUTTON_LEFT ? 'L' : '-',
                         report->buttons & MOUSE_BUTTON_MIDDLE ? 'M' : '-',
                         report->buttons & MOUSE_BUTTON_RIGHT ? 'R' : '-');
        tbfa_println(bufferx);
  }
  cursor_movement(report->x, report->y, report->wheel);
}

void cursor_movement(int8_t x, int8_t y, int8_t wheel) {
        sprintf(bufferx, "(%d %d %d)", x, y, wheel);
        tbfa_println(bufferx);
}

// END of BADUSB detector section

//
// OTHER Host devices detection section
//

// Invoked when a device with MassStorage interface is mounted
void tuh_msc_mount_cb(uint8_t dev_addr) {
  printout("massdev");
        sprintf(bufferx, "Mass Device attached, address = %d", dev_addr);
        tbfa_println(bufferx);
}

// Invoked when a device with MassStorage interface is unmounted
void tuh_msc_umount_cb(uint8_t dev_addr) {
        sprintf(bufferx, "Mass Device unmounted, address = %d", dev_addr);
        tbfa_println(bufferx);
}

// Invoked when a device with CDC (Communication Device Class) interface is mounted
void tuh_cdc_mount_cb(uint8_t idx) {
  printout("cdcdev");
        sprintf(bufferx, "CDC Device attached, idx = %d", idx);
        tbfa_println(bufferx);
}

// Invoked when a device with CDC (Communication Device Class) interface is unmounted
void tuh_cdc_umount_cb(uint8_t idx) {
        sprintf(bufferx, "CDC Device unmounted, idx = %d", idx);
        tbfa_println(bufferx);
}

// END of OTHER Host devices detector section


// START TryBreakFixAgain MOD Functions
#if defined(TBFA_TRAFFIC)
// START TryBreakFixAgain TrafficLight Functions
void tbfa_trafficlight(int tlight){
  
  if(tbfa_lightbreak == false){
    if(tlight == 1 && tbfa_lightstatus==2){
      tlight=2;
    }
    if(tlight == 1 && tbfa_lightstatus==4){
      tlight=4;
    }
    tbfa_lightstatus = tlight;
    switch (tlight) {
      case 1:
        pixels.neoPixelFill(0, 0, 255, true);
        break;
      case 2:
        pixels.neoPixelFill(0, 255, 0, true);
        break;
      case 3:
        pixels.neoPixelFill(255, 188, 0, true);
        break;
      case 4:
        pixels.neoPixelFill(255, 0, 0, true);
        break;
      default:
        pixels.neoPixelFill(0, 0, 0, true);
        pixels.neoPixelClear();
        break;
    }
  }
}

void tbfa_trafficlightstart(){
  tbfa_trafficlight(1);
  delay(300);
  tbfa_trafficlight(2);
  delay(300);
  tbfa_trafficlight(3);
  delay(300);
  tbfa_trafficlight(4);
  delay(300);
  tbfa_trafficlight(1);
  delay(300);
  tbfa_trafficlight(0);
}

/// DETECT GENERAL MOUNTING
void tuh_mount_cb (uint8_t dev_addr)
{
  tbfa_trafficlight(1);
}

/// DETECT GENERAL UNMOUNTING
void tuh_umount_cb(uint8_t dev_addr)
{
  //wait for 0.5 sec then Turn off LED;
  delay(500);
  tbfa_lightbreak = false;
  tbfa_trafficlight(0);
}
// END TryBreakFixAgain TrafficLight Functions
#endif

// START TryBreakFixAgain PRINT Functions
void tbfa_print(String in){
  SerialTinyUSB.print(in);
  tbfa_datalogger(in, 0);
}
void tbfa_println(String in){
  SerialTinyUSB.println(in);
  tbfa_datalogger(in, 1);
}
void tbfa_flush(){
  SerialTinyUSB.flush();
#if defined(TBFA_ESP01SRV)
  // START TryBreakFixAgain Serial on PIN 8/9
  Serial2.flush();
  // END TryBreakFixAgain Serial on PIN 8/9
#endif 
}
// END TryBreakFixAgain PRINT Functions

// START TryBreakFixAgain LOGGER Function for SD and WiFi
void tbfa_datalogger(String output, int where){
#if defined(TBFA_SDLOG)
  // START TryBreakFixAgain LOGGER SD-CARD
  dataLog = SD.open("datalog.txt", FILE_WRITE);
  if (dataLog) {
    if(where==0){
      dataLog.print(output);
    }else{
      dataLog.println(output);
    }
    dataLog.flush();
    dataLog.close();
  }else{
    SerialTinyUSB.println("Card not Ready");
  }
    // END TryBreakFixAgain LOGGER SD-CARD
#endif 
#if defined(TBFA_ESP01SRV)
  // START TryBreakFixAgain Serial on PIN 8/9
  if(where==0){
    Serial2.print(output);
  }else{
    Serial2.println(output);
  }
  // END TryBreakFixAgain Serial on PIN 8/9
#endif   
}
// END TryBreakFixAgain LOGGER Function for SD and WiFi

#if defined(TBFA_LIPO)
// START TryBreakFixAgain Battery Function for Pimoroni Pico LiPo 
void tbfa_batteryPack() {
    if (tbfa_batcount == 0) {
        if (tbfa_jumpbat == false) {
            tbfa_batValue = analogRead(tbfa_batPin);
            float tbfa_voltage = tbfa_batValue * tbfa_conversion_factor;
            float tbfa_percentage = 100 * ((tbfa_voltage - tbfa_empty_battery) / (tbfa_full_battery - tbfa_empty_battery));
            if (tbfa_percentage > 100) {
                tbfa_percentage = 100;
            } else if (tbfa_percentage < 0) {
                tbfa_percentage = 0;
            }
            int oldCursor = display.getCursorY();
            display.setCursor(0, 0);
            display.fillRect(0, 0, display.width(), 8, SSD1306_BLACK);
            if (digitalRead(tbfa_usbPower) == 1) {
                display.print("usbValve          USB");
                tbfa_charging = 1;
            } else {
                display.print("usbValve");
                int cleanprecent = tbfa_percentage;
                if (cleanprecent >= 99) {
                    display.print("         ");
                } else if (cleanprecent >= 9) {
                    display.print("          ");
                } else {
                    display.print("           ");
                }

                display.print(cleanprecent);
                display.println("% ");
                tbfa_charging = 0;
            }
            display.setCursor(0, oldCursor);
            display.display();
        }
    } else if (tbfa_batcount == 10000) {
        tbfa_batcount = -1;
    }
    tbfa_batcount++;
}
// END TryBreakFixAgain Battery Function for Pimoroni Pico LiPo 
#endif
//END TryBreakFixAgain MOD Functions
