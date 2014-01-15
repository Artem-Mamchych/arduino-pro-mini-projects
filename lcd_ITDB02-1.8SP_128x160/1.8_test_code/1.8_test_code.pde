/*************************
For 1.8inch SPI TFT LCD
*************************/

#define SCL 6
#define SDA 7
#define RS 8
#define WR 9
#define CS 10
#define RST 11


void delay1(unsigned int time)
{
 unsigned int i,j;
  for(i=0;i<time;i++)
   for(j=0;j<250;j++)
      asm("nop"::);
}




void LCD_CtrlWrite_IC(unsigned char c)
{
//bitdata=c;
digitalWrite(CS,LOW);
digitalWrite(RS,LOW);
digitalWrite(SDA,(c&0x80));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x40));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x20));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x10));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x08));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x04));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x02));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x01));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(CS,HIGH);


}
void  LCD_DataWrite_IC(unsigned char d)  
{

//bitdata=d;
digitalWrite(CS,LOW);
digitalWrite(RS,HIGH);
digitalWrite(SDA,(d&0x80));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x40));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x20));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x10));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x08));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x04));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x02));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x01));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(CS,HIGH);
}

void LCD_DataWrite(unsigned char LCD_DataH,unsigned char LCD_DataL)
{
LCD_DataWrite_IC(LCD_DataH);
LCD_DataWrite_IC(LCD_DataL);
}


void  write_command(unsigned char c)
{
//bitdata=c;
digitalWrite(CS,LOW);
digitalWrite(RS,LOW);
digitalWrite(SDA,(c&0x80));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x40));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x20));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x10));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x08));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x04));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x02));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(c&0x01));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(CS,HIGH);      
}

void  write_data(unsigned char d)
{
//bitdata=d;
digitalWrite(CS,LOW);
digitalWrite(RS,HIGH);
digitalWrite(SDA,(d&0x80));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x40));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x20));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x10));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x08));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x04));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x02));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(SDA,(d&0x01));digitalWrite(SCL,LOW);digitalWrite(SCL,HIGH);
digitalWrite(CS,HIGH);
}


void Reset()
{
    digitalWrite(RST,LOW);
    delay1(10);
    digitalWrite(RST,HIGH);
    delay1(10);
}
//////////////////////////////////////////////////////////////////////////////////////////////

void lcd_initial()
{

   digitalWrite(RST,LOW);
   delay1(10);
   digitalWrite(RST,HIGH);
   delay1(10);
 
//------------------------------------------------------------------//  
//-------------------Software Reset-------------------------------//
//------------------------------------------------------------------//


 
write_command(0x11);//Sleep exit 
delay1 (12);
 
//ST7735R Frame Rate
write_command(0xB1); 
write_data(0x01); write_data(0x2C); write_data(0x2D); 
write_command(0xB2); 
write_data(0x01); write_data(0x2C); write_data(0x2D); 
write_command(0xB3); 
write_data(0x01); write_data(0x2C); write_data(0x2D); 
write_data(0x01); write_data(0x2C); write_data(0x2D); 

write_command(0xB4); //Column inversion 
write_data(0x07); 
 
//ST7735R Power Sequence
write_command(0xC0); 
write_data(0xA2); write_data(0x02); write_data(0x84); 
write_command(0xC1); write_data(0xC5); 
write_command(0xC2); 
write_data(0x0A); write_data(0x00); 
write_command(0xC3); 
write_data(0x8A); write_data(0x2A); 
write_command(0xC4); 
write_data(0x8A); write_data(0xEE); 
 
write_command(0xC5); //VCOM 
write_data(0x0E); 
 
write_command(0x36); //MX, MY, RGB mode 
write_data(0xC8); 

//ST7735R Gamma Sequence
write_command(0xe0); 
write_data(0x0f); write_data(0x1a); 
write_data(0x0f); write_data(0x18); 
write_data(0x2f); write_data(0x28); 
write_data(0x20); write_data(0x22); 
write_data(0x1f); write_data(0x1b); 
write_data(0x23); write_data(0x37); write_data(0x00); 

write_data(0x07); 
write_data(0x02); write_data(0x10); 
write_command(0xe1); 
write_data(0x0f); write_data(0x1b); 
write_data(0x0f); write_data(0x17); 
write_data(0x33); write_data(0x2c); 
write_data(0x29); write_data(0x2e); 
write_data(0x30); write_data(0x30); 
write_data(0x39); write_data(0x3f); 
write_data(0x00); write_data(0x07); 
write_data(0x03); write_data(0x10);  

write_command(0x2a);
write_data(0x00);write_data(0x00);
write_data(0x00);write_data(0x7f);
write_command(0x2b);
write_data(0x00);write_data(0x00);
write_data(0x00);write_data(0x9f);

write_command(0xF0); //Enable test command  
write_data(0x01); 
write_command(0xF6); //Disable ram power save mode 
write_data(0x00); 
 
write_command(0x3A); //65k mode 
write_data(0x05); 
 

	write_command(0x29);//Display on
}

void dsp_single_colour(unsigned char DH,unsigned char DL)
{
 unsigned char i,j;
 //RamAdressSet();
 for (i=0;i<160;i++)
    for (j=0;j<128;j++)
        LCD_DataWrite(DH,DL);
}
void setup()
{
  pinMode(0,OUTPUT);  
  pinMode(1,OUTPUT); 
  pinMode(2,OUTPUT); 
  pinMode(3,OUTPUT); 
  pinMode(4,OUTPUT); 
  pinMode(5,OUTPUT); 
  pinMode(6,OUTPUT); 
  pinMode(7,OUTPUT); 
  pinMode(8,OUTPUT); 
  pinMode(9,OUTPUT); 
  pinMode(10,OUTPUT); 
  pinMode(11,OUTPUT); 
lcd_initial();
}

void loop()
{
write_command(0x2C);
  //  Disp_gradscal(); //灰阶
   // delay1(500); 
      
//    dsp_single_colour(0x84,0x10);//灰色
//    delay1(500);	

    dsp_single_colour(0xff,0xff);//白色
    delay1(50);	
    	
    dsp_single_colour(0x00,0x00);//黑色
    delay1(50);	
    	
    dsp_single_colour(0xf8,0x00);//红色
    delay1(50);	
    	
    dsp_single_colour(0x07,0xe0);//绿色
    delay1(50);	
    	
    dsp_single_colour(0x00,0x1f);//蓝色
    delay1(50);	

}
