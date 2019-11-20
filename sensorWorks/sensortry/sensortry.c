/*
 * chocolate_cannon.c
 *
 * Created: 5/14/2017 9:44:27 PM
 *  Author: ASUS
 */ 


//#include <avr/io.h>
#include <math.h>
#include "mpu6050_twi.h"
#include <avr/io.h>
//#include "uart.c"
#include "util/delay.h"
#include <string.h>
#include <stdio.h>

#ifndef F_CPU
#define F_CPU 8000000 // 16 MHz clock speed
#endif
#define rightDir 1
#define leftDir 2
#define upDir 3
#define downDir 4
#define B1R1 0
#define G1G2 1
#define B2R2 2
#define red 1
#define blue 0
#define slowerIteration 300.0
#define fasterIteration 120.0
#define delayVal 0.0
#define levelUpDelay 200
#define g 9.8
#define blockSize 4
//#define blockNo 2

#define D0 eS_PORTD0
#define D1 eS_PORTD1
#define D2 eS_PORTD2
#define D3 eS_PORTD3
#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7
#include "lcd.h"


float Acc_x,Acc_y,Acc_z,Temperature,Gyro_x,Gyro_y,Gyro_z;
//float Acc_x,Acc_y,Acc_z,Temperature,Gyro_x,Gyro_y,Gyro_z;
int sss,status,pos,wm_sz,snake[128],moveDir,seed,whichMatrix;
float Xa,Ya,Za,t;
float Xg=0,Yg=0,Zg=0;

int block11[4] = {18,26,34,42};
int block12[4] = {23,24,81,82};
int block13[4] = {87,95,103,111};
int block14[4] = {151,152,209,210};
int block15[4] = {179,180,181,182};
int block16[4] = {243,244,245,246};
int block17[4] = {207,215,223,231};
int block18[4] = {138,146,154,162};

int score = 0;
int level = -1;
float iteration = 0;

char *scoreStr;
char scr[6];
//unsigned char columnVal[8]={0b01111111,0b11111110,0b11111101,0b11111011,0b11110111,0b11101111,0b11011111,0b10111111};
//unsigned char columnVal[8]={0b00111000,0b00000000,0b00001000,0b00010000,0b00011000,0b00100000,0b00101000,0b00110000};
//unsigned char rowValue[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned char tempVal = 0b00000000;
int horizontalHeads[5]={-1,-1,-1,-1,-1},verticalHeads[5]={-1,-1,-1,-1,-1};
int horizontalBlocks[2][10], verticalBlocks[10][10];

int thirdLevelIt();
int fourthLevelIt();
void showScreen();

int intLen(float x)
{
	int temp = (int)x,rem,len=0;
	while(temp)
	{
		len++;
		temp /= 10;
	}
	
	return len;
	
}

void intToStr(int x,char ret[])
{
	int len = intLen(x);

	//while()
	//x *=10000;
	//printf("%f\n",x);

	int val = (int)x,rem,i=0,j;

	//char ret[6];
	if(!val)
	{
		ret[0] = '0';
		ret[1] = '\0';
		return;	
	}
	
	
	
	while(val)
	{
		rem = val % 10;
		ret[i++] = (rem + '0');
		//printf("%c\n",ret[i-1]);
		val /= 10;
	}

	for( i=0,j=len-1;i<j;i++,j--)
	{
		char temp = ret[i];
		ret[i] = ret[j];
		ret[j] = temp;
	}
	
	//ret[len] = '.';
	//ret[5] = '\0';



	//return ret;
}


int rowCount(int i)
{
	int ret;
	ret=((int)floor(i/8.0))%8;
	if(i%8==0)
	ret=(ret+7)%8;
	
	return ret;
}

int columnNo(int i)
{
	int ret = ((i%8)+7)%8;
	
	ret &= ((~1<<3)|7);
	
	return ret;
}

int ifSnakePresent(int x)
{
	
	for(int j=0;j<wm_sz;j++)
	{
		if(snake[j] == x)
		return 1;
	}
	
	return 0;
}

