
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_task_wdt.h"

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

/*
	Shifts current values of R1, B1, G1, R2, B2, G2 into the
	shift registers pointed to by current A, B, C address
	Values shifted in every 10 ms = 320 ms to shift in
*/

uint8_t matrix[16][32][3];

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
	/*
	gpio_set_level(A, 0);
	gpio_set_level(B, 1);
	gpio_set_level(C, 1);
	printf("hey\n");
	*/
    
	gpio_set_level(A, i & 1);
    gpio_set_level(B, i & 2);
    gpio_set_level(C, i & 4);
	//printf("address %d\n", i);
	
}

void shiftRow()
{	
	int i = 0;
	for(i = 0; i < 32; i++)
	{
		gpio_set_level(CLK, 0);
		gpio_set_level(R1, 1);
		vTaskDelay(5 / portTICK_PERIOD_MS);
		gpio_set_level(CLK, 1);
		vTaskDelay(5 / portTICK_PERIOD_MS);
		gpio_set_level(R1, 0);
	}
}

/*
	Task to run GPIO m8rx
*/
void blink_task(void *pvParameter)
{
    while(1) 
	{
		/*
		//Set addresss to 0, shift in current vals, latch for 1/2 second
		gpio_set_level(LATCH, 0);
		gpio_set_level(A, 0);
		gpio_set_level(B, 0);
		gpio_set_level(C, 0);
		shiftRow();
		gpio_set_level(LATCH, 1);
		vTaskDelay(500 / portTICK_PERIOD_MS);
		gpio_set_level(LATCH, 0);
		
		//set address to 1, shift in current vals, latch for 1/2 second
		gpio_set_level(A, 1);
		shiftRow();
		gpio_set_level(LATCH, 1);
		vTaskDelay(500 / portTICK_PERIOD_MS);
		*/

		int i,j;
		for(i=0;i<8;i++)
		{
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
			}
			gpio_set_level(LATCH, 1);
			vTaskDelay(5 / portTICK_PERIOD_MS);
		}
		
    }
}

void app_main()
{
	//configure each GPIO pin to operate as GPIO
	gpio_pad_select_gpio(OE);
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
	gpio_set_direction(OE, GPIO_MODE_OUTPUT);
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
	gpio_set_direction(G1, GPIO_MODE_OUTPUT);
	
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
	gpio_set_level(OE, 0);
	//gpio_set_level(R1, 1);
	clearMatrix();
	setAllRed();
	//Create task to run the matrix
    xTaskCreate(&blink_task, "blink_task", 4096, NULL, 5, NULL);
	esp_task_wdt_deinit();
}