/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/****************************************************************************
*
* This demo showcases BLE GATT server. It can send adv data, be connected by client.
* Run the gatt_client demo, the client demo will automatically connect to the gatt_server demo.
* Client demo will enable gatt_server's notify after connection. The two devices will then exchange
* data.
*
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h" //  implements BT controller and VHCI configuration procedures from the host side.

#include "esp_gap_ble_api.h" // implements GAP configuration, such as advertising and connection parameters
#include "esp_gatts_api.h" //  implements GATT configuration, such as creating services and characteristics
#include "esp_bt_defs.h"
#include "esp_bt_main.h" // implements initialization and enabling of the Bluedroid stack
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"

#define GATTS_TAG "GATTS_DEMO"

///Declare the static function
static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

#define GATTS_SERVICE_UUID_TEST_A   0x00FF
#define GATTS_CHAR_UUID_TEST_A      0xFF01
#define GATTS_DESCR_UUID_TEST_A     0x3333
#define GATTS_NUM_HANDLE_TEST_A     4 // The number of handles is defined as 4
// The handles are:
// 1) Service handle               2) Characteristic handle
// 3) Characteristic value handle  4) Characteristic descriptor handle

#define GATTS_SERVICE_UUID_TEST_B   0x00EE
#define GATTS_CHAR_UUID_TEST_B      0xEE01
#define GATTS_DESCR_UUID_TEST_B     0x2222
#define GATTS_NUM_HANDLE_TEST_B     4

#define TEST_DEVICE_NAME            "ESP_GATTS_DEMO"
#define TEST_MANUFACTURER_DATA_LEN  17

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40 // the characteristic length is defined as:

#define PREPARE_BUF_MAX_SIZE 1024

static uint8_t char1_str[] = {0x11,0x22,0x33};
static esp_gatt_char_prop_t a_property = 0;
static esp_gatt_char_prop_t b_property = 0;

// this example gives an initial value to the characteristic
static esp_attr_value_t gatts_demo_char1_val =
{
    .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
    .attr_len     = sizeof(char1_str),
    .attr_value   = char1_str, // dummy data {0x11,0x22,0x33};
};

static uint8_t adv_config_done = 0;
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)


// It is possible to also advertise customized raw data using the esp_ble_gap_config_adv_data_raw()
// and esp_ble_gap_config_scan_rsp_data_raw() functions, which require to create and
// pass a buffer for both advertising data and scanning response data.
// In this example, the raw data is represented by the raw_adv_data[] and raw_scan_rsp_data[] arrays
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
        0x02, 0x01, 0x06,
        0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd
};
static uint8_t raw_scan_rsp_data[] = {
        0x0f, 0x09, 0x45, 0x53, 0x50, 0x5f, 0x47, 0x41, 0x54, 0x54, 0x53, 0x5f, 0x44,
        0x45, 0x4d, 0x4f
};
#else

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

// The length of adv data must be less than 31 bytes
// static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
// adv data
// The function used to configure standard Bluetooth Specification advertisement parameters
// is esp_ble_gap_config_adv_data(), which takes a pointer to an esp_ble_adv_data_t structure
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false, // Set this advertising data as scan response or not
    .include_name = true, // Advertising data include device name or not
    .include_txpower = false, // Advertising data include TX power
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00, // External appearance of device
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN, - Manufacturer data length
    .p_manufacturer_data =  NULL, //&test_manufacturer[0], - Manufacturer data point
    .service_data_len = 0, // Service data length
    .p_service_data = NULL, // Service data point
    .service_uuid_len = sizeof(adv_service_uuid128), // Service uuid length
    .p_service_uuid = adv_service_uuid128, // Service uuid array point
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT), // Advertising flag of discovery mode, see BLE_ADV_DATA_FLAG detail
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x0006,
    //.max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

#define PROFILE_NUM 2
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1

// Each profile is defined as a struct where the struct members depend on the services
// and characteristics that are implemented in that Application Profile.
struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;             // GATT interface
    uint16_t app_id;               // Application ID
    uint16_t conn_id;              // Connection ID
    uint16_t service_handle;       // Service handle
    esp_gatt_srvc_id_t service_id; // Service ID
    uint16_t char_handle;          // Characteristic handle
    esp_bt_uuid_t char_uuid;       // Characteristic UUID
    esp_gatt_perm_t perm;          // Attribute permissions
    esp_gatt_char_prop_t property; // Characteristic properties
    uint16_t descr_handle;         // Client Characteristic Configuration descriptor handle
    esp_bt_uuid_t descr_uuid;      // Client Characteristic Configuration descriptor UUID
};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
// For initialization, this parameter is set to ESP_GATT_IF_NONE, which means that
// the Application Profile is not linked to any client yet
static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
    [PROFILE_B_APP_ID] = {
        .gatts_cb = gatts_profile_b_event_handler, /* This demo does not implement, similar as profile A */
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