int ifObstaclePresent(int x)
{
	if(level == 3)
	{
		if(ifBlockPresent(block11,x) || ifBlockPresent(block12,x) || ifBlockPresent(block13,x) || ifBlockPresent(block14,x)|| ifBlockPresent(block15,x)|| ifBlockPresent(block16,x))
		return 1;
	}
	
	else if(level == 4)
	{
		if(ifBlockPresent(block11,x) || ifBlockPresent(block12,x) || ifBlockPresent(block13,x) || ifBlockPresent(block14,x)|| ifBlockPresent(block15,x)|| ifBlockPresent(block16,x)|| ifBlockPresent(block17,x)|| ifBlockPresent(block18,x))
		return 1;
	}
}

int ifBlockPresent(int arra[],int x)
{
	for(int j=0;j<blockSize;j++)
	{
		if(arra[j] == x)
		return 1;
	}
	
	return 0;
}



int ifFoodPresent(int x)
{
	if(seed==x)
	return 1;
	
	return 0;
}


void Die()
{
	PORTA = 0x00;
	PORTD = 0xFF;
	PORTB = 0XFF;
	Lcd8_Set_Cursor(1,1);
	Lcd8_Write_String("    ");
	
	Lcd8_Set_Cursor(1,5);
	Lcd8_Write_String("Game over!");
	_delay_ms(1000);
	Lcd8_Clear();
	//PORTB = 0x00;
	
	score = 0;
	level = -1;
	initialize();
}

void showBlock(int arra[])
{
	for(int j=0;j<blockSize;j++)
	{
		int row = rowCount(arra[j]);
		int col = columnNo(arra[j]);
		int matrixNo = (int) ceil(arra[j]/64.0);
		
		unsigned char rowVal = tempVal;
		unsigned char colVal = tempVal;
		
		//setting up column
		if(matrixNo==1 || matrixNo==3)
		colVal |= 0;
		
		else
		colVal |= 2;
		
		colVal |= (col<<2);
		colVal |= (red<<5);
		
		//setting up row
		if(matrixNo ==3 || matrixNo == 4)
		rowVal |= 1;
		
		else
		rowVal |= 0;
		
		rowVal |= (row<<2);
		
		PORTA = rowVal;
		PORTB = colVal;
	}
}

void showThirdLevel()
{
	showBlock(block11);
	showBlock(block12);
	showBlock(block13);
	showBlock(block14);
	showBlock(block15);
	showBlock(block16);
	//_delay_ms(delayVal);
}

void showFourthLevel()
{
	showThirdLevel();
	showBlock(block17);
	showBlock(block18);
	//_delay_ms(delayVal);
	
}

void initialize(){
	//setting up level
	srand(time(0));
	
	if(level == -1){
		//srand(time(0));
	DDRA = 0xFF;
	DDRC = 0xFF;
	DDRB = 0xFF;
	DDRD = 0xFF;
	level = 1;
	Lcd8_Init();
	Lcd8_Set_Cursor(1,1);
	showScreen();
	}
	
	else
	{
		level++;
		Lcd8_Set_Cursor(1,8);
		intToStr(level,scr);
		Lcd8_Write_String(scr);
		Lcd8_Set_Cursor(2,8);
		Lcd8_Write_String("0");	
	}
	
	
	PORTA = 0x00;
	PORTB = 0xFF;
	//PORTB = 0x00;
	
	//setting up speed
	if(level == 1)
	iteration = slowerIteration;
	else if (level < 5)
	{
		iteration = fasterIteration;
				
	}
		
	
	else
	Die();
	
	//for a new level, reset the score
	score = 0;
	
	int i;
	sss=0;
	seed = (rand()%256)+1;
	if(seed==256) seed--;
	status=1;
	pos=2;
	whichMatrix = 0; //start from 0th matrix
	wm_sz=3;
	moveDir = rightDir;
	for(i=0;i<64;i++){
		snake[i]=0;
	}
	
	
	
	
	I2C_Init();		// Initialize I2C
	Gyro_Init();		// Initialize Gyro */
	
}





