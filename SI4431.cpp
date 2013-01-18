/*
Name:		EZRadioPRO library for Arduino
Version:	1.0
Created:	24.02.2012
Updated:	24.02.2012
Programmer:	Erezeev A.
Production:	JT5.RU
Source:		https://github.com/jt5/EZRadioPRO

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SI4431.h"

SI4431Class SI4431;

/****************************************************
�������������� ������� ��� ������ � ������� SI4431
*****************************************************/

#define EZ_WR(reg, val)       SI4431Class::WriteRegister (reg, val);
#define EZ_RR(reg)            SI4431Class::ReadRegister (reg);
#define EZ_SWRST()            EZ_WR (0x07, 0x80);
#define EZ_RSTATUS(ST1, ST2)  {ST1 = EZ_RR(0x03); ST2 = EZ_RR(0x04);}
#define EZ_WAIT(IRQBIT)       {while(IRQBIT != 0);}

void SI4431Class::begin()
{
	#ifdef LED_ENABLED
	// ������������ �� ����� ������ ����������� TX � RX
    TX_LED_DDR_SET
    RX_LED_DDR_SET
	#endif
	// ������������ �� ����� ����� ������ �����������
    NSS_DDR_SET
	// �������� ��� ����� nIRQ
    PORTB |= (1<<PB1);
	// ������������� SPI ����������
	DDR_SPI |= (1<<DD_MOSI)|(1<<DD_SCK)|(1 << DD_SS);
	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR = (1<<SPI2X); // 8 ���
}


void SI4431Class::WriteRegister(u8 reg, u8 value)
{
    // ����� ����������� ���������� ������� nSEL � ������ �������
    NSS_LO
    // ����� ����� �������� � ������� ������ SPI ��
    // (����� ���������� ������� ���!)     
    SPDR  =   (reg|0x80);
    while(!(SPSR & (1<<SPIF))); //��� ��������� ��������
    // ����� ����� �������� � ������� ����������� 
    SPDR = value;
    while(!(SPSR & (1<<SPIF))); //��� ��������� ��������    
    // �����������  ����������� ���������� ������� nSEL � ������� �������
    NSS_HI   
}

u8 SI4431Class::ReadRegister (u8 reg)
{   
    u8 value;
    // ����� ����������� ���������� ������� nSEL � ������ �������
    NSS_LO
    // ����� ����� �������� � ������� ������ SPI ��
    SPDR  =   (reg);
    while(!(SPSR & (1<<SPIF))); //��� ��������� ��������   
    SPDR = 0xFF; // ���������� ������ ����
    while(!(SPSR & (1<<SPIF))); // //��� ��������� ��������    
    // �����������  ����������� ���������� ������� nSEL � ������� �������
    NSS_HI
    // ���������� �������� �� SPI �������� ���������� �������� �����������
    value = SPDR;
    return value;               
}

void SI4431Class::Init(u8 TXPower)
{
	u8 ItStatus1, ItStatus2;
	  //������ �������� ��������� ���������� ��� ������� ������ ���������� � ������������ ������ NIRQ
      EZ_RSTATUS(ItStatus1, ItStatus2)
      //����������� �����
      EZ_SWRST()     //������ �������� 0x80 � ������� Operating & Function Control1
      // ���� ������ ���������� POR (����� ��� ������ �������) �� �����������
      EZ_WAIT (NIRQ)  	        
      //������ �������� ��������� ���������� ��� ������� ������ ���������� � ������������ ������ NIRQ        
      EZ_RSTATUS(ItStatus1, ItStatus2)  
      //put_messageP(PSTR("\r\nRadio is ready"));   
      // ���� ������ ���������� "����������" �� �����������        
      EZ_WAIT (NIRQ)                  
      //������ �������� ��������� ���������� ��� ������� ������ ���������� � ������������ ������ NIRQ        
      EZ_RSTATUS(ItStatus1, ItStatus2)  
      //print_P(PSTR("\r\nRadio is ready"));     		      
      /*��������� ���������� �������� ������� � xls ����� �� ������������� Si4431*/
      //��������� ������� ������� 868.7 MHz
      EZ_WR(0x75, 0x73)
      EZ_WR(0x76, 0x6C)
      EZ_WR(0x77, 0xC0)
      //�������� �������� ������ (9.6kbps)
      EZ_WR(0x6E, 0x4E)
      EZ_WR(0x6F, 0xA5)
      EZ_WR(0x70, 0x2C)
      //��������� �������� �������� ������� ��� �������� (+-45kHz)
      EZ_WR(0x72, 0x48)
      /*��������� ���������� ������ �������� xls �����(9.6 kbps, deviation: 45 kHz, channel filter BW: 102.2 kHz)*/
      EZ_WR(0x1C, 0x1E)
      EZ_WR(0x20, 0xD0)
      EZ_WR(0x21, 0x00)
      EZ_WR(0x22, 0x9D)
      EZ_WR(0x23, 0x49)
      EZ_WR(0x24, 0x00)
      EZ_WR(0x25, 0x24)
      EZ_WR(0x1D, 0x40)
      EZ_WR(0x1E, 0x0A)
      EZ_WR(0x2A, 0x20)						
      //����� ��������� 5 ����
      EZ_WR(0x34, 0x0A)
      //����� ����������� ��������� 20 ���
      EZ_WR(0x35, 0x2A)	
      //������ ������������ ������ (header bytes), ��������� ���������� ����� �������, ��������� ����� ����������� � 2 �����
      EZ_WR(0x33, 0x02)	
      //������ ����������� 0x2DD4
      EZ_WR(0x36, 0x2D)
      EZ_WR(0x37, 0xD4)
      //���������� ������������ ������� TX � RX � ������ �� CRC-16 (IBM)
      EZ_WR(0x30, 0x8D)
      //������ �������� ������������ ���� ��� ������
      EZ_WR(0x32, 0x00)
      //��������� ������ FIFO � GFSK ���������
      EZ_WR(0x71, 0x63)
      /* ������������ ����� GPIO*/
      EZ_WR(0x0C, 0x12)
      EZ_WR(0x0D, 0x15)	
      //������������ ���
      EZ_WR(0x69, 0x60)
	  //��������� �������� �����������
	  EZ_WR(0x6D, (TXPower | 0x18) & 0x1F);
      //������������ �������������
      EZ_WR(0x09, 0xD7)
      /*���������� �������� ������*/
      EZ_WR(0x07, 0x05)
      //�������� 2 ����������:
      // a) ������ ���������� ����� ����������� ������:  'ipkval'
      // �) ������ ���������� ����� ������ � ������������ �� CRC:  'icrcerror' 
      EZ_WR(0x05, 0x03)
      EZ_WR(0x06, 0x00)
      //������ �������� ��������� ���������� ��� ������� ������ ���������� � ������������ ������ NIRQ        
      EZ_RSTATUS(ItStatus1, ItStatus2);	
}