// To handle long characteristic write, a prepare buffer structure is defined and instantiated:
typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t a_prepare_write_env;
static prepare_type_env_t b_prepare_write_env;

void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

// GAP Event Handler
// Once the advertising data have been set, the GAP event ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT
// is triggered. For the case of raw advertising data set, the event triggered is
// ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT. Additionally when the raw scan response
// data is set, ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT event is triggered.

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
#ifdef CONFIG_SET_RAW_ADV_DATA
    // case 1
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done==0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    // case 2
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done==0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#else
    // case 3
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    // case 4
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#endif
    // case 5
    // If the advertising started successfully, an ESP_GAP_BLE_ADV_START_COMPLETE_EVT event
    // is generated, which in this example is used to check if the advertising status is indeed advertising.
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising start failed\n");
        }
        break;
    // case 6
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed\n");
        } else {
            ESP_LOGI(GATTS_TAG, "Stop adv successfully\n");
        }
        break;
    // case 7
    // The esp_ble_gap_update_conn_params() function triggers a GAP event
    // ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT, which is used to print the connection information
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(GATTS_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

// The example_write_event_env() function contains the logic for the write long characteristic procedure:
void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    esp_gatt_status_t status = ESP_GATT_OK;
    // The function then checks if the Prepare Write Request parameter represented by
    // the write.is_prep is set, which means that the client is requesting a Write Long Characteristic.
    // If present, the procedure continues with the preparation of multiple write responses,
    // if not present, then the server simply sends a single write response back.
    if (param->write.need_rsp){
        if (param->write.is_prep){
            // In order to use the prepare buffer, some memory space is allocated for it.
            // In case the allocation fails due to a lack of memory, an error is printed:
            if (prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(GATTS_TAG, "Gatt_server prep no mem\n");
                    status = ESP_GATT_NO_RESOURCES;
                }
            } else { // If the buffer is not NULL, which means initialization completed, the procedure then checks if the offset and message length of the incoming write fit the buffer.
                if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
                    status = ESP_GATT_INVALID_OFFSET;
                } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                    status = ESP_GATT_INVALID_ATTR_LEN;
                }
            }

            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            // Finally, the incoming data is copied to the buffer created and its length is incremented by the offset:
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK){
               ESP_LOGE(GATTS_TAG, "Send response error\n");
            }
            free(gatt_rsp);
            if (status != ESP_GATT_OK){
                return;
            }
            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;

        }else{
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}

// The client finishes the long write sequence by sending an Executive Write Request.
// This command triggers an ESP_GATTS_EXEC_WRITE_EVT event.
// The server handles this event by sending a response and executing the example_exec_write_event_env() function:

