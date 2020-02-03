
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
		matrix[4][4][color] = 1;
		matrix[4][7][color] = 1;	
		matrix[4][9][color] = 1;
		matrix[4][14][color] = 1;
		matrix[4][17][color] = 1;
		matrix[4][19][color] = 1;
		matrix[4][24][color] = 1;
		matrix[4][25][color] = 1;
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
	for(int i = 9; i<12; i++){
		matrix[9][i][color] = 1;
	}
	matrix[9][14][color] = 1;
	matrix[9][16][color] = 1;
	matrix[9][19][color] = 1;
	for(int i = 24; i<26; i++){
		matrix[9][i][color] = 1;
	}
	
	
	matrix[10][4][color] = 1;
	matrix[10][7][color] = 1;
	for(int i = 9; i<12; i++){
		matrix[10][i][color] = 1;
	}
	matrix[10][14][color] = 1;
	matrix[10][17][color] = 1;
	matrix[10][19][color] = 1;
	
	
	matrix[11][4][color] = 1;
	matrix[11][7][color] = 1;
	for(int i = 9; i<12; i++){
		matrix[11][i][color] = 1;
	}
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
	gpio_set_level(OE, 0);
	clearMatrix();
	setHere(2);
	setHere(0);
	printGreen();
	//Create task to run the matrix
    xTaskCreate(&blink_task, "blink_task", 4096, NULL, 5, NULL);
	esp_task_wdt_deinit();
}