void SI4431Class::RXEnable(void)
{
        /*��������� ����� �����*/
        EZ_WR(0x07, 0x05)
}		

void SI4431Class::RXDisable(void)
{
        /*���������� ����� �����*/
        EZ_WR(0x07, 0x01)
}		

void SI4431Class::FIFOReset(void)
{       //����� FIFO
        EZ_WR(0x08, 0x02)
        EZ_WR(0x08, 0x00)	
}
/************************************************************************
* ������� ��� �������� ����� ������ � ����                             
* len - ����� ������������� �����
* DataSrc - ��������� �� ������ ������������ ������ � SRAM
************************************************************************/
void SI4431Class::TXData(u8* DataSrc, u8 len)
{
	u8 Status1, Status2;
	//put_messageP(PSTR("\r\nTX dump:"));												
    //������ ������ ������ (�� XTAL �������� ��� ����������� �������� ������������ � TX!)
	EZ_WR(0x07, 0x01)
    //������� ��������� ������������ ����� �������� ������
	#ifdef LED_ENABLED
	TX_LED_SET						
	#endif
    //������ ����� ������ 
	EZ_WR(0x3E, len)
    // �������� ����� FIFO ��������� �������                        
    for (u8 i=0; i< len; i++)
    {
	  u8 c = *DataSrc++;
      EZ_WR(0x7F, c)		
	  //i2hex(c, I2A_BUFFER,2);
	  //put_message(I2A_BUFFER);				  
	}                                         
	//������ ���� ���������� ����� ���������� �� ��������� ��������
	//������� ����� ������������ ��� �������������� �� �� �������� �������� ������
	EZ_WR(0x05, 0x04)
	EZ_WR(0x06, 0x00)
	//������ �������� ��������� ���������� ��� ������� ������ ���������� � ������������ ������ NIRQ        
	EZ_RSTATUS(Status1, Status2)
	/*���������� �����������*/
	//����������� ��������� ����� � ������������� ��� ����������
	EZ_WR(0x07, 0x09)
	/*�������� ���������� �� ������������ ������*/
	//�� ���� ���������� 'ipksent'
	EZ_WAIT(NIRQ)
	//������ �������� ��������� ���������� ��� ������� ������ ���������� � ������������ ������ NIRQ        
	EZ_RSTATUS(Status1, Status2)						
	// ���� ����� ��������� ��� �����   
	_delay_ms(100);
	// ��������� ���������
	#ifdef LED_ENABLED
	TX_LED_CLR
	#endif
	//put_messageP(PSTR("\r\nTX end. Sent bytes: "));		
	//i2hex(len, I2A_BUFFER,2);
	//put_message(I2A_BUFFER);				  																
}

void SI4431Class::RXIRQEnable(void)
{
                              //�������� 2 ����������:
                              // a) ������ ���������� ����� ����������� ������:  'ipkval'
                              // �) ������ ���������� ����� ������ � ������������ �� CRC:  'icrcerror' 
				  EZ_WR(0x05, 0x03)
			      EZ_WR(0x06, 0x00)
}				  

void SI4431Class::ReadStatus(u8* Var1, u8* Var2)
{
 *Var1 = EZ_RR(0x03); *Var2 = EZ_RR(0x04);
 }				  

void SI4431Class::RXData(u8* DataDst, u8 len)
{
                    // �������� ������ �� RX FIFO
					//put_messageP(PSTR("\r\nRX dump:"));	
					while(len--)
					{
						//������ FIFO ����� ����. ������� 0x7F
						u8 c =  EZ_RR(0x7F)
						*DataDst++ = c;						
//						i2hex(c, I2A_BUFFER,2);
						//put_message(I2A_BUFFER);							
                        //putchar_ (c);
					}
}	

u8 SI4431Class::RXPacketLen(void)
{	
	 //������ ������ �������� ������
	 u8 len = EZ_RR(0x4B)   //read the Received Packet Length register
	//     i2hex(len, I2A_BUFFER, 2);
	//put_message(I2A_BUFFER);
	 return len;
}