void showBlocks(int i)
{
	//showing  blocks
	
	//from lsb to msb(columnVal)
	//first two bits denote the decoder number
	//next four bits denote column number
	
	//from lsb to msb(rowVal)
	//first two bits denote which matrix(and which decoder we are gonna activate)
	//next three bits denote row number	
	

	//PORTA=0xFF;
	//PORTD=0x00;

	unsigned color=tempVal, rowSelect=tempVal;
	int whichMatrix2 = (int)ceil(i/64.0);
	
	//if(whichMatrix2)
	//whichMatrix2--;
	
	//setting the color of the dot
	color |= (red<<5); 
	//snakeColor = snakeColor ^ 1;
	
	//if the block is in the 0th or 2nd matrix(decode number)
	if((i>=1 && i<=64) || (i>=129 && i<=192))
	color |= 0;
	
	else
	color |= 2;
	
	//setting up the column number
	color |= (columnNo(i)<<2);
	
	
	//setting up row value
	
	//decoder no
	if(whichMatrix2==1 || whichMatrix2==2)
	rowSelect |= 0;
	
	else
	rowSelect |= 1;
	
	//row no
	
	rowSelect |= (rowCount(i)<<2);
	
	PORTA = rowSelect;
	PORTD = color;
	
	

	

	
} 



void position(int i){

	/*
	from lsb to msb(columnVal)
	first two bits denote the decoder number
	next four bits denote column number
	
	from lsb to msb(rowVal)
	first two bits denote which matrix(and which decoder we are gonna activate)
	next three bits denote row number	
	*/

	//PORTA=0xFF;
	//PORTD=0x00;

	unsigned color=tempVal, rowSelect=tempVal;
	int whichMatrix2 = (int)ceil(snake[i]/64.0);
	
	/*if(whichMatrix2)
	whichMatrix2--;*/
	
	//setting the color of the dot
	if(i%2)
	color |= (blue<<5);
	
	else
	color |= (red<<5);
	//snakeColor = snakeColor ^ 1;
	
	//if the snake is in the 0th or 2nd matrix(decode number)
	if((snake[i]>=1 && snake[i]<=64) || (snake[i]>=129 && snake[i]<=192))
	color |= 0;
	
	else
	color |= 2;
	
	//setting up the column number
	//int columnNo = ((snake[i])%8+7)%8;
	
	//columnNo &= ((~1<<3)|7);
	
	color |= (columnNo(snake[i])<<2);
	
	
	//setting up row value
	
	//decoder no
	if(whichMatrix2==1 || whichMatrix2==2)
	rowSelect |= 0;
	
	else
	rowSelect |= 1;
	
	//row no
	
	rowSelect |= (rowCount(snake[i])<<2);
	
	PORTA = rowSelect;
	PORTB = color;
	
	

	//_delay_ms(delayVal);

}

void seedPosition(int i)
{
	//PORTA=0xFF; // this is for row selection
	//PORTD=0x00; //this is for color columns

	int row = rowCount(i);
	int col = columnNo(i);
	int matrixNo = (int)ceil(i/64.0);
	
	//assigning green color
	unsigned char columnVal = tempVal | 1;
	unsigned char rowVal = tempVal;
	
	//setting up decoder
	if(matrixNo == 2 || matrixNo == 4)
	columnVal |= (1<<5);
	
	//setting up column no
	columnVal |= (col<<2); 
	
	//choosing decoder
	if(matrixNo == 3 || matrixNo == 4)
	rowVal |= 1;
	
	//choosing row no
	rowVal |= (row<<2); 
	
	PORTA = rowVal;
	PORTB = columnVal;
	
	
	
}

