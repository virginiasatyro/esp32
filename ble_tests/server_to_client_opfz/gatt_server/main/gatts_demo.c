#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
// #include "bt.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#define GATTS_TAG "APP_BLUETOOTH"

// defines bluetooth.h
#define GATTS_SERVICE_UUID_TEST_A   0x00FF
#define GATTS_CHAR_UUID_TEST_A      0xFF01
#define GATTS_DESCR_UUID_TEST_A     0x3333
#define GATTS_NUM_HANDLE_TEST_A     4

/*
#define GATTS_SERVICE_UUID_TEST_B   0x00EE
#define GATTS_CHAR_UUID_TEST_B      0xEE01
#define GATTS_DESCR_UUID_TEST_B     0x2222
#define GATTS_NUM_HANDLE_TEST_B     4

#define GATTS_SERVICE_UUID_TEST_C   0x00DD
#define GATTS_CHAR_UUID_TEST_C      0xDD01
#define GATTS_DESCR_UUID_TEST_C     0x1111
#define GATTS_NUM_HANDLE_TEST_C     4*/

#define TEST_DEVICE_NAME            "ESP_GATTS_DEMO" //"MESTRIA"
#define TEST_MANUFACTURER_DATA_LEN  17

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40

#define PREPARE_BUF_MAX_SIZE 1024

#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

#define PROFILE_NUM 1 //3
#define PROFILE_A_APP_ID 0
// #define PROFILE_B_APP_ID 1
// #define PROFILE_C_APP_ID 2

// ---- defines bluetooth.h

// structs ------------------------------------------------------
/////////////////////////////////
/*
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
        0x02, 0x01, 0x06,
        0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd
};
static uint8_t raw_scan_rsp_data[] = {
        0x0f, 0x09, 0x45, 0x53, 0x50, 0x5f, 0x47, 0x41, 0x54, 0x54, 0x53, 0x5f, 0x44,
        0x45, 0x4d, 0x4f
};
#else*/
/////////////////////////////////////////

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

// The length of adv data must be less than 31 bytes
//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
//adv data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false, //true,
    .min_interval = 0x0006, //0x20,
    .max_interval = 0x0010, //0x40,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 32,
    .p_service_uuid = adv_service_uuid128, //adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x20,
    //.max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 32,
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

//#endif  //-------------

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

prepare_type_env_t a_prepare_write_env;
// prepare_type_env_t b_prepare_write_env;

static uint8_t char1_str[] = {0x11,0x22,0x33};

static esp_attr_value_t gatts_demo_char1_val =
{
    .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,
    .attr_len     = sizeof(char1_str),
    .attr_value   = char1_str,
};

static esp_gatt_char_prop_t a_property = 0;
//static esp_gatt_char_prop_t b_property = 0;
//static esp_gatt_char_prop_t c_property = 0;

// structs --------------------------------------------------------------

unsigned int voltage; // -----------

// functions -------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
//void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
//void gatts_profile_c_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

void temperature_dispatch(void* arg);
void door_dispatch(void* arg);
void status_dispatch(void* arg);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
    }, /*
    [PROFILE_B_APP_ID] = {
        .gatts_cb = gatts_profile_b_event_handler,                   // This demo does not implement, similar as profile A
        .gatts_if = ESP_GATT_IF_NONE,       // Not get the gatt_if, so initial is ESP_GATT_IF_NONE
    },

    [PROFILE_C_APP_ID] = {
        .gatts_cb = gatts_profile_c_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    },
*/

};
// functions -------------------------------------------------------------------

// start - bluetoot.c
struct ble_params {
	uint8_t gatts_if;
	uint16_t conn_id;
	uint16_t handle;
};

static uint8_t adv_config_done = 0;
int st_temp = 0, st_door = 0, st_onoff;
struct ble_params temp_ble, door_ble, status_ble;

char FLAG_CONF_TEMP = 0;
char FLAG_CONF_DOOR = 0;
char FLAG_NOTIFY_STATUS = 0;
char FLAG_INIT_DOOR = 0;
char FLAG_INIT_TEMP = 0;