// The executive write is used to either confirm or cancel the write procedure done before,
// by the Long Characteristic Write procedure. In order to do this, the function checks for
// the exec_write_flag in the parameters received with the event. If the flag equals the
// execute flag represented by exec_write_flag, the write is confirmed and the buffer is printed in the log;
// if not, then it means the write is canceled and all the data that has been written is deleted.
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC){
        esp_log_buffer_hex(GATTS_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }else{
        ESP_LOGI(GATTS_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

// The Application Profiles are stored in an array and corresponding callback functions
// gatts_profile_a_event_handler() and gatts_profile_b_event_handler() are assigned.
// The registering event handler:
static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    // case 1
    // When an Application Profile is registered, an ESP_GATTS_REG_EVT event is triggered
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true; // Service handle
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00; // Characteristic handle
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16; // Characteristic value handle
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_A; // Characteristic descriptor handle

        // set the device name, the esp_ble_gap_set_device_name() function is used.
        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(TEST_DEVICE_NAME); // "ESP_GATTS_DEMO"
        if (set_dev_name_ret){
            ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
#ifdef CONFIG_SET_RAW_ADV_DATA
        // It is possible to also advertise customized raw data using the esp_ble_gap_config_adv_data_raw()
        // and esp_ble_gap_config_scan_rsp_data_raw() functions, which require to
        // create and pass a buffer for both advertising data and scanning response data.
        // In this example, the raw data is represented by the raw_adv_data[] and raw_scan_rsp_data[] arrays
        esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
        if (raw_adv_ret){
            ESP_LOGE(GATTS_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
        }
        adv_config_done |= adv_config_flag;
        esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
        if (raw_scan_ret){
            ESP_LOGE(GATTS_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
        }
        adv_config_done |= scan_rsp_config_flag;
#else
        // The function used to configure standard Bluetooth Specification advertisement
        // parameters is esp_ble_gap_config_adv_data(), which takes a pointer to an
        // esp_ble_adv_data_t structure.
        //config adv data

        // esp_ble_gap_config_adv_data() configures the data that will be advertised to the
        // client and takes an esp_ble_adv_data_t structure, while esp_ble_gap_start_advertising()
        // makes the server to actually start advertising and takes an esp_ble_adv_params_t structure.

        // For this example, the advertisement parameters are initialized as follows:
        // static esp_ble_adv_params_t test_adv_params

        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret){
            ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= adv_config_flag;
        //config scan response data
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret){
            ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= scan_rsp_config_flag;

#endif
    // The register event is also used to create a profile attributes table by using the esp_ble_gatts_create_attr_tab()
    // function, which takes an esp_gatts_attr_db_t type structure that has all the information of the service table.
    // The way to create this table is:
        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_A);
        break;
    // case 2
    // Now that the services and characteristics are created and started, the program
    // can receive read and write events. The read operations are represented by the
    // ESP_GATTS_READ_EVT event, which has the following parameters:
    // uint16_t conn_id;          /*!< Connection id */
    // uint32_t trans_id;         /*!< Transfer id */
    // esp_bd_addr_t bda;         /*!< The bluetooth device address which been read */
    // uint16_t handle;           /*!< The attribute handle */
    // uint16_t offset;           /*!< Offset of the value, if the value is too long */
    // bool is_long;              /*!< The value is too long or not */
    // bool need_rsp;             /*!< The read operation need to do response */

    case ESP_GATTS_READ_EVT: {
        ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0xde;
        rsp.attr_value.value[1] = 0xed;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }
    // case 3
    // Managing Write Events
    // The write events are represented by the ESP_GATTS_WRITE_EVT event, which has the following parameters:
    // uint16_t conn_id;         /*!< Connection id */
    // uint32_t trans_id;        /*!< Transfer id */
    // esp_bd_addr_t bda;        /*!< The bluetooth device address which been written */
    // uint16_t handle;          /*!< The attribute handle */
    // uint16_t offset;          /*!< Offset of the value, if the value is too long */
    // bool need_rsp;            /*!< The write operation need to do response */
    // bool is_prep;             /*!< This write operation is prepare write */
    // uint16_t len;             /*!< The write attribute value length */
    // uint8_t *value;           /*!< The write attribute value */

    // There are two types of write events implemented in this example, write characteristic
    // value and write long characteristic value.
    // The first type of write is used when the characteristic value can fit in one
    // Attribute Protocol Maximum Transmission Unit (ATT MTU), which is usually 23 bytes long.
    // The second type is used when the attribute to write is longer than what can be
    // sent in one single ATT message by dividing the data into multiple chunks using
    // Prepare Write Responses, after which an Executive Write Request is used to
    // confirm or cancel the complete write request.
    case ESP_GATTS_WRITE_EVT: {
        ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
        if (!param->write.is_prep){
            ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
            esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
            if (gl_profile_tab[PROFILE_A_APP_ID].descr_handle == param->write.handle && param->write.len == 2){
                uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                if (descr_value == 0x0001){
                    if (a_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
                        ESP_LOGI(GATTS_TAG, "notify enable");
                        uint8_t notify_data[15];

                        for (int i = 0; i < sizeof(notify_data); ++i)
                        {
                            //notify_data[i] = i%0xff;
                          //  notify_data[i] = 0xff; // -------------

                            switch(i){
                              case 0: notify_data[0] = 0x10; break;
                              case 1: notify_data[1] = 0x11; break;
                              case 2: notify_data[2] = 0x12; break;
                              case 3: notify_data[3] = 0x13; break;
                              case 4: notify_data[4] = 0x14; break;
                              case 5: notify_data[5] = 0x15; break;
                              case 6: notify_data[6] = 0x16; break;
                              case 7: notify_data[7] = 0x17; break;
                              case 8: notify_data[8] = 0x18; break;
                              case 9: notify_data[9] = 0x19; break;
                              case 10: notify_data[10] = 0x1a; break;
                              case 11: notify_data[11] = 0x1b; break;
                              case 12: notify_data[12] = 0x1c; break;
                              case 13: notify_data[13] = 0x1d; break;
                              case 14: notify_data[14] = 0x1e; break;
                            }
                        }
                        //the size of notify_data[] need less than MTU size
                        // send data
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                sizeof(notify_data), notify_data, false);
                    }
                }else if (descr_value == 0x0002){
                    if (a_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                        ESP_LOGI(GATTS_TAG, "indicate enable");
                        uint8_t indicate_data[15];
                        for (int i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = i%0xff;
                            /////////////////////////////
                          /*  switch(i){
                              case 0: indicate_data[0] = 0x20; break;
                              case 1: indicate_data[1] = 0x21; break;
                              case 2: indicate_data[2] = 0x22; break;
                              case 3: indicate_data[3] = 0x23; break;
                              case 4: indicate_data[4] = 0x24; break;
                              case 5: indicate_data[5] = 0x25; break;
                              case 6: indicate_data[6] = 0x26; break;
                              case 7: indicate_data[7] = 0x27; break;
                              case 8: indicate_data[8] = 0x28; break;
                              case 9: indicate_data[9] = 0x29; break;
                              case 10: indicate_data[10] = 0x2a; break;
                              case 11: indicate_data[11] = 0x2b; break;
                              case 12: indicate_data[12] = 0x2c; break;
                              case 13: indicate_data[13] = 0x2d; break;
                              case 14: indicate_data[14] = 0x2e; break;
                            }*/
                        }
                        //the size of indicate_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                sizeof(indicate_data), indicate_data, true);
                    }
                }
                else if (descr_value == 0x0000){
                    ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
                }else{
                    ESP_LOGE(GATTS_TAG, "unknown descr value");
                    esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
                }
            }
        }
        // When a write event is triggered, this example prints logging messages,
        // and then executes the example_write_event_env() function.
        example_write_event_env(gatts_if, &a_prepare_write_env, param);
        break;
    }
    // case 4
    // The client finishes the long write sequence by sending an Executive Write Request.
    // This command triggers an ESP_GATTS_EXEC_WRITE_EVT event.
    // The server handles this event by sending a response and executing the example_exec_write_event_env() function:
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        example_exec_write_event_env(&a_prepare_write_env, param);
        break;
    // case 5
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    // case 5
    case ESP_GATTS_UNREG_EVT:
        break;
    // case 6
    // When a service is created successfully, an ESP_GATTS_CREATE_EVT event managed
    // by the profile GATT handler is triggered, and can be used to start the service
    // and add characteristics to the service. For the case of Profile A, the service
    // is started and the characteristics are added as follows:
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_A_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_A;

        // The service is started using the esp_ble_gatts_start_service() function with the service handle previously generated.
        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_A_APP_ID].service_handle);
        a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                        a_property,
                                                        &gatts_demo_char1_val, NULL); // {0x11,0x22,0x33}
        // Permissions:
        // ESP_GATT_PERM_READ: To read characteristic value is permitted
        // ESP_GATT_PERM_WRITE: To write characteristic value is permitted
        // Properties:
        // ESP_GATT_CHAR_PROP_BIT_READ: Characteristic can be read
        // ESP_GATT_CHAR_PROP_BIT_WRITE: Characteristic can be written
        // ESP_GATT_CHAR_PROP_BIT_NOTIFY: Characteristic can notify value changes

        // the read and write properties of an attribute are information that is shown
        // to the client in order to let the client know if the server accepts read and write requests.
        if (add_char_ret){
            ESP_LOGE(GATTS_TAG, "add char failed, error code =%x",add_char_ret);
        }
        break;
    // case 7
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    // case 8
    // Adding a characteristic to a service triggers an ESP_GATTS_ADD_CHAR_EVT event,
    // which returns the handle generated by the stack for the characteristic just added.
    case ESP_GATTS_ADD_CHAR_EVT: {
        uint16_t length = 0;
        const uint8_t *prf_char;

        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n",
                param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        gl_profile_tab[PROFILE_A_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);
        if (get_attr_ret == ESP_FAIL){
            ESP_LOGE(GATTS_TAG, "ILLEGAL HANDLE");
        }

        ESP_LOGI(GATTS_TAG, "the gatts demo char length = %x\n", length);
        for(int i = 0; i < length; i++){
            ESP_LOGI(GATTS_TAG, "prf_char[%x] =%x\n",i,prf_char[i]);
        }
        esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].descr_uuid,
                                                                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
        if (add_descr_ret){
            ESP_LOGE(GATTS_TAG, "add char descr failed, error code =%x", add_descr_ret);
        }
        break;
    }
    // case 9
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        gl_profile_tab[PROFILE_A_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
                 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    // case 10
    case ESP_GATTS_DELETE_EVT:
        break;
    // case 11
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
                 param->start.status, param->start.service_handle);
        break;
    // case 12
    case ESP_GATTS_STOP_EVT:
        break;
    // case 13
    // An ESP_GATTS_CONNECT_EVT is triggered when a client has connected to the GATT server
    // This event is used to update the connection parameters, such as latency,
    // minimum connection interval, maximum connection interval and time out.
    case ESP_GATTS_CONNECT_EVT: {
        esp_ble_conn_update_params_t conn_params = {0}; // The connection parameters are stored
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = param->connect.conn_id;
        //start sent the update connection parameters to the peer device.
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    }
    // case 14
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
    // case 15
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status %d attr_handle %d", param->conf.status, param->conf.handle);
        if (param->conf.status != ESP_GATT_OK){
            esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
        }
        break;
    // case 16
    case ESP_GATTS_OPEN_EVT:
    // case 17
    case ESP_GATTS_CANCEL_OPEN_EVT:
    // case 18
    case ESP_GATTS_CLOSE_EVT:
    // case 19
    case ESP_GATTS_LISTEN_EVT:
    // case 29
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}

