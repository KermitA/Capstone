//some test BT 
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#define SPP_TAG "SPP_ACCEPTOR_DEMO"
#define SPP_SERVER_NAME "SPP_SERVER"
#define EXAMPLE_DEVICE_NAME "Emergency Light Node 1"

#include "mdf_common.h"
#include "mwifi.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define SELECT_ROOT 0

//LED Matrix Pin Definitions
#define R1 12
#define B1 13
#define R2 32
#define B2 33
#define A 25
#define C 26
#define CLK 27
#define OE 14
#define G1 5
#define G2 17
#define B 16
#define LATCH 4


//Message type definitions
#define GET_MSG 0
#define SET_MSG 1
#define RETURN_MSG 2

//Color definitions
#define COLOR_RED 4
#define COLOR_GREEN 2
#define COLOR_BLUE 1
#define COLOR_YELLOW 6
#define COLOR_PINK 5
#define COLOR_WHITE 7
#define COLOR_CYAN 3
#define COLOR_CUSTOM 0

//Arrow/Graphic definitions
#define DIRECTION_UP 0
#define DIRECTION_DOWN 1
#define DIRECTION_LEFT 2
#define DIRECTION_RIGHT 3
#define DIRECTION_HERE 4

// #define MEMORY_DEBUG
#define BUF_SIZE 512

//For testing, can be deleted soon
char* root_mac = "c4:4f:33:3e:c7:ea";
char* node_mac = "c4:4f:33:3e:c5:89";

//Holds the mac address of the node
uint8_t sta_mac[MWIFI_ADDR_LEN]   = {0};

//Holds the current matrix to be displayed
uint8_t matrix[16][32][3];
//supported protocol version
int version = 0;
//fields that are used in mesh messages
int msgType, graphic, color, duration, currentTime;

static const char *TAG = "no-router";
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

//function prototypes
void clearMatrix(void);
void setAllRed(void);
void setAllGreen(void);
void setAllBlue(void);
void setUp(uint8_t);
void setDown(uint8_t);
void setRight(uint8_t);
void setLeft(uint8_t);
void setHere(uint8_t);
void modifyMatrix(void);
void parsePacket(cJSON *);

/*
	WORK IN PROGRESS
	Reads BT payload. Determines if it is a local request
	or a request that needs to be sent through the network.
*/
void bt_read_and_send(char *data)
{
	//first parse to JSON
	cJSON *root = cJSON_Parse(data);
	
	//grab the MAC
	cJSON * msgMac = cJSON_GetObjectItem(root, "dest");
	uint8_t dest_addr[MWIFI_ADDR_LEN] = {0};
	do {
		uint32_t mac_data[MWIFI_ADDR_LEN] = {0};
		sscanf(msgMac->valuestring, MACSTR,
			   mac_data, mac_data + 1, mac_data + 2,
			   mac_data + 3, mac_data + 4, mac_data + 5);

		for (int i = 0; i < MWIFI_ADDR_LEN; i++) 
		{
			dest_addr[i] = mac_data[i];
		}
	} while (0);
	
	uint8_t identicalMac = 1;
	int i = 0;
	for(i = 0; i < MWIFI_ADDR_LEN; i++)
	{
		if(dest_addr[i] != sta_mac[i])
		{
			identicalMac = 0;
			break;
		}
	}
	
	if(identicalMac)
	{
		//continue parsing and modify matrix
		int msgVersion = cJSON_GetObjectItem(root, "Version")->valueint;
		if(msgVersion != version)
			return;
		msgType = cJSON_GetObjectItem(root, "MsgType")->valueint;
		if(msgType == SET_MSG)
		{
			color = cJSON_GetObjectItem(root, "Color")->valueint;
			graphic = cJSON_GetObjectItem(root, "Graphic")->valueint;
			duration = cJSON_GetObjectItem(root, "Duration")->valueint;
			modifyMatrix();
		}
		gpio_set_level(23, 1);
		//write conditions for GET_MSG, RETURN_MSG here
		cJSON_Delete(root);
	}
	else
	{
		//send out
	}
	
	
	
}