void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    esp_gatt_status_t status = ESP_GATT_OK;
    if (param->write.need_rsp){
        if (param->write.is_prep){
            if (prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(GATTS_TAG, "Gatt_server prep no mem\n");
                    status = ESP_GATT_NO_RESOURCES;
                }
            } else {
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

//static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_A_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_A;

        esp_ble_gap_set_device_name(TEST_DEVICE_NAME);

        //config adv data
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

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_A);
        break;
    case ESP_GATTS_READ_EVT: {
        /*ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 2;
        rsp.attr_value.value[0] = voltage & 0xff;
        rsp.attr_value.value[1] = (voltage >> 8) & 0xff;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;*/
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
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                sizeof(notify_data), notify_data, false);
                    }
                }else if (descr_value == 0x0002){
                    /*if (a_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                        ESP_LOGI(GATTS_TAG, "indicate enable");
                        st_door = 1;
                        FLAG_INIT_DOOR = 1;
                        door_ble.gatts_if = gatts_if;
                        door_ble.conn_id = param->write.conn_id;
                        door_ble.handle = gl_profile_tab[PROFILE_A_APP_ID].char_handle;
                     }*/

                }
                else if (descr_value == 0x0000){
                    ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
                }else{
                    ESP_LOGE(GATTS_TAG, "unknown descr value");
                    esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
                }

            } else if(param->write.value[0] == 0xff) {
            //	erase_door_data(); -------------------
            }
        }
        example_write_event_env(gatts_if, &a_prepare_write_env, param);
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        example_exec_write_event_env(&a_prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_CONF_EVT:
    	//ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT_A");
    	FLAG_CONF_DOOR = 1;
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_A_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_A_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_A;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_A_APP_ID].service_handle);
        a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        //a_property = ESP_GATT_CHAR_PROP_BIT_INDICATE | ESP_GATT_CHAR_PROP_BIT_WRITE;
        esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                        a_property,
                                                        &gatts_demo_char1_val, NULL);
        if (add_char_ret){
            ESP_LOGE(GATTS_TAG, "add char failed, error code =%x",add_char_ret);
        }
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT: {
        uint16_t length = 0;
        const uint8_t *prf_char;

        ESP_LOGI(GATTS_TAG, "ADD_CHAR_A_EVT, status %d,  attr_handle %d, service_handle %d\n",
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
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        gl_profile_tab[PROFILE_A_APP_ID].descr_handle = param->add_char_descr.attr_handle;
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
    case ESP_GATTS_CONNECT_EVT: {
        esp_ble_conn_update_params_t conn_params = {0};
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
    case ESP_GATTS_DISCONNECT_EVT:
    	FLAG_NOTIFY_STATUS = 0;
    	st_door = 0;
    	st_temp = 0;
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT");
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}
/*
void gatts_profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {

    switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_B_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
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
        rsp.attr_value.len = 2;
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
                        }
                        //the size of notify_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_B_APP_ID].char_handle,
                                                sizeof(notify_data), notify_data, false);
                    }
                }else if (descr_value == 0x0002){
                    if (b_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                        ESP_LOGI(GATTS_TAG, "indicate enable");
                        st_temp = 1;
                        FLAG_INIT_TEMP = 1;
                        temp_ble.gatts_if = gatts_if;
                        temp_ble.conn_id = param->write.conn_id;
                        temp_ble.handle = gl_profile_tab[PROFILE_B_APP_ID].char_handle;
                    }
                }
                else if (descr_value == 0x0000){
                    ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
                }else{
                    ESP_LOGE(GATTS_TAG, "unknown value");
                    esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
                }

            } else if(param->write.value[0] == 0xff) {
            	//erase_temp_data();
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
    case ESP_GATTS_CONF_EVT:
    	//ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT");
        FLAG_CONF_TEMP = 1;
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_B_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_B_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_B_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_B_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_B;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_B_APP_ID].service_handle);
        b_property = ESP_GATT_CHAR_PROP_BIT_INDICATE | ESP_GATT_CHAR_PROP_BIT_WRITE;

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
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_B_EVT, status %d,  attr_handle %d, service_handle %d\n",
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
    case ESP_GATTS_DISCONNECT_EVT:
    	ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT_B");
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    	ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT");
        break;
    default:
        break;
    }
}
*/
/*
void gatts_profile_c_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {

    switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        gl_profile_tab[PROFILE_C_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_C_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_C_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_C_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_C;

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_C_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_C);
        break;
    case ESP_GATTS_READ_EVT: {
        ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 2;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);

        break;
    }
    case ESP_GATTS_WRITE_EVT: {
        ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d\n", param->write.conn_id, param->write.trans_id, param->write.handle);
        if (!param->write.is_prep){
            ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
            esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
            if (gl_profile_tab[PROFILE_C_APP_ID].descr_handle == param->write.handle && param->write.len == 2){
                uint16_t descr_value= param->write.value[1]<<8 | param->write.value[0];
                if (descr_value == 0x0001){
                    if (c_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
                        ESP_LOGI(GATTS_TAG, "notify enable");
                        FLAG_NOTIFY_STATUS = 1;
                        status_ble.gatts_if = gatts_if;
                        status_ble.conn_id = param->write.conn_id;
                        status_ble.handle = gl_profile_tab[PROFILE_C_APP_ID].char_handle;

                    }
                }else if (descr_value == 0x0002){
                    if (c_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                        ESP_LOGI(GATTS_TAG, "indicate enable");

                    }
                }
                else if (descr_value == 0x0000){
                    ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
                    FLAG_NOTIFY_STATUS = 0;
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
    case ESP_GATTS_CONF_EVT:
    	//ESP_LOGI(GATTS_TAG," ESP_GATTS_CONF_EVT");
    	break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_C_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_C_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_C_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_C_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_C;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_C_APP_ID].service_handle);
        c_property = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        esp_err_t add_char_ret =esp_ble_gatts_add_char( gl_profile_tab[PROFILE_C_APP_ID].service_handle, &gl_profile_tab[PROFILE_C_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                        c_property,
                                                        NULL, NULL);
        if (add_char_ret){
            ESP_LOGE(GATTS_TAG, "add char failed, error code =%x",add_char_ret);
        }
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_C_EVT, status %d,  attr_handle %d, service_handle %d\n",
                 param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);

        gl_profile_tab[PROFILE_C_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_C_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_C_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_C_APP_ID].service_handle, &gl_profile_tab[PROFILE_C_APP_ID].descr_uuid,
                                     ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                     NULL, NULL);
        break;
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        gl_profile_tab[PROFILE_C_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        ESP_LOGI(GATTS_TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
                 param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TAG, "SERVICE_C_START_EVT, status %d, service_handle %d\n",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_C_APP_ID].conn_id = param->connect.conn_id;
        break;
    case ESP_GATTS_DISCONNECT_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    	ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONGEST_EVT");
    	break;
    case ESP_GATTS_RESPONSE_EVT:
    	ESP_LOGI(GATTS_TAG, "ESP_GATTS_RESPONSE_EVT");
    	break;
    default:
        break;
    }
}
*/
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {

    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising start failed\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed\n");
        }
        else {
            ESP_LOGI(GATTS_TAG, "Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(GATTS_TAG, "update connetion params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
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

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
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

/*
void temperature_dispatch(void* arg)
{
    static uint8_t indicate_data[20], eof[3] = {0x45,0x4F,0x46}, aux[2];
    char data[21], file[80]; int i;
    static unsigned int count = 0, index_file = 0;
    esp_err_t ret;
    static unsigned int size; static FILE* f;
    static int size_count = 0;		//stores the number of file transfers needed to complete the download

    while(1){
        switch (st_temp) {
            case 0:
                //do nothing
                vTaskDelay(pdMS_TO_TICKS(10));
                break;

            case 1:
                //init
                if(FLAG_INIT_TEMP) {
                	if(take_temp_file()) {
                		//Opens all files and calculate the amount of packets to transfer
                		index_file = 0;
                        size_count = 0;
                		sprintf(file, "/sdcard/temp%d.dat", index_file);
                        f = fopen(file, "r");
                		while(f != NULL) {
                        	fseek(f, 0, SEEK_END);
                    		size = ftell(f)/20; // get current file pointer
                    		size_count += size;
                    		fclose(f);
                    		index_file++;
                    		sprintf(file, "/sdcard/temp%d.dat", index_file);
                        	f = fopen(file, "r");
                		}

                		ESP_LOGI(GATTS_TAG, "Transferindo %d bytes em %d pacotes...", size_count*20, size_count);
                		aux[0] = size_count & 0xff;
                		aux[1] = (size_count >> 8) & 0xff;
                		esp_ble_gatts_send_indicate(temp_ble.gatts_if, temp_ble.conn_id, temp_ble.handle, 2, aux, false);
                		fclose(f);
                		size_count = 0;
                		index_file = 0;
                		FLAG_INIT_TEMP = 0;
                		give_temp_file();
                    } else {
                        ESP_LOGE(GATTS_TAG, "Failed to open temp file. The file is busy!");
                        vTaskDelay(pdMS_TO_TICKS(10));
                        break;
                    }
                }

                if(index_file <= get_index_tempfile()) {
                    ESP_LOGI(GATTS_TAG, "Opening temp%d file to transfer", index_file);
                    if(take_temp_file()) {
                        sprintf(file, "/sdcard/temp%d.dat", index_file);
                        f = fopen(file, "r");
                        if (f == NULL) {
                            ESP_LOGE(GATTS_TAG, "Failed to open temp%d for reading", index_file);
                            give_temp_file();
                            st_temp = 0;
                            vTaskDelay(pdMS_TO_TICKS(10));
                            break;
                        }
                    } else {
                        ESP_LOGE(GATTS_TAG, "Failed to open temp file. The file is busy!");
                        vTaskDelay(pdMS_TO_TICKS(10));
                        break;
                    }

                    st_temp = 2;
                    FLAG_CONF_TEMP = 1;
                } else {
                    //No more files to transfer
                    ret = esp_ble_gatts_send_indicate(temp_ble.gatts_if, temp_ble.conn_id, temp_ble.handle, sizeof(eof), eof, true);

                    if(ret != ESP_OK)
                        ESP_LOGE(GATTS_TAG, "Erro EOF. %d", count);
                    ESP_LOGI(GATTS_TAG, "Arquivo temp%d transferido!", (index_file-1));
                    st_temp = 0;
                    count = 0;
                    index_file = 0;
                }
                break;

            case 2:

                if(FLAG_CONF_TEMP) {
                    if(fgets(data, sizeof(data), f) != NULL) {
                        count++;
                        for (i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = (uint8_t)data[i];
                        }
                        ret = esp_ble_gatts_send_indicate(temp_ble.gatts_if, temp_ble.conn_id, temp_ble.handle, sizeof(indicate_data), indicate_data, true);
                        FLAG_CONF_TEMP = 0;

                        if(ret != ESP_OK){
                            ESP_LOGE(GATTS_TAG, "Erro na transferência. %d", count);
                            fclose(f);
                            give_temp_file();
                            count = 0;
                            index_file = 0;
                            st_temp = 0;
                            //TODO: processar erros de envio
                            break;
                        }
                    } else {

                        fclose(f);
                        give_temp_file();
                        st_temp = 1;
                        index_file++;
                        break;
                    }

                    ESP_LOGI(GATTS_TAG, "%d", count);
                }

                vTaskDelay(pdMS_TO_TICKS(10));
                break;

            default:
                break;
        }
    }
}
*/
/*
void door_dispatch(void* arg)
{
    static uint8_t indicate_data[20], eof[3] = {0x45,0x4F,0x46}, aux[2];
    char data[21], file[80]; int i;
    static unsigned int count = 0, index_file = 0;
    esp_err_t ret;
    static unsigned int size; static FILE* f;
    static int size_count = 0;		//stores the number of file transfers needed to complete the download

    while(1){
        switch (st_door) {
            case 0:
                //do nothing
                vTaskDelay(pdMS_TO_TICKS(10));
                break;

            case 1:
                //init
                if(FLAG_INIT_DOOR) {
                	if(take_door_file()) {
                		//Opens all files and calculate the amount of packets to transfer
                		index_file = 0;
                        size_count = 0;
                		sprintf(file, "/sdcard/door%d.dat", index_file);
                        f = fopen(file, "r");
                		while(f != NULL) {
                        	fseek(f, 0, SEEK_END);
                    		size = ftell(f)/20; // get current file pointer
                    		size_count += size;
                    		fclose(f);
                    		index_file++;
                    		sprintf(file, "/sdcard/door%d.dat", index_file);
                        	f = fopen(file, "r");
                		}

                		ESP_LOGI(GATTS_TAG, "Transferindo %d bytes em %d pacotes...", size_count*20, size_count);
                		aux[0] = size_count & 0xff;
                		aux[1] = (size_count >> 8) & 0xff;
                		esp_ble_gatts_send_indicate(door_ble.gatts_if, door_ble.conn_id, door_ble.handle, 2, aux, false);
                		fclose(f);
                		size_count = 0;
                		index_file = 0;
                		FLAG_INIT_DOOR = 0;
                		give_door_file();
                    } else {
                        ESP_LOGE(GATTS_TAG, "Failed to open door file. The file is busy!");
                        vTaskDelay(pdMS_TO_TICKS(10));
                        break;
                    }
                }

                if(index_file <= get_index_doorfile()) {
                    ESP_LOGI(GATTS_TAG, "Opening door%d file to transfer", index_file);
                    if(take_door_file()) {
                        sprintf(file, "/sdcard/door%d.dat", index_file);
                        f = fopen(file, "r");
                        if (f == NULL) {
                            ESP_LOGE(GATTS_TAG, "Failed to open door%d for reading", index_file);
                            give_door_file();
                            st_door = 0;
                            vTaskDelay(pdMS_TO_TICKS(10));
                            break;
                        }
                    } else {
                        ESP_LOGE(GATTS_TAG, "Failed to open door file. The file is busy!");
                        vTaskDelay(pdMS_TO_TICKS(10));
                        break;
                    }
                    st_door = 2;
                    FLAG_CONF_DOOR = 1;
                } else {
                    //No more files to transfer
                    ret = esp_ble_gatts_send_indicate(door_ble.gatts_if, door_ble.conn_id, door_ble.handle, sizeof(eof), eof, true);

                    if(ret != ESP_OK)
                        ESP_LOGE(GATTS_TAG, "Erro EOF. %d", count);
                    ESP_LOGI(GATTS_TAG, "Arquivo door%d transferido!", (index_file-1));
                    st_door = 0;
                    count = 0;
                    index_file = 0;
                }

                break;

            case 2:

                if(FLAG_CONF_DOOR) {
                    if(fgets(data, sizeof(data), f) != NULL) {
                        count++;
                        for (i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = (uint8_t)data[i];
                        }
                        ret = esp_ble_gatts_send_indicate(door_ble.gatts_if, door_ble.conn_id, door_ble.handle, sizeof(indicate_data), indicate_data, true);
                        FLAG_CONF_DOOR = 0;

                        if(ret != ESP_OK){
                            ESP_LOGE(GATTS_TAG, "Erro na transferência. %d", count);
                            fclose(f);
                            give_door_file();
                            count = 0;
                            index_file = 0;
                            st_door = 0;
                            //TODO: processar erros de envio
                            break;
                        }
                    } else {

                        fclose(f);
                        give_door_file();
                        st_door = 1;
                        index_file++;
                        break;
                    }
	                ESP_LOGI(GATTS_TAG, "%d", count);
                }
				vTaskDelay(pdMS_TO_TICKS(10));
                break;

            default:
                break;
        }
    }
}
*/
// void status_dispatch(void* arg) { ------------------
void app_main(void* arg){
  //initilizations ----------------------------------------------------
  esp_err_t ret;

  // Initialize NVS.
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK( ret );

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg); //  the BT controller is initialized and enabled
  if (ret) {
      ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
      return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret) {
      ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
      return;
  }

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

  ret = esp_ble_gatts_app_register(PROFILE_A_APP_ID); // Application Profiles are registered using the Application ID
  if (ret){
      ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
      return;
  }

  esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
  if (local_mtu_ret){
      ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
  }



  uint8_t data[15];

	data[0] = 0x01;
  data[1] = 0x02;
	data[2] = 0x03;
	data[3] = 0x04;
  data[4] = 0x01;
  data[5] = 0x02;
  data[6] = 0x03;
  data[7] = 0x04;
  data[8] = 0x01;
  data[9] = 0x02;
  data[10] = 0x03;
  data[11] = 0x04;
  data[12] = 0x01;
  data[13] = 0x02;
  data[14] = 0x03;
  data[15] = 0x04;


	esp_ble_gatts_send_indicate(status_ble.gatts_if, status_ble.conn_id, status_ble.handle,
                                                5, data, false);


		vTaskDelay(pdMS_TO_TICKS(1000));

  //----------------------------------------------------
/*  printf("PASSA DO MTU");
  	// unsigned int temperature; ----------------------
    */
//	uint8_t data[4];
/*
	while(1) {
    printf("ENTRA NO WHILE");
		if(FLAG_NOTIFY_STATUS) {
			temperature = (uint16_t)(100*read_temperature());
			data[0] = temperature & 0xff;
			data[1] = (temperature >> 8) & 0xff;
			data[2] = (uint8_t)get_ioDoor();
			data[3] = 0;
			data[4] = teste;
			data[0] = 0x00;
      data[1] = 0x01;
      data[2] = 0x02;
      data[3] = 0x03;

			esp_ble_gatts_send_indicate(temp_ble.gatts_if, temp_ble.conn_id, temp_ble.handle,
                                                5, data, false);
*/
uint8_t notify_data[15];
for (int i = 0; i < sizeof(notify_data); ++i)
{
    //notify_data[i] = i%0xff;
    switch(i){
      case 0: notify_data[0] = 0x11; break;
      case 1: notify_data[1] = 0x22; break;
      case 2: notify_data[2] = 0x33; break;
      case 3: notify_data[3] = 044; break;
      case 4: notify_data[4] = 0x55; break;
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
//th

    esp_ble_gatts_send_indicate(temp_ble.gatts_if, temp_ble.conn_id, temp_ble.handle,
                                              sizeof(notify_data), notify_data, false);
		vTaskDelay(pdMS_TO_TICKS(5000));
}