static void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_B_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_B_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_B_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_B_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_B;

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_B_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_B);
        break;
    case ESP_GATTS_READ_EVT: {
        ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 4;
        rsp.attr_value.value[0] = 0x00; // -------------------------------------
        //rsp.attr_value.value[0] = 0xde;
        rsp.attr_value.value[1] = 0xed;
        rsp.attr_value.value[2] = 0xbe;
        rsp.attr_value.value[3] = 0xef;
        // In this example, a response is constructed with dummy data and sent back to
        // the host using the same handle given by the event.
        // In addition to the response, the GATT interface, the connection ID and
        // the transfer ID are also included as parameters in the esp_ble_gatts_send_response() function.
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT: {
        ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d\n", param->write.conn_id, param->write.trans_id, param->write.handle);
        if (!param->write.is_prep){
            ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
            esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
            if (gl_profile_tab[PROFILE_B_APP_ID].descr_handle == param->write.handle && param->write.len == 2){
                uint16_t descr_value= param->write.value[1]<<8 | param->write.value[0];
                if (descr_value == 0x0001){
                    if (b_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
                        ESP_LOGI(GATTS_TAG, "notify enable");
                        uint8_t notify_data[15];
                        for (int i = 0; i < sizeof(notify_data); ++i)
                        {
                            notify_data[i] = i%0xff;
                            ////////////////////////////
                            /*switch(i){
                              case 0: notify_data[0] = 0x30; break;
                              case 1: notify_data[1] = 0x31; break;
                              case 2: notify_data[2] = 0x32; break;
                              case 3: notify_data[3] = 0x33; break;
                              case 4: notify_data[4] = 0x34; break;
                              case 5: notify_data[5] = 0x35; break;
                              case 6: notify_data[6] = 0x36; break;
                              case 7: notify_data[7] = 0x37; break;
                              case 8: notify_data[8] = 0x38; break;
                              case 9: notify_data[9] = 0x39; break;
                              case 10: notify_data[10] = 0x3a; break;
                              case 11: notify_data[11] = 0x3b; break;
                              case 12: notify_data[12] = 0x3c; break;
                              case 13: notify_data[13] = 0x3d; break;
                              case 14: notify_data[14] = 0x3e; break;
                            }*/
                        }
                        //the size of notify_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_B_APP_ID].char_handle,
                                                sizeof(notify_data), notify_data, false);
                    }
                }else if (descr_value == 0x0002){
                    if (b_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                        ESP_LOGI(GATTS_TAG, "indicate enable");
                        uint8_t indicate_data[15];
                        for (int i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = i%0xff;
                        }
                        //the size of indicate_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_B_APP_ID].char_handle,
                                                sizeof(indicate_data), indicate_data, true);
                    }
                }
                else if (descr_value == 0x0000){
                    ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
                }else{
                    ESP_LOGE(GATTS_TAG, "unknown value");
                }

            }
        }
        example_write_event_env(gatts_if, &b_prepare_write_env, param);
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        example_exec_write_event_env(&b_prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_B_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_B_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_B_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_B;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_B_APP_ID].service_handle);
        b_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        esp_err_t add_char_ret =esp_ble_gatts_add_char( gl_profile_tab[PROFILE_B_APP_ID].service_handle, &gl_profile_tab[PROFILE_B_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                        b_property,
                                                        NULL, NULL);
        if (add_char_ret){
            ESP_LOGE(GATTS_TAG, "add char failed, error code =%x",add_char_ret);
        }
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

        gl_profile_tab[PROFILE_B_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_B_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_B_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_B_APP_ID].service_handle, &gl_profile_tab[PROFILE_B_APP_ID].descr_uuid,
                                     ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                     NULL, NULL);
        break;
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        gl_profile_tab[PROFILE_B_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
                 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_B_APP_ID].conn_id = param->connect.conn_id;
        break;
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT status %d attr_handle %d", param->conf.status, param->conf.handle);
        if (param->conf.status != ESP_GATT_OK){
            esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
        }
    break;
    case ESP_GATTS_DISCONNECT_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}

