#include "derivative.h"
#include "i2c.h"
void Init_I2C(void)
{
 SIM_SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;
 SIM_SCGC4 |= SIM_SCGC4_I2C0_MASK;
 PORTE_PCR24 = PORT_PCR_MUX(5);
 PORTE_PCR25 = PORT_PCR_MUX(5);
 I2C0_F = 0x14; //24,000,000 Hz / 80 = 300 kHz (0x14 = 80 on the SCL divider, MUL = 0, ref manual page 706)
 I2C0_C1 = I2C_C1_IICEN_MASK;
}
/* Inicia Transmissao I2C
* SlaveID: endereco do slave
* "Mode" define modo Read (1) ou Write (0)
*/
void IIC_StartTransmission (char SlaveID, char Mode)
{
 SlaveID = SlaveID << 1;
 SlaveID |= (Mode & 0x01);
 i2c_Start();
 i2c_write_byte(SlaveID);
}
void Pause(void){
 int n;
 for(n=1;n<50;n++) {
	 __asm("nop");
 }
}
/* Le um registrador do slave I2C
* SlaveID: endereco do slave
* RegisterAddress: endereco do registrador interno do slave
* Retorna o valor do registrador
*/
char I2CReadRegister(char SlaveID, char RegisterAddress)
{
 char result;
 IIC_StartTransmission(SlaveID,MWSR);
 i2c_Wait();
 i2c_write_byte(RegisterAddress);
 i2c_Wait();
 i2c_RepeatedStart();
 i2c_write_byte((SlaveID << 1) | 0x01);
 i2c_Wait();
 i2c_EnterRxMode();
 i2c_DisableAck(); // Desabilita ACK por ser o penultimo byte a ler
 result = I2C0_D ; // Dummy read
 i2c_Wait();
 i2c_Stop(); // Envia STOP por ser o ultimo byte
 result = I2C0_D ; // Le o byte
 Pause();
 return result;
}
/* Escreve um byte no registrador interno do slave I2C
* SlaveID: endereco do slave
* RegisterAddress: endereco do registrador interno do slave
* Data: Valor a escrever
*/
void I2CWriteRegister(char SlaveID, char RegisterAddress, char Data)
{
 IIC_StartTransmission(SlaveID,MWSR);
 i2c_Wait();
 i2c_write_byte(RegisterAddress);
 i2c_Wait();
 i2c_write_byte(Data);
 i2c_Wait();
 i2c_Stop();
 Pause();
}
/* Le "N" registradores internos do slave I2C
* SlaveID: endereco do slave
* RegisterAddress: endereco do primeiro registrador interno do slave a ser lido
* n: Numero de registradores a serem lidos em sequencia
* Resultado armazenado no ponteiro *r
*/
void I2CReadMultiRegisters(char SlaveID, char RegisterAddress, char n, char * r)
{
 char i;
 IIC_StartTransmission(SlaveID,MWSR);
 i2c_Wait();
 i2c_write_byte(RegisterAddress);
 i2c_Wait();
 i2c_RepeatedStart();
 i2c_write_byte((SlaveID << 1) | 0x01);
 i2c_Wait();
 i2c_EnterRxMode();
 i2c_EnableAck();
 i = I2C0_D ; // Dummy read
 i2c_Wait();
 for(i=0;i<n-2;i++) // le n-1 bytes
 {
 *r = I2C0_D;
 r++;
 i2c_Wait();
 }
 i2c_DisableAck(); // Desabilita ACK por ser o penultimo byte a ler
 *r = I2C0_D; // Le penultimo byte
 r++;
 i2c_Wait();
 i2c_Stop(); // Envia STOP por ser o ultimo byte
 *r = I2C0_D; // Le ultimo byte
 Pause();
}
