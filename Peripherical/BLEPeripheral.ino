#include <carloop.h>
#include <TinyGPS++.h>

#if defined(ARDUINO) 
SYSTEM_MODE(SEMI_AUTOMATIC); 
#endif

#define MIN_CONN_INTERVAL          0x0028 // 50ms.
#define MAX_CONN_INTERVAL          0x0190 // 500ms.
#define SLAVE_LATENCY              0x0000 // No slave latency.
#define CONN_SUPERVISION_TIMEOUT   0x03E8 // 10s.

#define BLE_PERIPHERAL_APPEARANCE  BLE_APPEARANCE_UNKNOWN
#define BLE_DEVICE_NAME            "BLE_Peripheral"

// Length of characteristic value.
#define CHARACTERISTIC1_MAX_LEN    8
#define CHARACTERISTIC2_MAX_LEN    8
#define CHARACTERISTIC3_MAX_LEN    8
#define CHARACTERISTIC4_MAX_LEN    8

void sendObdRequest();
void waitForObdResponse();
void delayUntilNextRequest();

Carloop<CarloopRevision2> carloop;

// Primary service 128-bits UUID
static uint8_t service1_uuid[16] = { 0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
// Characteristics 128-bits UUID
static uint8_t char1_uuid[16]    = { 0x71,0x3d,0x00,0x02,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
static uint8_t char2_uuid[16]    = { 0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };

// Primary service 128-bits UUID
static uint8_t service2_uuid[16] = { 0x71,0x3d,0x00,0x00,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
// Characteristics 128-bits UUID
static uint8_t char3_uuid[16]    = { 0x71,0x3d,0x00,0x02,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
static uint8_t char4_uuid[16]    = { 0x71,0x3d,0x00,0x03,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };

// GAP and GATT characteristics value
static uint8_t  appearance[2] = { 
  LOW_BYTE(BLE_PERIPHERAL_APPEARANCE), 
  HIGH_BYTE(BLE_PERIPHERAL_APPEARANCE) 
};

static uint8_t  change[4] = {
  0x00, 0x00, 0xFF, 0xFF
};

static uint8_t  conn_param[8] = {
  LOW_BYTE(MIN_CONN_INTERVAL), HIGH_BYTE(MIN_CONN_INTERVAL), 
  LOW_BYTE(MAX_CONN_INTERVAL), HIGH_BYTE(MAX_CONN_INTERVAL), 
  LOW_BYTE(SLAVE_LATENCY), HIGH_BYTE(SLAVE_LATENCY), 
  LOW_BYTE(CONN_SUPERVISION_TIMEOUT), HIGH_BYTE(CONN_SUPERVISION_TIMEOUT)
};

static advParams_t adv_params = {
  .adv_int_min   = 0x0030,
  .adv_int_max   = 0x0030,
  .adv_type      = BLE_GAP_ADV_TYPE_ADV_IND,
  .dir_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC,
  .dir_addr      = {0,0,0,0,0,0},
  .channel_map   = BLE_GAP_ADV_CHANNEL_MAP_ALL,
  .filter_policy = BLE_GAP_ADV_FP_ANY
};

// BLE peripheral advertising data
static uint8_t adv_data[] = {
  0x02,
  BLE_GAP_AD_TYPE_FLAGS,
  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,   
  
  0x11,
  BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE,
  0x1e, 0x94, 0x8d, 0xf1, 0x48, 0x31, 0x94, 0xba, 0x75, 0x4c, 0x3e, 0x50, 0x00, 0x00, 0x3d, 0x71 
};

// BLE peripheral scan respond data
static uint8_t scan_response[] = {
  0x08,
  BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
  'R', 'B',  'L', '-', 'D', 'U', 'O'
};

// Characteristic value handle
static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;
static uint16_t character3_handle = 0x0000;
static uint16_t character4_handle = 0x0000;

// Buffer of characterisitc value.
static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN] = { 0x01 };
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN] = { 0x00 };
static uint8_t characteristic3_data[CHARACTERISTIC2_MAX_LEN] = { 0x03 };
static uint8_t characteristic4_data[CHARACTERISTIC2_MAX_LEN] = { 0x04 };

// Timer task.
static btstack_timer_source_t characteristic2;

// OBD CAN Message IDs
const auto OBD_REQUEST_ID = 0x7E0;
const auto OBD_REPLY_ID = 0x7E8;
const auto OBD_PID_SERVICE = 0x01;

// OBD PID constants
const auto ENGINE_COOLANT_TEMP = 0x05;
const auto ENGINE_RPM = 0x0C;
const auto VEHICLE_SPEED = 0x0D;
const auto MAF_SENSOR = 0x10;
const auto O2_VOLTAGE = 0x14;
const auto THROTTLE = 0x11;

const uint8_t pid = ENGINE_RPM;
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
    Serial.println("Character2 read:");
    memcpy(buffer, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
    characteristic_len = CHARACTERISTIC2_MAX_LEN;
  }
  else if (character3_handle == value_handle) {
    Serial.println("Character3 read:");
    memcpy(buffer, characteristic3_data, CHARACTERISTIC3_MAX_LEN);
    characteristic_len = CHARACTERISTIC3_MAX_LEN;
  }
  else if (character4_handle == value_handle) {
    Serial.println("Character4 read:");
    memcpy(buffer, characteristic4_data, CHARACTERISTIC4_MAX_LEN);
    characteristic_len = CHARACTERISTIC4_MAX_LEN;
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
  // Serial.println("characteristic2_notify");
  characteristic2_data[CHARACTERISTIC2_MAX_LEN-1]++;
  ble.sendNotify(character2_handle, characteristic2_data, CHARACTERISTIC2_MAX_LEN);
  
  ble.setTimer(ts, 10000);
  ble.addTimer(ts);
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
  
  // Add primary sevice2.
  ble.addService(service2_uuid);
  character3_handle = ble.addCharacteristic(char3_uuid, ATT_PROPERTY_READ, characteristic3_data, CHARACTERISTIC3_MAX_LEN);
  character4_handle = ble.addCharacteristic(char4_uuid, ATT_PROPERTY_READ, characteristic4_data, CHARACTERISTIC4_MAX_LEN);

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
  ble.setTimer(&characteristic2, 10000);
  ble.addTimer(&characteristic2);

  carloop.begin();
  Serial.println("Carloop Initialized");
  transitionTime = millis();
}

void loop() {
    carloop.update();
    obdLoopFunction();
}

void sendObdRequest(){
  CANMessage message;
  message.id = OBD_REQUEST_ID;
  message.len = 8;
  message.data[0] = 0x02;
  message.data[1] = OBD_PID_SERVICE;
  message.data[2] = pid;
  carloop.can().transmit(message);
  obdLoopFunction = waitForObdResponse;
  transitionTime = millis();
}

void waitForObdResponse(){
  if(millis() - transitionTime >= 100){
    obdLoopFunction = delayUntilNextRequest;
    transitionTime = millis();
    return;
  }
  bool responseReceived = false;
  CANMessage message;
  while(carloop.can().receive(message)){
    if(message.id == OBD_REPLY_ID && message.data[2] == pid){
      responseReceived = true;
    }
  }
  if(responseReceived){
    Serial.println("Response Received");
  }
}

void delayUntilNextRequest(){
  if(millis() - transitionTime >= 400){
    obdLoopFunction = sendObdRequest;
    transitionTime = millis();
  }
}