//BT EVENT HANDLERS
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
	char bt_received_data[1024];
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
        esp_bt_dev_set_device_name(EXAMPLE_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        esp_spp_start_srv(sec_mask,role_slave, 0, SPP_SERVER_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_OPEN_EVT");
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT");
        break;
    case ESP_SPP_START_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT");
        break;
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT:
#if (SPP_SHOW_MODE == SPP_SHOW_DATA)
        ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d",
                 param->data_ind.len, param->data_ind.handle);
        esp_log_buffer_hex("",param->data_ind.data,param->data_ind.len);
		snprintf(bt_received_data, (size_t)param->data_ind.len, "%s/n", (char *)param->data_ind.data);
		/*
		uint8_t cmdColor = bt_received_data[0] - '0';
		uint8_t cmdDirection = bt_received_data[1];
		if(cmdColor < 4)
		{
			clearMatrix();
			if(cmdDirection >= 'a' && cmdDirection <= 'z')
			{
				if(cmdDirection == 'l')
					setLeft(cmdColor);
				else if(cmdDirection == 'r')
					setRight(cmdColor);
				else if(cmdDirection == 'u')
					setUp(cmdColor);
				else if(cmdDirection == 'd')
					setDown(cmdColor);
				else if(cmdDirection == 'h')
					setHere(cmdColor);
			}
			else
			{
				if(cmdColor == 1)
					setAllRed();
				else if(cmdColor == 2)
					setAllGreen();
				else if(cmdColor == 3)
					setAllBlue();
			}
		}
		*/
		//printf("First char: %d\n", cmdColor);
		//printf("%s\n", bt_received_data);
		esp_spp_write(param->write.handle, strlen(bt_received_data), 
				(uint8_t *)bt_received_data);
		
#else
        //gettimeofday(&time_new, NULL);
        data_num += param->data_ind.len;
        if (time_new.tv_sec - time_old.tv_sec >= 3) {
            print_speed();
        }
		char * bt_received_data = (char *)param->data_ind.data;
		printf(bt_received_data);
#endif
        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT");
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT");
        //gettimeofday(&time_old, NULL);
        break;
    default:
        break;
    }
}

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_BT_GAP_AUTH_CMPL_EVT:{
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(SPP_TAG, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(SPP_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            ESP_LOGE(SPP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:{
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit) {
            ESP_LOGI(SPP_TAG, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            ESP_LOGI(SPP_TAG, "Input pin code: 1234");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
    default: {
        ESP_LOGI(SPP_TAG, "event: %d", event);
        break;
    }
    }
    return;
}

void init_bt()
{
	//Bluetooth config
	esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

    /*
     * Set default parameters for Legacy Pairing
     * Use variable pin, input pin code when pairing
     */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);
}

//END BT EVENT HANDLERS

void setRight(uint8_t color)
{
	matrix[2][21][color] = 1;
	matrix[2][22][color] = 1;
	matrix[13][21][color] = 1;
	matrix[13][22][color] = 1;
	
	matrix[3][2][color] = 1;
	matrix[3][3][color] = 1;
	matrix[3][22][color] = 1;
	matrix[3][23][color] = 1;
	matrix[12][2][color] = 1;
	matrix[12][3][color] = 1;
	matrix[12][22][color] = 1;
	matrix[12][23][color] = 1;
	
	matrix[4][3][color] = 1;
	matrix[4][4][color] = 1;
	matrix[4][23][color] = 1;
	matrix[4][24][color] = 1;
	matrix[11][3][color] = 1;
	matrix[11][4][color] = 1;
	matrix[11][23][color] = 1;
	matrix[11][24][color] = 1;
	
	matrix[5][1][color] = 1;
	matrix[5][4][color] = 1;
	matrix[5][5][color] = 1;
	matrix[5][24][color] = 1;
	matrix[5][25][color] = 1;
	matrix[10][1][color] = 1;
	matrix[10][4][color] = 1;
	matrix[10][5][color] = 1;
	matrix[10][24][color] = 1;
	matrix[10][25][color] = 1;
	
	matrix[6][2][color] = 1;
	for(int i = 5; i<27; i++){
		matrix[6][i][color] = 1;
	}
	matrix[9][2][color] = 1;
	for(int i = 5; i<27; i++){
		matrix[9][i][color] = 1;
	}
	
	for(int i = 3; i<28; i++){
		matrix[7][i][color] = 1;
	}
	for(int i = 3; i<28; i++){
		matrix[8][i][color] = 1;
	}
}

void setLeft(uint8_t color)
{
	matrix[2][6][color] = 1;
	matrix[2][7][color] = 1;
	matrix[13][6][color] = 1;
	matrix[13][7][color] = 1;
	
	matrix[3][5][color] = 1;
	matrix[3][6][color] = 1;
	matrix[3][25][color] = 1;
	matrix[3][26][color] = 1;
	matrix[12][5][color] = 1;
	matrix[12][6][color] = 1;
	matrix[12][25][color] = 1;
	matrix[12][26][color] = 1;
	
	matrix[4][4][color] = 1;
	matrix[4][5][color] = 1;
	matrix[4][24][color] = 1;
	matrix[4][25][color] = 1;
	matrix[11][4][color] = 1;
	matrix[11][5][color] = 1;
	matrix[11][24][color] = 1;
	matrix[11][25][color] = 1;
	
	matrix[5][3][color] = 1;
	matrix[5][4][color] = 1;
	matrix[5][23][color] = 1;
	matrix[5][24][color] = 1;
	matrix[5][27][color] = 1;
	matrix[10][3][color] = 1;
	matrix[10][4][color] = 1;
	matrix[10][23][color] = 1;
	matrix[10][24][color] = 1;
	matrix[10][27][color] = 1;
	
	matrix[6][26][color] = 1;
	for(int i = 2; i<24; i++){
		matrix[6][i][color] = 1;
	}
	matrix[9][26][color] = 1;
	for(int i = 2; i<24; i++){
		matrix[9][i][color] = 1;
	}
	for(int i = 1; i<27; i++){
		matrix[7][i][color] = 1;
	}
	for(int i = 1; i<27; i++){
		matrix[8][i][color] = 1;
	}
}

void setHere(uint8_t color)
{
	matrix[3][4][color] = 1;
	matrix[3][7][color] = 1;
	for(int i = 9; i<13; i++){
		matrix[3][i][color] = 1;
	}
	for(int i = 14; i<18; i++){
		matrix[3][i][color] = 1;
	}
	for(int i = 19; i<23; i++){
		matrix[3][i][color] = 1;
	}
	for(int i = 24; i<26; i++){
		matrix[3][i][color] = 1;
	}
	
	
	for(int i = 4; i<7; i++){
		matrix[i][4][color] = 1;
		matrix[i][7][color] = 1;	
		matrix[i][9][color] = 1;
		matrix[i][14][color] = 1;
		matrix[i][17][color] = 1;
		matrix[i][19][color] = 1;
		matrix[i][24][color] = 1;
		matrix[i][25][color] = 1;
	}
	
	
	for(int i = 4; i<8; i++){
		matrix[7][i][color] = 1;
	}
	for(int i = 9; i<12; i++){
		matrix[7][i][color] = 1;
	}
	for(int i = 14; i<18; i++){
		matrix[7][i][color] = 1;
	}
	for(int i = 19; i<22; i++){
		matrix[7][i][color] = 1;
	}
	for(int i = 24; i<26; i++){
		matrix[7][i][color] = 1;
	}
	
	
	for(int i = 4; i<8; i++){
		matrix[8][i][color] = 1;
	}
	for(int i = 9; i<12; i++){
		matrix[8][i][color] = 1;
	}
	for(int i = 14; i<16; i++){
		matrix[8][i][color] = 1;
	}
	for(int i = 19; i<22; i++){
		matrix[8][i][color] = 1;
	}
	for(int i = 24; i<26; i++){
		matrix[8][i][color] = 1;
	}
	
	
	matrix[9][4][color] = 1;
	matrix[9][7][color] = 1;
	matrix[9][9][color] = 1;
	matrix[9][14][color] = 1;
	matrix[9][16][color] = 1;
	matrix[9][19][color] = 1;
	for(int i = 24; i<26; i++){
		matrix[9][i][color] = 1;
	}
	
	
	matrix[10][4][color] = 1;
	matrix[10][7][color] = 1;
	
   matrix[10][9][color] = 1;
   
	matrix[10][14][color] = 1;
	matrix[10][17][color] = 1;
	matrix[10][19][color] = 1;
	
	
	matrix[11][4][color] = 1;
	matrix[11][7][color] = 1;
	
    matrix[11][9][color] = 1;
   
	matrix[11][14][color] = 1;
	matrix[11][17][color] = 1;
	matrix[11][19][color] = 1;
	for(int i = 24; i<26; i++){
		matrix[11][i][color] = 1;
	}
	
	
	matrix[12][4][color] = 1;
	matrix[12][7][color] = 1;
	
	for(int i = 9; i<13; i++){
		matrix[12][i][color] = 1;
	}
	
	matrix[12][14][color] = 1;
	matrix[12][17][color] = 1;
	for(int i = 19; i<23; i++){
		matrix[12][i][color] = 1;
	}
	for(int i = 24; i<26; i++){
		matrix[12][i][color] = 1;
	}
}

void setUp(uint8_t color)
{
	matrix[1][8][color] = 1;
	matrix[1][24][color] = 1;
	matrix[8][8][color] = 1;
	matrix[8][24][color] = 1;
	
	matrix[2][7][color] = 1;
	matrix[2][9][color] = 1;
	matrix[2][23][color] = 1;
	matrix[2][25][color] = 1;
	matrix[9][7][color] = 1;
	matrix[9][9][color] = 1;
	matrix[9][23][color] = 1;
	matrix[9][25][color] = 1;
	
	matrix[3][6][color] = 1;
	matrix[3][10][color] = 1;
	matrix[3][22][color] = 1;
	matrix[3][26][color] = 1;
	matrix[10][6][color] = 1;
	matrix[10][10][color] = 1;
	matrix[10][22][color] = 1;
	matrix[10][26][color] = 1;
	
	matrix[4][5][color] = 1;
	matrix[4][11][color] = 1;
	matrix[4][21][color] = 1;
	matrix[4][27][color] = 1;
	matrix[11][5][color] = 1;
	matrix[11][11][color] = 1;
	matrix[11][21][color] = 1;
	matrix[11][27][color] = 1;
	
	matrix[5][4][color] = 1;
	matrix[5][12][color] = 1;
	matrix[5][20][color] = 1;
	matrix[5][28][color] = 1;
	matrix[12][4][color] = 1;
	matrix[12][12][color] = 1;
	matrix[12][20][color] = 1;
	matrix[12][28][color] = 1;
	
	matrix[6][3][color] = 1;
	matrix[6][13][color] = 1;
	matrix[6][19][color] = 1;
	matrix[6][29][color] = 1;
	matrix[13][3][color] = 1;
	matrix[13][13][color] = 1;
	matrix[13][19][color] = 1;
	matrix[13][29][color] = 1;
}

void setDown(uint8_t color)
{
	matrix[6][8][color] = 1;
	matrix[6][24][color] = 1;
	matrix[14][8][color] = 1;
	matrix[14][24][color] = 1;
	
	matrix[5][7][color] = 1;
	matrix[5][9][color] = 1;
	matrix[5][23][color] = 1;
	matrix[5][25][color] = 1;
	matrix[13][7][color] = 1;
	matrix[13][9][color] = 1;
	matrix[13][23][color] = 1;
	matrix[13][25][color] = 1;
	
	matrix[4][6][color] = 1;
	matrix[4][10][color] = 1;
	matrix[4][22][color] = 1;
	matrix[4][26][color] = 1;
	matrix[12][6][color] = 1;
	matrix[12][10][color] = 1;
	matrix[12][22][color] = 1;
	matrix[12][26][color] = 1;
	
	matrix[3][5][color] = 1;
	matrix[3][11][color] = 1;
	matrix[3][21][color] = 1;
	matrix[3][27][color] = 1;
	matrix[11][5][color] = 1;
	matrix[11][11][color] = 1;
	matrix[11][21][color] = 1;
	matrix[11][27][color] = 1;
	
	matrix[2][4][color] = 1;
	matrix[2][12][color] = 1;
	matrix[2][20][color] = 1;
	matrix[2][28][color] = 1;
	matrix[10][4][color] = 1;
	matrix[10][12][color] = 1;
	matrix[10][20][color] = 1;
	matrix[10][28][color] = 1;
	
	matrix[1][3][color] = 1;
	matrix[1][13][color] = 1;
	matrix[1][19][color] = 1;
	matrix[1][29][color] = 1;
	matrix[9][3][color] = 1;
	matrix[9][13][color] = 1;
	matrix[9][19][color] = 1;
	matrix[9][29][color] = 1;
}

void setAllRed()
{
	int i = 0;
	int j = 0;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 32; j++)
		{
			matrix[i][j][0] = 1;
		}
	}
}

void setAllGreen()
{
	int i = 0;
	int j = 0;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 32; j++)
		{
			matrix[i][j][1] = 1;
		}
	}	
}

