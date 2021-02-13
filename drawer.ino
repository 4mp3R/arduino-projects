#include <EEPROM.h>

/**
 * Analog joystick
 * X  - A0
 * Y  - A1
 * SW - D2
 *
 * MAX72XX 8x8 LED matrix
 * 10 CLK      - D10
 * 11 Load(CS) - D11
 * 12 DataIn   - D12
 */
 
const byte displayCLK = 10;
const byte displayCS = 11;
const byte displayDIN = 12;

byte pointerX = 0, pointerY = 0;
volatile byte displayData[8] = {};
volatile unsigned long lastClickTime = 0;

void setup() {
  Serial.begin(9600);

  initDisplay();

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), onClick, FALLING);

  initFromSnapshot();
}

void loop() {
  int y = analogRead(0);
  int x = analogRead(1);

  if (x < 200) pointerX = constrain(pointerX + 1, 0, 7);
  else if (x > 800) pointerX = constrain(pointerX - 1, 0, 7);

  if (y < 200) pointerY = constrain(pointerY - 1, 0, 7);
  else if (y > 800) pointerY = constrain(pointerY + 1, 0, 7);

  draw();

  delay(200);
}

void initDisplay() {
  pinMode(displayCLK, OUTPUT);
  pinMode(displayCS, OUTPUT);
  pinMode(displayDIN, OUTPUT);

  displayWrite(0x09, 0x00);  // BCD decoding
  displayWrite(0x0a, 0x01);  // Brightness
  displayWrite(0x0b, 0x07);  // Scan limit 8 LEDs
  displayWrite(0x0c, 0x01);  // Exit power saving mode
  displayWrite(0x0f, 0x00);  // Display 0
}

void draw() {
  for (byte row = 0, addr = 0x01; row < 8; row++, addr++) {
    byte rowData = displayData[row];
    if (pointerX == row) {
      rowData |= 1 << pointerY; // Always show the cursor
    }
    displayWrite(addr, rowData);
  }
}

void onClick() {
  if (millis() - lastClickTime < 500) return; // 0.5s click debounce
  
  lastClickTime = millis();
  displayData[pointerX] ^= 1 << pointerY; // Toggle pointed LED
  EEPROM.write(pointerX, displayData[pointerX]);
}

void initFromSnapshot() {
  for (int row = 0; row < 8; row++) {
    displayData[row] = EEPROM.read(row);
  }
}

void displayWriteByte(byte data) {
  for (byte i = 0, mask = 0b10000000; i < 8; i++, mask >>= 1) {
    digitalWrite(displayCLK, LOW);
    digitalWrite(displayDIN, data & mask);
    digitalWrite(displayCLK, HIGH);
  }
}

void displayWrite(byte address, byte data) {
  digitalWrite(displayCS, LOW);
  displayWriteByte(address);
  displayWriteByte(data);
  digitalWrite(displayCS, HIGH);
}