void seedChange()
{
	//score=0;
	if(seed==pos)
	{	
		
		wm_sz++;
		score++;
		
		Lcd8_Set_Cursor(2,8);
		intToStr(score,scr);
		Lcd8_Write_String(scr);
		
		PORTA |= 0b10000000;
		_delay_ms(1);
		PORTA &= 0b01111111;
		
		int timervalue;
		do{
		timervalue=(rand()%256)+1;
		
		}while(ifSnakePresent(timervalue) || (ifObstaclePresent(timervalue) && level>2)); 
		
		seed=timervalue;
		
		if(seed == 255 && level==2)
		seed=seed-1;
		
		
		
		//int row = rowCount(i);
		int col = columnNo(seed);
		int matrixNo = (int)ceil(seed/64.0);
		if((matrixNo == 2 || matrixNo == 4) && col==0)
		{
			seed+= ((rand()%4)+1);
		}
		
		
		if(score == 3){
		PORTA = 0x00;
		PORTD = 0xff;
		_delay_ms(levelUpDelay);
		initialize();
		}
		
		/*if(score==1)
		{
			data1update();
		}*/
		//i=0;
	}
}


void shift()
{
	int i;

	for(i=wm_sz-1;i>0;i--)
	{
		snake[i]=snake[i-1];
	}
	snake[0]=pos;
	
	if( level > 2 && ifObstaclePresent(snake[0]))
	{
		Die();	
	}
	
	for(i=2;i<wm_sz;i++)
	{
		if(snake[i]==pos)
		{
			Die();
			
		}
	}
}


void right(){
	sss=1;
	int i,u/*,off*/;

		if(pos%8==0)  //CHECK WHETHER POSITION IS RIGHT EXTREMITY
		{

			//if head is at the end of 0th or 2nd matrix
			if(!whichMatrix || whichMatrix == 2)
			pos= pos + 57;

			//head is at the edge of 1st or 3rd matrix
			else
			pos = pos - 71;


			//matrix 2 to 3 or vice versa
			//matrix 0 to 1 or vice versa
			whichMatrix ^= 1;

			seedChange();

			shift();
			
			//300 iterations of the loop and 30ms total delay is ideal
			int it;
			if(level<3)
			it = (int)round(iteration/wm_sz);
			
			else if(level == 3)
			it = thirdLevelIt();
			
			else
			it = fourthLevelIt();
			
			
			
			//float delay = 30.0/it;
			for(u=0;u<it;u++)
			{
				for(i=0;i<wm_sz;i++)
				{
					position(i);
					//_delay_ms(delayVal/wm_sz);
					//seedPosition(seed);
				}
				//float delay = .3;
				//_delay_ms(delayVal);
				seedPosition(seed);
				
				if(level==3)
				showThirdLevel();
				
				else if(level == 4)
				showFourthLevel();
			}
			/*for(off=0;off<=15;off++)
			{
				//offstate();
			}*/
			//_delay_ms(1);
		}

		else
		{
			pos=pos+1;
			seedChange();
			shift();
			
			int it;
			if(level<3)
			it = (int)round(iteration/wm_sz);
			
			else if(level == 3)
			it = thirdLevelIt();
			
			else
			it = fourthLevelIt();
			
			for(u=0;u<it;u++)
			{
				for(i=0;i<wm_sz;i++)
				{
					position(i);
					//_delay_ms(100);
					//seedPosition(seed);
				}
				
				//_delay_ms(delayVal);
				seedPosition(seed);
				if(level==3)
				showThirdLevel();
				
				else if(level == 4)
				showFourthLevel();
			}
			//for(off=0;off<=15;off++)
			//{
				//offstate();
			//}
			//_delay_ms(1);
		}
		status=1;
		//init_interrupts();
}