// The event is captured by the gatts_event_handler(), which used to store the generated
// interface in the profile table, and then the event is forwarded to the corresponding profile event handler
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gatts_if == gl_profile_tab[idx].gatts_if) {
                if (gl_profile_tab[idx].gatts_cb) {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

void app_main(void)
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // The BT controller implements the Host Controller Interface (HCI) on the
    // controller side, the Link Layer (LL) and the Physical Layer (PHY)
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg); //  the BT controller is initialized and enabled
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    // the controller is enabled in BLE Mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    //  the Bluedroid stack, which includes the common definitions and APIs for
    // both BT Classic and BLE, is initialized and enabled
    ret = esp_bluedroid_init(); // Bluedroid initialized
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable(); // Bluedroid enabled
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }


    // The Bluetooth stack is up and running at this point in the program flow,
    // however the functionality of the application has not been defined yet.
    // The functionality is defined by reacting to events such as what happens
    // when another device tries to read or write parameters and establish a connection.
    // The two main managers of events are the GAP and GATT event handlers

    // The application needs to register a callback function for each event handler
    // in order to let the application know which functions are going to handle the GAP and GATT events

    // The structure implementation is: struct gatts_profile_inst {}

    ret = esp_ble_gatts_register_callback(gatts_event_handler); //  handle all the events that are pushed to the application from the BLE stack
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler); //  handle all the events that are pushed to the application from the BLE stack
    if (ret){
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }

    // The GATT Server example application is organized by using Application Profiles.
    // Each Application Profile describes a way to group functionalities that are
    // designed for one client application, such as a mobile app running on a smartphone or tablet.
    // In this way, a single design, enabled by different Application Profiles,
    // can behave differently when used by different smartphone apps, allowing the
    // server to react differently according to the client app that is being used.
    // In practice, each profile is seen by the client as an independent BLE service.
    // It is up to the client to discriminate the services that it is interested in.

    // Each profile is defined as a struct where the struct members depend on the
    // services and characteristics that are implemented in that Application Profile.
    // The members also include a GATT interface, Application ID, Connection ID and a
    // callback function to handle profile events. In this example, each profile is composed by:
    // 1) GATT interface            2) Application ID        3) Connection ID       4) Service handle
    // 5) Service ID                6) Characteristic handle 7) Characteristic UUID 8) Attribute permissions
    // 9) Characteristic properties 10)Client Characteristic 11) Configuration descriptor handle
    // 12) Client Characteristic    13) Configuration descriptor UUID

    // This profile was designed to have one service and one characteristic, and that
    // the characteristic has one descriptor. The service has a handle and an ID, in the same
    // manner that each characteristic has a handle, an UUID, attribute permissions and properties.

    // The Application Profiles are registered using the Application ID, which is an
    // user-assigned number to identify each profile. In this way, multiple Application
    // Profiles can run in one server
    ret = esp_ble_gatts_app_register(PROFILE_A_APP_ID); // Application Profiles are registered using the Application ID
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_B_APP_ID); // Application Profiles are registered using the Application ID
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret){
        ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }

    return;
}
