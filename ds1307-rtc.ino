#include <Wire.h>

const int DS3231_ADDRESS = 0x68;
const int DS3231_REG_TIME = 0x00;
struct DateTime {
  unsigned int year;
  byte month;
  byte date;
  byte hour;
  byte minute;
  byte second;
};

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  delay(1000);
  printFormattedDateTime(getDateTime());
}

unsigned int bcd2dec(byte bcd) {
  return (bcd >> 4) * 10 + (bcd & 0b00001111);
}

byte printZeroPadded(byte number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

DateTime getDateTime() {
  struct DateTime dateTime;
  
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_REG_TIME);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 7);
  while (!Wire.available()) {};

  dateTime.second = bcd2dec(Wire.read());
  dateTime.minute = bcd2dec(Wire.read());
  dateTime.hour = bcd2dec(Wire.read());
  Wire.read(); // Skip the day byte
  dateTime.date = bcd2dec(Wire.read());
  dateTime.month = bcd2dec(Wire.read());
  dateTime.year = bcd2dec(Wire.read());
  Wire.endTransmission();

  return dateTime;
}

void printFormattedDateTime(DateTime dateTime) {
  Serial.print(dateTime.year);
  Serial.print("/");
  printZeroPadded(dateTime.month);
  Serial.print("/");
  printZeroPadded(dateTime.date);
  Serial.print(" ");
  printZeroPadded(dateTime.hour);
  Serial.print(":");
  printZeroPadded(dateTime.minute);
  Serial.print(":");
  printZeroPadded(dateTime.second);
  Serial.println("");
}