void left()
{
	sss=1;
	int i,u/*,off*/;

		if(((pos+7)%8)==0)//CHECK WHETHER POSITION IS LEFT EXTREMITY

		{
			//if head at the edge of the 0th or the 2nd matrix
			if(!whichMatrix || whichMatrix == 2)
			pos = pos + 71;

			//if head is at the edge of the 1st or 3rd matrix
			else
			pos=pos-57;

			//matrix 2 to 3 or vice versa
			//matrix 0 to 1 or vice versa
			whichMatrix ^= 1;

			seedChange();
			shift();
			int it;
			
			if(level<3)
			it = (int)round(iteration/wm_sz);
			
			else if(level == 3)
			it = thirdLevelIt();
			
			else
			it = fourthLevelIt();
			
			for(u=0;u<it;u++)
			{
				for(i=0;i<wm_sz;i++)
				{
					position(i);
					//seedPosition(seed);
				}
				//_delay_ms(delayVal);
				seedPosition(seed);
				if(level==3)
				showThirdLevel();
				
				else if(level == 4)
				showFourthLevel();
			}
			/*for(off=0;off<=15;off++)
			{
				offstate();
			}*/
			//_delay_ms(1);
		}
		else
		{
			pos=pos-1;
			seedChange();
			shift();
			int it;
			
			if(level<3)
			it = (int)round(iteration/wm_sz);
			
			else if(level == 3)
			it = thirdLevelIt();
			
			else
			it = fourthLevelIt();
			
			for(u=0;u<it;u++)
			{
				for(i=0;i<wm_sz;i++)
				{
					position(i);
					//seedPosition(seed);
				}
				//_delay_ms(delayVal);
				seedPosition(seed);
				if(level==3)
				showThirdLevel();
				
				else if(level == 4)
				showFourthLevel();
			}
			/*for(off=0;off<=15;off++)
			{
				offstate();
			}*/
			//_delay_ms(1);
		}
		status=3;
		//init_interrupts();

}

/*void no_inp(){
	if(status==1) right();
}*/
void up()
{
	sss=1;
	int i,u/*,off*/;
	int rowNo = rowCount(pos);
	
	if(rowNo == 0)//CHECK WHETHER POSITION IS UP EXTREMITY

	{
		if((pos>=129 && pos<=136) || (pos>=193 && pos<=200))
		pos=pos-72;
		
		else
		pos = pos + 184;
		
		seedChange();
		shift();
		
		int it;
		if(level<3)
		it = (int)round(iteration/wm_sz);
		
		else if(level == 3)
		it = thirdLevelIt();
		
		else
		it = fourthLevelIt();
		
		for(u=0;u<it;u++)
		{
			for(i=0;i<wm_sz;i++)
			{
				position(i);
				//seedPosition(seed);
			}
			//_delay_ms(delayVal);
			seedPosition(seed);
			if(level==3)
			showThirdLevel();
			
			else if(level == 4)
			showFourthLevel();
		}
		/*for(off=0;off<=15;off++)
		{
			offstate();
		}*/
		//_delay_ms(1);
	}
	else
	{
		pos=pos-8;
		seedChange();
		shift();
		
		int it;
		if(level<3)
		it = (int)round(iteration/wm_sz);
		
		else if(level == 3)
		it = thirdLevelIt();
		
		else
		it = fourthLevelIt();
		
		for(u=0;u<it;u++)
		{
			for(i=0;i<wm_sz;i++)
			{
				position(i);
				//seedPosition(seed);
			}
			//_delay_ms(delayVal);
			seedPosition(seed);
			if(level==3)
			showThirdLevel();
			
			else if(level == 4)
			showFourthLevel();
		}
		/*for(off=0;off<=15;off++)
		{
			offstate();
		}*/
		//_delay_ms(1);
	}
	status=2;
	//init_interrupts();

}

