#include <ArduinoBLE.h>
#include <ESP32Time.h>

ESP32Time rtc;
//ESP32Time rtc(3600);  // offset in seconds GMT+1

BLEService dateService("1ce76320-2d32-41af-b4c4-46836ea7a62a"); // Bluetooth® Low Energy LED Service
BLECharacteristic dateCharacteristic("ad804469-19ec-406a-b949-31ae17e43813", BLERead | BLENotify | BLEWrite, 8);
BLECharacteristic lightCharacteristic("947aad02-c25d-11ed-afa1-0242ac120002", BLERead | BLENotify , 3);


uint8_t ble_value = 0x0;
int analogValue = 500;
int minValue = analogValue; 
int maxValue = analogValue; 

void setup() {
  Serial.begin(9600);
  rtc.setTime(1609459200);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("COOP-DOOR");
  BLE.setAdvertisedService(dateService);
  dateService.addCharacteristic(dateCharacteristic);
  dateService.addCharacteristic(lightCharacteristic);
  BLE.addService(dateService);
  dateCharacteristic.writeValue(0);
  lightCharacteristic.writeValue(0);


  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      delay(500);

      manageDate();
      manageLight();


    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }





}


void manageLight(){
  analogValue = random(10, 1000);
  minValue = min(analogValue, minValue);
  maxValue = max(analogValue, maxValue);
  Serial.print(" value ");
  Serial.print(analogValue);
  Serial.print(" minValue ");
  Serial.print(minValue);
  Serial.print(" maxValue ");
  Serial.print(maxValue);
  Serial.println("");
  float divider = 1000 / 255.0; //1000 max allowed value, 255 max byte value
  uint8_t currentValue = analogValue / divider;
  uint8_t scaledMinValue = minValue / divider;
  uint8_t scaledMaxValue = maxValue / divider;

  uint8_t ble_value_array[5] = {currentValue, scaledMinValue, scaledMaxValue };

  // Write the array to the characteristic
  lightCharacteristic.writeValue(ble_value_array, 5);

  
}





void manageDate() {
  if (dateCharacteristic.written()) {
    Serial.println("Date update");
    long xx = getLongFromBytes(dateCharacteristic.value());
    Serial.println(xx);
    rtc.setTime(xx);
    Serial.println(rtc.getEpoch());


  } else {
    Serial.println(rtc.getEpoch());
    byte* tmpDate = getBytesFromLong(rtc.getEpoch());
    for (int i = 0; i < 8; i++) {
      Serial.print(tmpDate[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    dateCharacteristic.writeValue(tmpDate, 8);
    delete[] tmpDate;
  }

}


byte* getBytesFromLong(long x  ) {
  byte* bytes = new byte[8];
  for (int i = 0; i < 8; i++) {
    bytes[i] = x & 0xff;
    x = (x - bytes[i]) / 256;
  }
  return bytes;

}

long getLongFromBytes(const byte* bytes) {
  long result = 0;
  for (int i = 7; i >= 0; i--) {
    result = (result * 256) + bytes[i];
  }
  return result;
}
