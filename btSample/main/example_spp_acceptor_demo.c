
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"


#define I2CSDA 18 
#define I2CSCL 19
#define LUX_SENSOR_ADDR 0x10

#define TESTPIN 23
#define DUTY50 4095
#define DUTY0 0

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

#define NODE_ID 1
#define SPP_TAG "SPP_ACCEPTOR_DEMO"
#define SPP_SERVER_NAME "SPP_SERVER"
#define EXAMPLE_DEVICE_NAME "Emergency Light Node 1"

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static struct timeval time_new, time_old;
static long data_num = 0;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

void clearMatrix(void);
void setAllRed(void);
void setAllGreen(void);
void setAllBlue(void);
void setUp(uint8_t);
void setDown(uint8_t);
void setRight(uint8_t);
void setLeft(uint8_t);
void setHere(uint8_t);

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

/*
	Shifts current values of R1, B1, G1, R2, B2, G2 into the
	shift registers pointed to by current A, B, C address
	Values shifted in every 10 ms = 320 ms to shift in
*/

uint8_t matrix[16][32][3];

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


void printGreen()
{
	int i, j;
	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 32; j++)
		{
			printf("%d ", matrix[i][j][1]);
		}
		printf("\n");
	}
}

/*
	Task to run GPIO m8rx
*/
void blink_task(void *pvParameter)
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
				vTaskDelay(5 / portTICK_PERIOD_MS);
				gpio_set_level(CLK, 1);
			    vTaskDelay(5 / portTICK_PERIOD_MS);
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

void app_main()
{	
	//OE Dimmer PWM config
	ledc_timer_config_t pwmConfig;
	pwmConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
	pwmConfig.duty_resolution = LEDC_TIMER_13_BIT;
	pwmConfig.timer_num = LEDC_TIMER_0;
	pwmConfig.freq_hz = 5000;
	
	ledc_channel_config_t OEConfig;
	OEConfig.gpio_num = OE;
	OEConfig.speed_mode = LEDC_HIGH_SPEED_MODE;
	OEConfig.channel = LEDC_CHANNEL_0;
	OEConfig.intr_type = LEDC_INTR_DISABLE;
	OEConfig.timer_sel = LEDC_TIMER_0;
	OEConfig.duty = 0;
	
	ledc_timer_config(&pwmConfig);
	ledc_channel_config(&OEConfig);

	//I2C peripheral config
	i2c_config_t luxConfig;
	luxConfig.mode = I2C_MODE_MASTER;
	luxConfig.sda_io_num = I2CSDA;
	luxConfig.sda_pullup_en = GPIO_PULLUP_ENABLE;
	luxConfig.scl_io_num = I2CSCL;
	luxConfig.scl_pullup_en = GPIO_PULLUP_ENABLE;
	luxConfig.master.clk_speed = 100000;
	i2c_param_config(0, &luxConfig);
	i2c_driver_install(0, I2C_MODE_MASTER, 0, 0, 0);
	
	//configure lux sensor settings
	i2c_cmd_handle_t luxCmd = i2c_cmd_link_create();
	i2c_master_start(luxCmd);	//Start bit
	i2c_master_write_byte(luxCmd, (LUX_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);	//Address
	i2c_master_write_byte(luxCmd, 0x00, true);		//Command: Write Config
	i2c_master_write_byte(luxCmd, 0b00000000, true); //Write Config Properties: 1 gain, 800 ms int time, etc.
	i2c_master_write_byte(luxCmd, 0b11000000, true);
	i2c_master_stop(luxCmd);	//stop bit
	i2c_master_cmd_begin(0, luxCmd, 1000 / portTICK_RATE_MS);	//all 1 second to send
	i2c_cmd_link_delete(luxCmd);
	
	luxCmd = i2c_cmd_link_create();
	i2c_master_start(luxCmd);	//Start bit
	i2c_master_write_byte(luxCmd, (LUX_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);	//Address
	i2c_master_write_byte(luxCmd, 0x03, true);		//Command: PowerSave
	i2c_master_write_byte(luxCmd, 0b00000000, true); //Write Config Properties: Enable power save
	i2c_master_write_byte(luxCmd, 0b00000000, true);
	i2c_master_stop(luxCmd);	//stop bit
	i2c_master_cmd_begin(0, luxCmd, 1000 / portTICK_RATE_MS);	//all 1 second to send
	i2c_cmd_link_delete(luxCmd);
	
	
	
	
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

	
	//configure each GPIO pin to operate as GPIO
	//gpio_pad_select_gpio(OE);
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
	//gpio_set_direction(OE, GPIO_MODE_OUTPUT);
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
	//for now, set OE as low indefinitely
	//gpio_set_level(OE, 0);
	clearMatrix();
	setHere(2);
	setHere(0);
	
	//disable task watchdog (temporary)
	esp_task_wdt_deinit();
	
	//Task to drive LED Matrix. Tie to core 1 to avoid comm related latencies
	xTaskCreatePinnedToCore(&blink_task, "blink_task", 4096, NULL, 5, NULL, 1);

	//idle task test: Read lux sensor once every second, print to console
	uint8_t als_MSB;
	uint8_t als_LSB;
	uint16_t alsVal;
	uint32_t luxVal;
	while(1)
	{
		luxCmd = i2c_cmd_link_create();
		i2c_master_start(luxCmd);	//Start bit
		i2c_master_write_byte(luxCmd, (LUX_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);	//Address
		i2c_master_write_byte(luxCmd, 0x04, true);		//Command: Read ALS (Ambient Light)
		i2c_master_start(luxCmd);		//Start bit 
		i2c_master_write_byte(luxCmd, (LUX_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);	//specify a read from sensor
		i2c_master_read_byte(luxCmd, &als_LSB, 0);	//Read LSB
		i2c_master_read_byte(luxCmd, &als_MSB, 1);	//Read MSB, NAK
		i2c_master_stop(luxCmd);	//stop bit
		i2c_master_cmd_begin(0, luxCmd, 1000 / portTICK_RATE_MS);	//allow 1 second to send + read
		i2c_cmd_link_delete(luxCmd);
		
		alsVal = (als_MSB << 8) | als_LSB;
		luxVal = 0.0576 * alsVal;		//Constant retrieved from datasheet to convert ALS to lux
		//printf("ALS Val: %d       Lux Val: %d\n", alsVal, luxVal);
		
		//normal light conditions
		if(luxVal >= 100 && luxVal < 250)
		{
			ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, (1 << 13) * 0.5);
			ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
		}
		//Bright light conditions
		else if (luxVal >= 250)
		{
			ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
			ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
		}
		//Low light conditions
		else
		{
			ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, (1 << 13) * 0.75);
			ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
		}
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}