void down()
{
	sss=1;
	int i,u/*,off*/;
	int rowNo = rowCount(pos);
	
		if(rowNo==7 )//CHECK WHETHER POSITION IS DOWN EXTREMITY

		{
			
			if((pos>=57 && pos<=64) || (pos>=121 && pos<=128))
			pos=pos+72;

			else
			pos = pos - 184;
			
			seedChange();
			shift();
			
			int it;
			if(level<3)
			it = (int)round(iteration/wm_sz);
			
			else if(level == 3)
			it = thirdLevelIt();
			
			else
			it = fourthLevelIt();
			
			for(u=0;u<it;u++)
			{
				for(i=0;i<wm_sz;i++)
				{
					position(i);
				//	seedPosition(seed);
				}
				//_delay_ms(delayVal);
				seedPosition(seed);
				if(level==3)
				showThirdLevel();
				
				else if(level == 4)
				showFourthLevel();

			}
			//_delay_ms(1);
		}
		else
		{
			pos=pos+8;
			seedChange();
			shift();
			
			int it;
			if(level<3)
			it = (int)round(iteration/wm_sz);
			
			else if(level == 3)
			it = thirdLevelIt();
			
			else
			it = fourthLevelIt();
			
			for(u=0;u<it;u++)
			{
				for(i=0;i<wm_sz;i++)
				{
					position(i);
					//seedPosition(seed);
				}
				//_delay_ms(delayVal);
				seedPosition(seed);
				if(level==3)
				showThirdLevel();
				
				else if(level == 4)
				showFourthLevel();
			}
			/*for(off=0;off<=15;off++)
			{
				offstate();
			}*/
			//_delay_ms(1);
		}
		status=4;
		//init_interrupts();

}





void mover()
{
	Read_RawValue(&Acc_x, &Acc_y, &Acc_z, &Temperature, &Gyro_x, &Gyro_y, &Gyro_z);

	// Divide raw value by sensitivity scale factor
	Xa = Acc_x/16384.0;
	Ya = Acc_y/16384.0;
	//Za = Acc_z/16384.0;

	Xa=Xa*9.8;
	Ya=Ya*9.8;
	//Za *=g;
	

	Xg = Gyro_x/16.4;
	Yg = Gyro_y/16.4;
	Zg = Gyro_z/16.4; 
	
	
	if(Ya<-3.0 && moveDir != rightDir)
	moveDir = leftDir;

	else if (Ya>3.0 && moveDir != leftDir)
	moveDir = rightDir;

	else if(Xa>3.0 && moveDir != downDir)
	moveDir = upDir;

	else if(Xa<-3.0 && moveDir != upDir)
	moveDir = downDir;

	if(moveDir == rightDir)
		right();

	else if(moveDir == leftDir)
		left();

	else if(moveDir == upDir)
	up();

	else if(moveDir == downDir)
	down();
	
	//Xa=0.0;
	//Ya=0.0;

	//_delay_ms(100);
}



int main()
{
//	srand(time(0));
	//DDRD=0xFF;
	//DDRC=0xFF;
	//Lcd8_Init();
	//Lcd8_Set_Cursor(1,1);
	//Lcd8_Write_String("Game On");
	//_delay_ms(2000);
	
	initialize();
	//Lcd8_Init();
	//Lcd8_Set_Cursor(1,1);
	
	//right();
	while(1){
		
		
		//Lcd8_Write_String("Game On");
		//_delay_ms(2000);
		
		mover();
		//showAllBlocks();
	}

	return 0;

}

int thirdLevelIt()
{
	return ((fasterIteration*3+fasterIteration*delayVal)/(3+delayVal+24*2.5));
}

int fourthLevelIt()
{
	return ((fasterIteration*3+fasterIteration*delayVal)/(3+delayVal+36*2));
}

void showScreen()
{
	Lcd8_Write_String("Game On");
	_delay_ms(500);
	Lcd8_Clear();
	Lcd8_Set_Cursor(1,1);
	Lcd8_Write_String("Level: ");
	Lcd8_Set_Cursor(1,8);
	intToStr(level,scr);
	Lcd8_Write_String(scr);
	Lcd8_Set_Cursor(2,1);
	Lcd8_Write_String("Score: ");
	Lcd8_Set_Cursor(2,8);
	intToStr(score,scr);
	Lcd8_Write_String(scr);
}