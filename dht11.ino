/**
 * Reads data from humidity & temperature sensor DHT11 without 3rd party libraries.
 *
 * DHT11 (Data) ------- (DIGITAL PIN 2)
 *                |---- (DIGITAL PIN 3)
 */

const byte interruptPinA = 2;
const byte interruptPinB = 3;

volatile unsigned long highToLow = 0;
volatile unsigned long lowToHigh = 0;

const byte dataPacketOffset = 1;
const byte dataPacketSize = 40;
const byte dataBitDurationThreshold = 30;

volatile byte highDurations[dataPacketOffset + dataPacketSize] = {};
volatile byte highDurationsIndex = 0;

struct Data {
  byte humidityInteger;
  byte humidityDecimal;
  byte temperatureInteger;
  byte temperatureDecimal;
  bool isChecksumOK;
};

void setup() {
  Serial.begin(9600);
}

void loop() {
  init();
  requestData();
  listenForData(); 

  delay(1000);

  byte *dataPointer = highDurations + dataPacketOffset;
  // dump(dataPointer, dataPacketSize);
  struct Data data = parseData(dataPointer);

  Serial.print("Humidity: ");
  Serial.print(data.humidityInteger);
  Serial.print(".");
  Serial.print(data.humidityDecimal);

  Serial.print(", Temperature: ");
  Serial.print(data.temperatureInteger);
  Serial.print(".");
  Serial.print(data.temperatureDecimal);

  Serial.print(", Checksum: ");
  Serial.println(data.isChecksumOK ? "OK" : "ERROR");
}

void init() {
  detachInterrupt(digitalPinToInterrupt(interruptPinA));
  detachInterrupt(digitalPinToInterrupt(interruptPinB));

  // Reset the high signal durations pointer
  highDurationsIndex = highToLow = lowToHigh = 0;

  // Set up initial condition - bus is high
  pinMode(interruptPinA, OUTPUT);
  pinMode(interruptPinB, INPUT_PULLUP);
  digitalWrite(interruptPinA, HIGH);
  delay(250);
}

void requestData() {
  // Pull the bus down for >18ms to request data from the sensor
  digitalWrite(interruptPinA, LOW);
  delay(20);
}

void listenForData() {
  // Get ready to receive data from the sensor
  pinMode(interruptPinA, INPUT_PULLUP);
  pinMode(interruptPinB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(interruptPinA), onFall, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPinB), onRise, RISING);
}

void onFall() {
  highToLow = micros();

  if (lowToHigh > 0) {
    highDurations[highDurationsIndex++] = highToLow - lowToHigh;
  }
}

void onRise() {
  lowToHigh = micros();
}

byte durationsToByte(byte *durations) {
  byte acc = 0;

  for (int i=0; i<8; i++) {
    byte value = durations[i] > dataBitDurationThreshold ? 1 : 0;
    acc |= value << (7 - i);
  }

  return acc;
}

Data parseData(byte *ptr) {
  struct Data data;
  
  data.humidityInteger = durationsToByte(ptr);
  ptr += 8;
  
  data.humidityDecimal = durationsToByte(ptr);
  ptr += 8;
  
  data.temperatureInteger = durationsToByte(ptr);
  ptr += 8;

  data.temperatureDecimal = durationsToByte(ptr);
  ptr += 8;

  byte checksumRemote = durationsToByte(ptr);
  byte checksumLocal = data.humidityInteger
    + data.humidityDecimal
    + data.temperatureInteger
    + data.temperatureDecimal;
  data.isChecksumOK = checksumRemote == checksumLocal;

  return data;
}

void dump(byte *data, byte dataLength) {
  for (int i = 0; i < dataLength; i++) {
    if (i % 8 == 0) Serial.println("");
    else Serial.print(" ");

    Serial.print(data[i]);
  }

  Serial.println("");
}
