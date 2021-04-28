#include "carloop.h"
#include "obd.h"
#include "ble.h"

SYSTEM_MODE(SEMI_AUTOMATIC); 
SYSTEM_THREAD(ENABLED);

// Loop Function
void sendDummy();

// Utils
void printValues();
bool byteArray8Equal(uint8_t a1[8], uint8_t a2[8]);

int canMessageCount = 0;
uint8_t pidIndex = 0;
uint8_t lastMessageData[CHARACTERISTIC1_MAX_LEN];

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
  Serial.print("Disconnected: ");
  Serial.println(handle, HEX);
}

uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size) {   
  uint8_t characteristic_len = 0;
  Serial.print("Read value handler: ");
  Serial.println(value_handle, HEX);
  // TODO
  return characteristic_len;
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  Serial.print("Write value handler: ");
  Serial.println(value_handle, HEX);
  // TODO
  return 0;
}

static void characteristic1_notify(btstack_timer_source_t *ts) {
  memcpy(characteristic1_data, lastMessageData, CHARACTERISTIC1_MAX_LEN);
  ble.sendNotify(character1_handle, characteristic1_data, CHARACTERISTIC1_MAX_LEN);

  Serial.print("NOTIFICATION: ");
  int i = 0;
  for(i = CHARACTERISTIC1_MAX_LEN-1; i >= 0; i--){
    Serial.printf("%02x ", characteristic1_data[i]);
  }
  Serial.println("");
}

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.println("BLE peripheral start.");
  
  ble.init();
  ble.onConnectedCallback(deviceConnectedCallback);
  ble.onDisconnectedCallback(deviceDisconnectedCallback);
  ble.onDataReadCallback(gattReadCallback);
  ble.onDataWriteCallback(gattWriteCallback);
  
  ble.addService(BLE_UUID_GAP);
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_DEVICE_NAME, ATT_PROPERTY_READ|ATT_PROPERTY_WRITE, (uint8_t*)BLE_DEVICE_NAME, sizeof(BLE_DEVICE_NAME));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_APPEARANCE, ATT_PROPERTY_READ, appearance, sizeof(appearance));
  ble.addCharacteristic(BLE_UUID_GAP_CHARACTERISTIC_PPCP, ATT_PROPERTY_READ, conn_param, sizeof(conn_param));

  ble.addService(BLE_UUID_GATT);
  ble.addCharacteristic(BLE_UUID_GATT_CHARACTERISTIC_SERVICE_CHANGED, ATT_PROPERTY_INDICATE, change, sizeof(change));

  ble.addService(service1_uuid);
  character1_handle = ble.addCharacteristicDynamic(char1_uuid, ATT_PROPERTY_NOTIFY, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
  
  ble.setAdvertisementParams(&adv_params);
  ble.setAdvertisementData(sizeof(adv_data), adv_data);
  ble.setScanResponseData(sizeof(scan_response), scan_response);
  ble.startAdvertising();
  Serial.println("BLE start advertising.");
  
  characteristic1.process = &characteristic1_notify;
}

void loop() {
  printValues();
  sendDummy();
}

void sendDummy(){
  // CAN ID
  memcpy(&lastMessageData[8], &OBD_CAN_REPLY_ID_MIN, 3);
  // Bytes
  lastMessageData[7] = 0x01;
  // Mode
  lastMessageData[7] = 0x41;
  // PID
  lastMessageData[5] = pidsToRequest[pidIndex];
  pidIndex++;
  if (pidIndex >= NUM_PIDS_TO_REQUEST)
     pidIndex = 0;
  // Data
  lastMessageData[4]++;
  ble.setTimer(&characteristic1, 1);
  ble.addTimer(&characteristic1);
  canMessageCount++;
  delay(1000);
}

void printValues() {
  static const unsigned long interval = 10000;
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay < interval) {
    return;
  }
  lastDisplay = millis();
  Serial.printf("CAN messages: %d ", canMessageCount);
  Serial.println("");
}