void setAllBlue()
{
	int i = 0;
	int j = 0;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 32; j++)
		{
			matrix[i][j][2] = 1;
		}
	}	
}

void clearMatrix()
{
	int i = 0;
	int j = 0;
	int k = 0;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 32; j++)
		{
			for(k = 0; k < 3; k++)
			{
				matrix[i][j][k] = 0;
			}
		}
	}
}

/*	modifyMatrix
This function takes newly parsed JSON data
and modifies the output matrix state to match
*/
void modifyMatrix()
{
	int pixels[] = {0, 0, 0};
	if((color & COLOR_RED) == COLOR_RED)
		pixels[0] = 1;
	if((color & COLOR_GREEN) == COLOR_GREEN)
		pixels[1] = 1;
	if((color & COLOR_BLUE) == COLOR_BLUE)
		pixels[2] = 1;
	clearMatrix();
	int i;
	switch (graphic)
	{
		case DIRECTION_UP:
			for(i = 0; i < 3; i++)
			{
				if(pixels[i])
					setUp(pixels[i]);
			}
			break;
		case DIRECTION_DOWN:
			for(i = 0; i < 3; i++)
			{
				if(pixels[i])
					setDown(pixels[i]);
			}
			break;
		case DIRECTION_LEFT:
			for(i = 0; i < 3; i++)
			{
				if(pixels[i])
					setLeft(pixels[i]);
			}
			break;
		case DIRECTION_RIGHT:
			for(i = 0; i < 3; i++)
			{
				if(pixels[i])
					setRight(pixels[i]);
			}
			break;
		case DIRECTION_HERE:
			for(i = 0; i < 3; i++)
			{
				if(pixels[i])
					setHere(pixels[i]);
			}
			break;
	}
	currentTime = 0;
}

