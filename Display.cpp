#include "Display.h"

#include <Wire.h>
#include "wiring_private.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     1 // Reset pin #4 (or -1 if sharing Arduino reset pin)

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

static const unsigned char PROGMEM logoBmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

Display display;

Display::Display() : lcd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
{
}

Display::~Display() 
{
}

void Display::begin()
{
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if(!lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
	Serial.println(F("SSD1306 allocation failed"));
		while(true); // Don't proceed, loop forever
	}

	// Show initial display buffer contents on the screen --
	// the library initializes this with an Adafruit splash screen.
	lcd.display();
	delay(2000); // Pause for 2 seconds

	// Clear the buffer
	lcd.clearDisplay();

	showLogo();
}

void Display::showLogo()
{
	lcd.clearDisplay();

	lcd.drawBitmap((lcd.width() - LOGO_WIDTH ) / 2
		, (lcd.height() - LOGO_HEIGHT) / 2
		, logoBmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
	lcd.display();
	delay(1000);
}

void Display::setText(String text, uint16_t x, uint16_t y, uint16_t textSize, uint16_t color, uint16_t backgroundColor) 
{
  lcd.clearDisplay();

  lcd.setTextSize(textSize);                // Normal 1:1 pixel scale
  lcd.setTextColor(color, backgroundColor); // Draw white text
  lcd.setCursor(x, y);                      // Start at top-left corner
  lcd.println(text.c_str());
  lcd.display();
}

