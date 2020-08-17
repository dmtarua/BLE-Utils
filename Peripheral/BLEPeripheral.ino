#include "carloop.h"
#include "TinyGPS++.h"
#include "obd.h"
#include "ble.h"

SYSTEM_MODE(SEMI_AUTOMATIC); 
SYSTEM_THREAD(ENABLED);

// Loop Function
void sendObdRequest();
void waitForObdResponse();
void delayUntilNextRequest();

// Utils
void printValuesAtInterval();
void printValues();
bool byteArray8Equal(uint8_t a1[8], uint8_t a2[8]);
String messageToString(const CANMessage &message);

Carloop<CarloopRevision2> carloop;
int canMessageCount = 0;
uint8_t pidIndex = NUM_PIDS_TO_REQUEST - 1;
uint8_t lastMessageData[8];
auto *obdLoopFunction = sendObdRequest;
unsigned long transitionTime = 0;

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Device Connected!");
      break;
    case BLE_STATUS_CONNECTION_ERROR:
      Serial.println("Connection Error!");
      break;
    default: break;
  }
}

void deviceDisconnectedCallback(uint16_t handle) {
  Serial.println("Disconnected.");
}

uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size) {   
  uint8_t characteristic_len = 0;
  Serial.print("Read value handler: ");
  Serial.println(value_handle, HEX);

  if (character2_handle == value_handle) {
    Serial.println("Character2 read.");
    memcpy(buffer, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    characteristic_len = CHARACTERISTIC2_MAX_LEN;
  }
  return characteristic_len;
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  Serial.print("Write value handler: ");
  Serial.println(value_handle, HEX);

  if (character1_handle == value_handle) {
    memcpy(characteristic1_data, buffer, size);
    Serial.print("Characteristic1 write value: ");
    for (uint8_t index = 0; index < size; index++) {
      Serial.print(characteristic1_data[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
  return 0;
}

static void characteristic2_notify(btstack_timer_source_t *ts) {
  memcpy(characteristic2_data, lastMessageData, 8);
  ble.sendNotify(character2_handle, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("BLE peripheral demo.");

  // Initialize ble_stack.
  ble.init();

  // Register BLE callback functions.
  ble.onConnectedCallback(deviceConnectedCallback);
  ble.onDisconnectedCallback(deviceDisconnectedCallback);
  ble.onDataReadCallback(gattReadCallback);
  ble.onDataWriteCallback(gattWriteCallback);

  // Add GAP service and characteristics
  ble.addService(BLE_UUID_GAP);
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE, (uint8_t*)BLE_DEVICE_NAME, sizeof(BLE_DEVICE_NAME));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_APPEARANCE, ATT_PROPERTY_READ, appearance, sizeof(appearance));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_PPCP, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));

  // Add GATT service and characteristics
  ble.addService(BLE_UUID_GATT);
  ble.addCharacteristic(BLE_UUID_GATT_CHARACTERISTIC_SERVICE_CHANGED, ATT_PROPERTY_INDICATE, change, sizeof(change));

  // Add primary service1.
  ble.addService(service1_uuid);
  // Add characteristic to service1, return value handle of characteristic.
  character1_handle = ble.addCharacteristicDynamic(char1_uuid, ATT_PROPERTY_WRITE_WITHOUT_RESPONSE, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
  character2_handle = ble.addCharacteristicDynamic(char2_uuid, ATT_PROPERTY_READ|ATT_PROPERTY_NOTIFY, characteristic2_data, CHARACTERISTIC2_MAX_LEN);

  // Set BLE advertising parameters
  ble.setAdvertisementParams(&adv_params);

  // Set BLE advertising and scan respond data
  ble.setAdvertisementData(sizeof(adv_data), adv_data);
  ble.setScanResponseData(sizeof(scan_response), scan_response);
  
  // Start advertising.
  ble.startAdvertising();
  Serial.println("BLE start advertising.");
    
  // set one-shot timer
  characteristic2.process = &characteristic2_notify;

  carloop.begin();
  transitionTime = millis();
}

void loop() {
    carloop.update();
    printValuesAtInterval();
    obdLoopFunction();
}

// OBD Loop Functions
void sendObdRequest() {
  pidIndex = (pidIndex + 1) % NUM_PIDS_TO_REQUEST;
  CANMessage message;
  message.id = OBD_CAN_BROADCAST_ID;
  message.len = 8;
  message.data[0] = 0x02;
  message.data[1] = OBD_MODE_CURRENT_DATA; 
  message.data[2] = pidsToRequest[pidIndex];
  carloop.can().transmit(message);
  obdLoopFunction = waitForObdResponse;
  transitionTime = millis();
}

void waitForObdResponse() {
  if (millis() - transitionTime >= 100) {
    obdLoopFunction = delayUntilNextRequest;
    transitionTime = millis();
    return;
  }
  CANMessage message;
  while (carloop.can().receive(message)) {
    canMessageCount++;
    if (!byteArray8Equal(message.data, lastMessageData)) {
      Serial.print("Sending: ");
      Serial.println(messageToString(message));
      memcpy(lastMessageData, message.data, 8);
      ble.setTimer(&characteristic2, 1);
      ble.addTimer(&characteristic2);
    }
  }
}

void delayUntilNextRequest() {
  if (millis() - transitionTime >= 80) {
    obdLoopFunction = sendObdRequest;
    transitionTime = millis();
  }
}

void printValuesAtInterval() {
  static const unsigned long interval = 10000;
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay < interval) {
    return;
  }
  lastDisplay = millis();
  printValues();
}

void printValues() {
  Serial.printf("Battery voltage: %12f ", carloop.battery());
  Serial.printf("CAN messages: %12d ", canMessageCount);
  Serial.println("");
}

String messageToString(const CANMessage &message) {
  String str = "";
  for (int i = 0; i <= (message.len - 1); i++) {
    str += String::format("%02x", message.data[i]);
  }
  return str;
}

bool byteArray8Equal(uint8_t a1[8], uint8_t a2[8]) {
  for (int i = 0; i < 8; i++) {
    if (a1[i] != a2[i]) return false;
  }
  return true;
}