void parsePacket(cJSON * root)
{
	printf("hey\n");
	int msgVersion = cJSON_GetObjectItem(root, "Version")->valueint;
	if(msgVersion != version)
		return;
	msgType = cJSON_GetObjectItem(root, "MsgType")->valueint;
	if(msgType == SET_MSG)
	{
		color = cJSON_GetObjectItem(root, "Color")->valueint;
		graphic = cJSON_GetObjectItem(root, "Graphic")->valueint;
		duration = cJSON_GetObjectItem(root, "Duration")->valueint;
		//if(graphic == 2)
			//setUp(1);
		modifyMatrix();
	}
	gpio_set_level(23, 1);
	//write conditions for GET_MSG, RETURN_MSG here
	cJSON_Delete(root);
}


/**
 * @brief uart initialization
 */
static mdf_err_t uart_initialize()
{
    uart_config_t uart_config = {
        .baud_rate = CONFIG_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    MDF_ERROR_ASSERT(uart_param_config(CONFIG_UART_PORT_NUM, &uart_config));
    MDF_ERROR_ASSERT(uart_set_pin(CONFIG_UART_PORT_NUM, CONFIG_UART_TX_IO, CONFIG_UART_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    MDF_ERROR_ASSERT(uart_driver_install(CONFIG_UART_PORT_NUM, 2 * BUF_SIZE, 2 * BUF_SIZE, 0, NULL, 0));
    return MDF_OK;
}

static void uart_handle_task(void *arg)
{
    int recv_length   = 0;
    mdf_err_t ret     = MDF_OK;
    cJSON *json_root  = NULL;
    cJSON *json_addr  = NULL;
    cJSON *json_group = NULL;
    cJSON *json_data  = NULL;
    cJSON *json_dest_addr  = NULL;

    // Configure a temporary buffer for the incoming data
    uint8_t *data                     = (uint8_t *) MDF_MALLOC(BUF_SIZE);
    size_t size                       = MWIFI_PAYLOAD_LEN;
    char *jsonstring                  = NULL;
    uint8_t dest_addr[MWIFI_ADDR_LEN] = {0};
    mwifi_data_type_t data_type       = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN]   = {0};

    MDF_LOGI("Uart handle task is running");

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

    /* uart initialization */
    MDF_ERROR_ASSERT(uart_initialize());

    while (1) {
        memset(data, 0, BUF_SIZE);
        recv_length = uart_read_bytes(CONFIG_UART_PORT_NUM, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (recv_length <= 0) {
            continue;
        }

        ESP_LOGD("UART Recv data:", "%s", data);

        json_root = cJSON_Parse((char *)data);
        MDF_ERROR_CONTINUE(!json_root, "cJSON_Parse, data format error, data: %s", data);

        /**
         * @brief Check if it is a group address. If it is a group address, data_type.group = true.
         */
        json_addr = cJSON_GetObjectItem(json_root, "dest_addr");
        json_group = cJSON_GetObjectItem(json_root, "group");

        if (json_addr) {
            data_type.group = false;
            json_dest_addr = json_addr;
        } else if (json_group) {
            data_type.group = true;
            json_dest_addr = json_group;
        } else {
            MDF_LOGW("Address not found");
            cJSON_Delete(json_root);
            continue;
        }

        /**
         * @brief  Convert mac from string format to binary
         */
        do {
            uint32_t mac_data[MWIFI_ADDR_LEN] = {0};
            sscanf(json_dest_addr->valuestring, MACSTR,
                   mac_data, mac_data + 1, mac_data + 2,
                   mac_data + 3, mac_data + 4, mac_data + 5);

            for (int i = 0; i < MWIFI_ADDR_LEN; i++) {
                dest_addr[i] = mac_data[i];
            }
        } while (0);


        json_data = cJSON_GetObjectItem(json_root, "data");
        char *recv_data = cJSON_PrintUnformatted(json_data);

        size = asprintf(&jsonstring, "{\"src_addr\": \"" MACSTR "\", \"data\": %s}", MAC2STR(sta_mac), recv_data);
        ret = mwifi_write(dest_addr, &data_type, jsonstring, size, true);
        MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));

FREE_MEM:
        MDF_FREE(recv_data);
        MDF_FREE(jsonstring);
        cJSON_Delete(json_root);
    }

    MDF_LOGI("Uart handle task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

static void node_read_task(void *arg)
{
    mdf_err_t ret                    = MDF_OK;
    char *data                       = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size                      = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
	cJSON * recvData;

    MDF_LOGI("Node read task is running");

    for (;;) {
        if (!mwifi_is_connected() && !(mwifi_is_started() && esp_mesh_is_root())) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);

        /**
         * @brief Pre-allocated memory to data and size must be specified when passing in a level 1 pointer
         */
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
		
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_read", mdf_err_to_name(ret));
        MDF_LOGI("Node receive, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);
		recvData = cJSON_Parse(data);
		parsePacket(recvData);

        /* forwoad to uart */
       // uart_write_bytes(CONFIG_UART_PORT_NUM, data, size);
		//uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);
    }

    MDF_LOGW("Node read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/**
 * @brief printing system information
 */
static void print_system_info_timercb(void *timer)
{
    uint8_t primary                 = 0;
    wifi_second_chan_t second       = 0;
    mesh_addr_t parent_bssid        = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    mesh_assoc_t mesh_assoc         = {0x0};
    wifi_sta_list_t wifi_sta_list   = {0x0};

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    esp_wifi_get_channel(&primary, &second);
    esp_wifi_vnd_mesh_get(&mesh_assoc);
    esp_mesh_get_parent_bssid(&parent_bssid);

    MDF_LOGI("System information, channel: %d, layer: %d, self mac: " MACSTR ", parent bssid: " MACSTR
             ", parent rssi: %d, node num: %d, free heap: %u", primary,
             esp_mesh_get_layer(), MAC2STR(sta_mac), MAC2STR(parent_bssid.addr),
             mesh_assoc.rssi, esp_mesh_get_total_node_num(), esp_get_free_heap_size());

    for (int i = 0; i < wifi_sta_list.num; i++) {
        MDF_LOGI("Child mac: " MACSTR, MAC2STR(wifi_sta_list.sta[i].mac));
    }

#ifdef MEMORY_DEBUG

    if (!heap_caps_check_integrity_all(true)) {
        MDF_LOGE("At least one heap is corrupt");
    }

    mdf_mem_print_heap();
    mdf_mem_print_record();
    mdf_mem_print_task();
#endif /**< MEMORY_DEBUG */
}

static mdf_err_t wifi_init()
{
    mdf_err_t ret          = nvs_flash_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    MDF_ERROR_ASSERT(ret);

    tcpip_adapter_init();
    MDF_ERROR_ASSERT(esp_event_loop_init(NULL, NULL));
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg));
    MDF_ERROR_ASSERT(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_STA));
    MDF_ERROR_ASSERT(esp_wifi_set_ps(WIFI_PS_NONE));
    MDF_ERROR_ASSERT(esp_mesh_set_6m_rate(false));
    MDF_ERROR_ASSERT(esp_wifi_start());

    return MDF_OK;
}

/**
 * @brief All module events will be sent to this task in esp-mdf
 *
 * @Note:
 *     1. Do not block or lengthy operations in the callback function.
 *     2. Do not consume a lot of memory in the callback function.
 *        The task memory of the callback function is only 4KB.
 */
static mdf_err_t event_loop_cb(mdf_event_loop_t event, void *ctx)
{
    MDF_LOGI("event_loop_cb, event: %d", event);

    switch (event) {
        case MDF_EVENT_MWIFI_STARTED:
            MDF_LOGI("MESH is started");
            break;

        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("Parent is connected on station interface");
            break;

        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("Parent is disconnected on station interface");
            break;

        default:
            break;
    }

    return MDF_OK;
}

void setAddress(int i)
{
	if (i == 0)
		i = 7;
	else
		i -= 1;
	gpio_set_level(A, i & 1);
    gpio_set_level(B, i & 2);
    gpio_set_level(C, i & 4);
	//printf("address %d %d %d\n", i & 1, i & 2, i & 4);
	
}

/*
	Task to run GPIO m8rx
*/
void matrix_task(void *pvParameter)
{
	while(1) 
	{		
		int i,j;
		for(i=0;i<8;i++)
		{
			//vTaskDelay(500 / portTICK_PERIOD_MS);
			gpio_set_level(LATCH, 0);
			setAddress(i);
			
			for(j=0;j<32;j++)
			{
				gpio_set_level(CLK,0);
				gpio_set_level(R1, matrix[i][j][0]);
				gpio_set_level(G1, matrix[i][j][1]);
				gpio_set_level(B1, matrix[i][j][2]);
				gpio_set_level(R2, matrix[i + 8][j][0]);
				gpio_set_level(G2, matrix[i + 8][j][1]);
				gpio_set_level(B2, matrix[i + 8][j][2]);
				//vTaskDelay(5 / portTICK_PERIOD_MS);
				gpio_set_level(CLK, 1);
			    //vTaskDelay(5 / portTICK_PERIOD_MS);
				gpio_set_level(R1, 0);
				gpio_set_level(R2, 0);
				gpio_set_level(B1, 0);
				gpio_set_level(B2, 0);
				gpio_set_level(G1, 0);
				gpio_set_level(G2, 0);
				
			}
			//printf("%d\n", i);
			gpio_set_level(LATCH, 1);
			//vTaskDelay(500 / portTICK_PERIOD_MS);
		}
    }
}

void init_matrix()
{
	//configure each GPIO pin to operate as GPIO
	gpio_pad_select_gpio(CLK);
	gpio_pad_select_gpio(LATCH);
	gpio_pad_select_gpio(A);
	gpio_pad_select_gpio(B);
	gpio_pad_select_gpio(C);
	gpio_pad_select_gpio(R1);
	gpio_pad_select_gpio(R2);
	gpio_pad_select_gpio(B1);
	gpio_pad_select_gpio(B2);
	gpio_pad_select_gpio(G1);
	gpio_pad_select_gpio(G2);
	
    //Set each pin as output
    gpio_set_direction(CLK, GPIO_MODE_OUTPUT);
	gpio_set_direction(LATCH, GPIO_MODE_OUTPUT);
	gpio_set_direction(A, GPIO_MODE_OUTPUT);
	gpio_set_direction(B, GPIO_MODE_OUTPUT);
	gpio_set_direction(C, GPIO_MODE_OUTPUT);
	gpio_set_direction(R1, GPIO_MODE_OUTPUT);
	gpio_set_direction(R2, GPIO_MODE_OUTPUT);
	gpio_set_direction(B1, GPIO_MODE_OUTPUT);
	gpio_set_direction(B2, GPIO_MODE_OUTPUT);
	gpio_set_direction(G1, GPIO_MODE_OUTPUT);
	gpio_set_direction(G2, GPIO_MODE_OUTPUT);
	
	gpio_set_level(A, 0);
	gpio_set_level(B, 0);
	gpio_set_level(C, 0);
	gpio_set_level(R1, 0);
	gpio_set_level(R2, 0);
	gpio_set_level(B1, 0);
	gpio_set_level(B2, 0);
	gpio_set_level(G1, 0);
	gpio_set_level(G2, 0);

	clearMatrix();	
}

void app_main()
{
	init_bt();
	gpio_pad_select_gpio(23);
	gpio_set_direction(23, GPIO_MODE_OUTPUT);
	gpio_set_level(23, 0);
	gpio_pad_select_gpio(2);
	gpio_set_direction(2, GPIO_MODE_INPUT);
	gpio_pad_select_gpio(15);
	gpio_set_direction(15, GPIO_MODE_OUTPUT);
	
	
	int rootSel = 1;	//set as root
    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    mwifi_config_t config   = {
        .channel   = CONFIG_MESH_CHANNEL,
        .mesh_id   = CONFIG_MESH_ID,
        .mesh_type = CONFIG_DEVICE_TYPE,
    };
	if(rootSel == SELECT_ROOT)
		config.mesh_type = MWIFI_MESH_ROOT;
	else
		config.mesh_type = MWIFI_MESH_NODE;
	

    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());

    /**
     * @brief select/extend a group memebership here
     *      group id can be a custom address
     */
    const uint8_t group_id_list[2][6] = {{0x01, 0x00, 0x5e, 0xae, 0xae, 0xae},
                                         {0x01, 0x00, 0x5e, 0xae, 0xae, 0xaf}};

    MDF_ERROR_ASSERT(esp_mesh_set_group_id((mesh_addr_t *)group_id_list, 
                                sizeof(group_id_list)/sizeof(group_id_list[0])));

    /**
     * @brief Data transfer between wifi mesh devices
     */
    xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);


	init_matrix();
	//Task to drive LED Matrix. Tie to core 1 to avoid comm related latencies
	
	//setLeft(2);
	xTaskCreatePinnedToCore(&matrix_task, "matrix_task", 4096, NULL, 7, NULL, 1);
    /* Periodic print system information */
    TimerHandle_t timer = xTimerCreate("print_system_info", 10000 / portTICK_RATE_MS,
                                       true, NULL, print_system_info_timercb);
    xTimerStart(timer, 0);

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
	
	setAllGreen();
	setAllBlue();
	setAllRed();
	
    /**
     * @brief uart handle task:
     *  receive json format data,eg:`{"dest_addr":"30:ae:a4:80:4c:3c","data":"send data"}`
     *  forward data item to destination address in mesh network
     */
    //xTaskCreate(uart_handle_task, "uart_handle_task", 4 * 1024,
               // NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
	while(1)
	{
		vTaskDelay(100/portTICK_PERIOD_MS);
		size_t size                      = MWIFI_PAYLOAD_LEN;
		uint8_t dest_addr[MWIFI_ADDR_LEN] = {0};
		mwifi_data_type_t data_type       = {0};
		mdf_err_t ret                    = MDF_OK;
		if(gpio_get_level(2) == 1)
		{	
			vTaskDelay(80/portTICK_PERIOD_MS);
			gpio_set_level(15, 1);
			
			//create a packet to set a left arrow
			cJSON *root = cJSON_CreateObject();
			cJSON_AddNumberToObject(root, "Version", 0);
			cJSON_AddNumberToObject(root, "MsgType", SET_MSG);
			cJSON_AddNumberToObject(root, "Graphic", DIRECTION_UP);
			cJSON_AddNumberToObject(root, "Color", COLOR_BLUE);
			cJSON_AddNumberToObject(root, "Duration", 5);
			
			char * payload = cJSON_Print(root);
			do {
				uint32_t mac_data[MWIFI_ADDR_LEN] = {0};
				sscanf(node_mac, MACSTR,
					   mac_data, mac_data + 1, mac_data + 2,
					   mac_data + 3, mac_data + 4, mac_data + 5);

				for (int i = 0; i < MWIFI_ADDR_LEN; i++) {
					dest_addr[i] = mac_data[i];
				}
			} while (0);

			size = 1024;
			//size = asprintf(&jsonstring, "{\"src_addr\": \"" MACSTR "\", \"data\": %s}", MAC2STR(sta_mac), recv_data);
			ret = mwifi_write(dest_addr, &data_type, payload, size, true);
			MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));
			gpio_set_level(15, 0);
	FREE_MEM:
        MDF_FREE(payload);
        cJSON_Delete(root);
			//printf("sent payload: %s\n", payload);
		}
			
	}
}
