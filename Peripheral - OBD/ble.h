#define MIN_CONN_INTERVAL          0x0028 // 50ms.
#define MAX_CONN_INTERVAL          0x0190 // 500ms.
#define SLAVE_LATENCY              0x0000 // No slave latency.
#define CONN_SUPERVISION_TIMEOUT   0x03E8 // 10s.

#define BLE_PERIPHERAL_APPEARANCE  BLE_APPEARANCE_UNKNOWN
#define BLE_DEVICE_NAME            "BLE_Peripheral"

// Length of characteristic value.
#define CHARACTERISTIC1_MAX_LEN    8
#define CHARACTERISTIC2_MAX_LEN    8

// Primary service 128-bits UUID
static uint8_t service1_uuid[16] = { 0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
// Characteristics 128-bits UUID
static uint8_t char1_uuid[16]    = { 0x71,0x3d,0x00,0x02,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };

// Primary service 128-bits UUID
static uint8_t service2_uuid[16] = { 0x71,0x3d,0x00,0x00,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };
// Characteristics 128-bits UUID
static uint8_t char2_uuid[16]    = { 0x71,0x3d,0x00,0x02,0x51,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e };

// GAP and GATT characteristics value
static uint8_t  appearance[2] = {LOW_BYTE(BLE_PERIPHERAL_APPEARANCE), HIGH_BYTE(BLE_PERIPHERAL_APPEARANCE)};

static uint8_t  change[4] = {0x00, 0x00, 0xFF, 0xFF};

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
  0x02, BLE_GAP_AD_TYPE_FLAGS, BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE, 
  0x11, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE, 0x1e, 0x94, 0x8d, 0xf1, 0x48, 0x31, 0x94, 0xba, 0x75, 0x4c, 0x3e, 0x50, 0x00, 0x00, 0x3d, 0x71
};

// BLE peripheral scan respond data
static uint8_t scan_response[] = {0x08, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, 'R', 'B',  'L', '-', 'D', 'U', 'O'};

// Characteristic value handle
static uint16_t character1_handle = 0x0000;
static uint16_t character2_handle = 0x0000;

// Buffer of characteristic value.
static uint8_t characteristic1_data[CHARACTERISTIC1_MAX_LEN] = { 0x00 };
static uint8_t characteristic2_data[CHARACTERISTIC2_MAX_LEN] = { 0x01 };

// Timer task.
static btstack_timer_source_t characteristic1;
