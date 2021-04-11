#include "carloop.h"
#include "ble.h"

#define CAN_BAUDRATE    500000  ///< CAN baudrate (250000, 500000)

SYSTEM_MODE(SEMI_AUTOMATIC); 
SYSTEM_THREAD(ENABLED);

// Loop Function
void sniff();

// Utils
void printValues();
String messageToString(const CANMessage &message);

Carloop<CarloopRevision2> carloop;
int canMessageCount = 0;
uint8_t lastMessageData[8];

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

  if (character1_handle == value_handle) {
    Serial.println("Character1 read.");
    memcpy(buffer, characteristic1_data, CHARACTERISTIC1_MAX_LEN);
    characteristic_len = CHARACTERISTIC1_MAX_LEN;
  }
  
  else if (character2_handle == value_handle) {
    Serial.println("Character2 read.");
    memcpy(buffer, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    characteristic_len = CHARACTERISTIC2_MAX_LEN;
  }
  return characteristic_len;
}

int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  Serial.print("Write value handler: ");
  Serial.println(value_handle, HEX);
  // TODO
  return 0;
}

static void characteristic1_notify(btstack_timer_source_t *ts) {
  memcpy(characteristic1_data, lastMessageData, 8);
  ble.sendNotify(character1_handle, characteristic1_data, CHARACTERISTIC1_MAX_LEN);

  Serial.print("NOTIFICATION: ");
  int i = 0;
  for(i = 0; i < CHARACTERISTIC1_MAX_LEN; i++){
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
  character1_handle = ble.addCharacteristicDynamic(char1_uuid, ATT_PROPERTY_READ|ATT_PROPERTY_NOTIFY, characteristic1_data, CHARACTERISTIC1_MAX_LEN);

  ble.addService(service2_uuid);
  character2_handle = ble.addCharacteristicDynamic(char2_uuid, ATT_PROPERTY_READ, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
  
  ble.setAdvertisementParams(&adv_params);
  ble.setAdvertisementData(sizeof(adv_data), adv_data);
  ble.setScanResponseData(sizeof(scan_response), scan_response);
  ble.startAdvertising();
  Serial.println("BLE start advertising.");
  
  characteristic1.process = &characteristic1_notify;

  carloop.setCANSpeed(CAN_BAUDRATE);
  carloop.begin();
}

void loop() {
    carloop.update();
    printValues();
    sniff();
    delay(1);
}

void sniff() {
  CANMessage message;
  if (carloop.can().receive(message)) {
    canMessageCount++;
    Serial.print("CAN Message: ");
    Serial.println(messageToString(message));
    memcpy(lastMessageData, message.data, 8);
    ble.setTimer(&characteristic1, 1);
    ble.addTimer(&characteristic1);
  }
}

void printValues() {
  static const unsigned long interval = 10000;
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay < interval) {
    return;
  }
  lastDisplay = millis();
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
