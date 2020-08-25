#include "carloop.h"
#include "TinyGPS++.h"
#include "obd.h"
#include "ble.h"

#if defined(ARDUINO) 
SYSTEM_MODE(SEMI_AUTOMATIC); 
#endif

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

uint32_t ble_advdata_decode(uint8_t type, uint8_t advdata_len, uint8_t *p_advdata, uint8_t *len, uint8_t *p_field_data) {
  uint8_t index = 0;
  uint8_t field_length, field_type;

  while (index < advdata_len) {
    field_length = p_advdata[index];
    field_type = p_advdata[index + 1];
    if (field_type == type) {
      memcpy(p_field_data, &p_advdata[index + 2], (field_length - 1));
      *len = field_length - 1;
      return 0;
    }
    index += field_length + 1;
  }
  return 1;
}

void reportCallback(advertisementReport_t *report) {
  uint8_t index;

  Serial.println("reportCallback: ");
  Serial.print("The advEventType: ");
  Serial.println(report->advEventType, HEX);
  Serial.print("The peerAddrType: ");
  Serial.println(report->peerAddrType, HEX);
  Serial.print("The peerAddr: ");
  for (index = 0; index < 6; index++) {
    Serial.print(report->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("The rssi: ");
  Serial.println(report->rssi, DEC);

  if (report->advEventType == BLE_GAP_ADV_TYPE_SCAN_RSP) {
    Serial.print("The scan response data: ");
  }
  else {
    Serial.print("The advertising data: ");
  }
  for (index = 0; index < report->advDataLen; index++) {
    Serial.print(report->advData[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.println(" ");

  uint8_t len;
  uint8_t adv_name[31];
  
  if (0x00 == ble_advdata_decode(0x08, report->advDataLen, report->advData, &len, adv_name)) {
    Serial.print("  The length of Short Local Name : ");
    Serial.println(len, HEX);
    Serial.print("  The Short Local Name is        : ");
    Serial.println((const char *)adv_name);
    if (0x00 == memcmp(adv_name, "Biscuit", min(7, len))) {
      ble.stopScanning();
      device.addr_type = report->peerAddrType;
      memcpy(device.addr, report->peerAddr, 6);

      ble.connect(report->peerAddr, report->peerAddrType);
    }
  }
  else if (0x00 == ble_advdata_decode(0x09, report->advDataLen, report->advData, &len, adv_name)) {
    Serial.print("  The length of Complete Local Name : ");
    Serial.println(len, HEX);
    Serial.print("  The Complete Local Name is        : ");
    Serial.println((const char *)adv_name);
    if (0x00 == memcmp(adv_name, "Heart Rate", min(7, len))) {

    }
  }
}

void deviceConnectedCallback(BLEStatus_t status, uint16_t handle) {
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Device connected!");
      connected_id = handle;
      device.connected_handle = handle;
      ble.discoverPrimaryServices(handle);
      break;
    default: break;
  }
}

void deviceDisconnectedCallback(uint16_t handle){
  Serial.print("Disconnected handle:");
  Serial.println(handle,HEX);
  if (connected_id == handle) {
    Serial.println("Restart scanning.");
    // Disconnect from remote device, restart to scanning.
    connected_id = 0xFFFF;
    ble.startScanning();
  }
}

static void discoveredServiceCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_service_t *service) {
  uint8_t index;
  if (status == BLE_STATUS_OK) {   // Found a service.
    Serial.println(" ");
    Serial.print("Service start handle: ");
    Serial.println(service->start_group_handle, HEX);
    Serial.print("Service end handle: ");
    Serial.println(service->end_group_handle, HEX);
    Serial.print("Service uuid16: ");
    Serial.println(service->uuid16, HEX);
    Serial.print("The uuid128 : ");
    for (index = 0; index < 16; index++) {
      Serial.print(service->uuid128[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    if (0x00 == memcmp(service->uuid128, service1_uuid, 16)) {
      Serial.println("Target uuid128");
      device.service.service = *service;
    }
  }
  else if (status == BLE_STATUS_DONE) {
    Serial.println("Discovered service done");
    ble.discoverCharacteristics(device.connected_handle, &device.service.service);
  }
}

static void discoveredCharsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_t *characteristic) {
  uint8_t index;
  if (status == BLE_STATUS_OK) {   // Found a characteristic.
    Serial.println(" ");
    Serial.print("characteristic start handle: ");
    Serial.println(characteristic->start_handle, HEX);
    Serial.print("characteristic value handle: ");
    Serial.println(characteristic->value_handle, HEX);
    Serial.print("characteristic end_handle: ");
    Serial.println(characteristic->end_handle, HEX);
    Serial.print("characteristic properties: ");
    Serial.println(characteristic->properties, HEX);
    Serial.print("characteristic uuid16: ");
    Serial.println(characteristic->uuid16, HEX);
    Serial.print("characteristic uuid128 : ");
    for (index = 0; index < 16; index++) {
      Serial.print(characteristic->uuid128[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    if (chars_index < 2) { 
      device.service.chars[chars_index].chars= *characteristic;
      chars_index++;
    }
  }
  else if (status == BLE_STATUS_DONE) {
    Serial.println("Discovered characteristic done");
    chars_index = 0;
    ble.discoverCharacteristicDescriptors(device.connected_handle, &device.service.chars[chars_index].chars);
  }
}

static void discoveredCharsDescriptorsCallback(BLEStatus_t status, uint16_t con_handle, gatt_client_characteristic_descriptor_t *descriptor) {
  uint8_t index;
  if (status == BLE_STATUS_OK) {   // Found a descriptor.
    Serial.println(" ");
    Serial.print("descriptor handle: ");
    Serial.println(descriptor->handle, HEX);
    Serial.print("descriptor uuid16: ");
    Serial.println(descriptor->uuid16, HEX);
    Serial.print("descriptor uuid128 : ");
    for (index = 0; index < 16; index++) {
      Serial.print(descriptor->uuid128[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    if (desc_index < 2) {
      device.service.chars[chars_index].descriptor[desc_index++] = *descriptor;
    }
  }
  else if (status == BLE_STATUS_DONE) {
    Serial.println("Discovered descriptor done");
    chars_index++;
    if (chars_index < 2) {
      desc_index=0;
      ble.discoverCharacteristicDescriptors(device.connected_handle, &device.service.chars[chars_index].chars);
    }
    else {
      ble.readValue(device.connected_handle,&device.service.chars[1].chars);
    }
  }
}

void gattReadCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length) {
  uint8_t index;
  if (status == BLE_STATUS_OK) {
    Serial.println(" ");
    Serial.println("Read characteristic ok");
    Serial.print("conn handle: ");
    Serial.println(con_handle, HEX);
    Serial.print("value handle: ");
    Serial.println(value_handle, HEX);
        
    Serial.print("The value : ");
    for (index = 0; index < length; index++) {
      Serial.print(value[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
  else if (status == BLE_STATUS_DONE) {
    uint8_t data[]= {0x01,0x02,0x03,0x04,0x05,1,2,3,4,5};
    ble.writeValue(device.connected_handle, device.service.chars[0].chars.value_handle, sizeof(data), data);
  }
}

void gattWrittenCallback(BLEStatus_t status, uint16_t con_handle) {
  if (status == BLE_STATUS_DONE) {
    Serial.println(" ");
    Serial.println("Write characteristic done");
    ble.readDescriptorValue(device.connected_handle, device.service.chars[0].descriptor[0].handle);
  }
}

void gattReadDescriptorCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length) {
  uint8_t index;
  if(status == BLE_STATUS_OK) {
    Serial.println(" ");
    Serial.println("Read descriptor ok");
    Serial.print("conn handle: ");
    Serial.println(con_handle, HEX);
    Serial.print("value handle: ");
    Serial.println(value_handle, HEX);
    Serial.print("The value : ");
    for (index = 0; index < length; index++) {
      Serial.print(value[index], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
  else if (status == BLE_STATUS_DONE) {
    ble.writeClientCharsConfigDescriptor(device.connected_handle, &device.service.chars[0].chars, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
  }
}

void gattWriteCCCDCallback(BLEStatus_t status, uint16_t con_handle) {
  if (status == BLE_STATUS_DONE) {
    Serial.println("gattWriteCCCDCallback done");
    if (gatt_notify_flag == 0) { 
      gatt_notify_flag = 1;
      ble.writeClientCharsConfigDescriptor(device.connected_handle, &device.service.chars[1].chars, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
    }
    else if (gatt_notify_flag == 1) {
      gatt_notify_flag = 2;
    }
  }
}

void gattNotifyUpdateCallback(BLEStatus_t status, uint16_t con_handle, uint16_t value_handle, uint8_t *value, uint16_t length) {
  uint8_t index;
  Serial.println(" ");
  Serial.println("Notify Update value ");
  Serial.print("conn handle: ");
  Serial.println(con_handle, HEX);
  Serial.print("value handle: ");
  Serial.println(value_handle, HEX);
  Serial.print("The value : ");
  for (index = 0; index < length; index++) {
    Serial.print(value[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
}

void setup() {
  Serial.begin(115200);
  delay(5000);
    
  Serial.println("BLE central demo!");
  // Initialize ble_stack.
  ble.init();
    
  // Register callback functions.
  ble.onConnectedCallback(deviceConnectedCallback);
  ble.onDisconnectedCallback(deviceDisconnectedCallback);
  ble.onScanReportCallback(reportCallback);

  ble.onServiceDiscoveredCallback(discoveredServiceCallback);
  ble.onCharacteristicDiscoveredCallback(discoveredCharsCallback);
  ble.onDescriptorDiscoveredCallback(discoveredCharsDescriptorsCallback);
  ble.onGattCharacteristicReadCallback(gattReadCallback);
  ble.onGattCharacteristicWrittenCallback(gattWrittenCallback);
  ble.onGattDescriptorReadCallback(gattReadDescriptorCallback);

  ble.onGattWriteClientCharacteristicConfigCallback(gattWriteCCCDCallback);
  ble.onGattNotifyUpdateCallback(gattNotifyUpdateCallback);

  // Set scan parameters.
  ble.setScanParams(BLE_SCAN_TYPE, BLE_SCAN_INTERVAL, BLE_SCAN_WINDOW);
  
  // Start scanning.
  ble.startScanning();
  Serial.println("Start scanning ");
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
      // Serial.print("Sending: ");
      // Serial.println(messageToString(message));
      memcpy(lastMessageData, message.data, 8);
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
