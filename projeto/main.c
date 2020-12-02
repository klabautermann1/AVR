/*
 * projeto.c
 *
 * Created: 15/11/2020 13:42:24
 * Author : embedded02
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "Keypad.h"
#include "nokia5110.h"

#define TAM 6
int pass[TAM] = {1,2,3,4,5,6}; //senha de f�brica
int enabled = 1; //flag de ativa��o do teclado matricial

int compArray(int s[], int p[]){
	int j = 0, ret =0;
	
	while(s[j] == p[j] && j < TAM) j++;
	if(j >= TAM) ret = 1;
	return ret;
}
void setArray(int s[], int n){
	int i = TAM;
	while(i) s[--i] = 0;
}
void copyArray(int s[], int p[]){
	int i = TAM;
	while(i) i--, s[i] = p[i];
}

void showArray(int s[],int n){ //exibe a senha no display (teste)
	int i;
	for(i=0; i<n;i++){
		nokia_lcd_set_cursor(6*i,40);
		nokia_lcd_write_char((char)(s[i]+0x30),1);
	}
	nokia_lcd_render(); //atualiza o display
}

int getPassWord(int senha[], char msn[]){
	int i = 0, nr;
	nokia_lcd_clear();
	nokia_lcd_set_cursor(0,0);
	nokia_lcd_write_string(msn,1);
	nokia_lcd_render();
	
	while(i<TAM){
		nr = KP_GetKey(); //l� constantemente o teclado
		KP_WaitRelease(); // espera a tecla ser solta
	
		if(nr != Key_None && nr != 'E' && nr != 'F')
		{
			senha[i] = (int)(nr-0x30); // nr(hexa) to nr(int)
            nokia_lcd_set_cursor(i*6,10);
			nokia_lcd_write_char('+',1);
			nokia_lcd_render();
			i++;
		}
		if(nr == 'E' || nr == 'F')
			return 0;
	}
	return 1;
}

int verifPassWord(int senha[], int pass[]){
	int ret;
	nokia_lcd_clear();
	
	ret = compArray(senha,pass);
	if(ret == 1){
		nokia_lcd_set_cursor(0,16);
		nokia_lcd_write_string("allowed",2); //escreve no buffer do LCD
	}
	else{
		nokia_lcd_set_cursor(0,16);
		nokia_lcd_write_string("denied ",2); //escreve no buffer do LCD
	}
	nokia_lcd_render();
	
	setArray(senha,0); //inicializa a senha
	return ret;
}

int changePassWord(int curr[]){
	int new[TAM];
	if(getPassWord(new, "Senha atual: ") && compArray(new, curr)){
		if(getPassWord(new, "Nova senha: ")){
			copyArray(curr,new);
			nokia_lcd_clear();
			nokia_lcd_set_cursor(0,20);
			nokia_lcd_write_string("changed",2);
			nokia_lcd_render();
		}
	}
	else return 0;
	return 1;
}

///#######

void EEPROM_write(unsigned int uiEndereco, unsigned char ucDado)
{
	while(EECR & (1<<EEPE));//espera completar um escrita pr�via
	EEAR = uiEndereco;//carrega o endere�o para a escrita
	EEDR = ucDado;//carrega o dado a ser escrito
	EECR |= (1<<EEMPE);//escreve um l�gico em EEMPE
	EECR |= (1<<EEPE);//inicia a escrita ativando EEPE
}

unsigned char EEPROM_read(unsigned int uiEndereco)
{
	while(EECR & (1<<EEPE)); //espera completar um escrita pr�via
	EEAR = uiEndereco; //escreve o endere�o de leitura
	EECR |= (1<<EERE); //inicia a leitura ativando EERE
	return EEDR; //retorna o valor lido do registrador de dados
}
ISR(INT0_vect){
	enabled = !enabled;
}
int main(void)
{
	unsigned char nr;
	int senha[TAM] = {0,0,0,0,0,0}, i;

	//GPIO
	DDRB = 0xFF; //porta B como sa�da
	DDRD &= ~(1<<2); //pino PD2 como entrada
	PORTD |= 1<<2; //habiida resistor de pull-up do pino PD2
	
	//Interrup??es
	EICRA = 0b00000011; //configura para interrup??o na borta de subida para ambas INT1 e INT0;
	EIMSK = 0b00000001; //habilita ambas INT1 e INT0
	sei(); // habilita interrup��es globais
	
	KP_Setup();
	nokia_lcd_init(); //inicia o LCD
	
	//ler senha salva na eeprom
	if(EEPROM_read(0x00) == 1){ //Digito verificador se for lido 1 significa que algo est� salvo na eeprom, 0 significa que nada foi salvo ainda
		for(i=0;i<TAM;i++)
			pass[i] = EEPROM_read(i+1);
	}

    while(1)
    {	
		nokia_lcd_clear();
		nokia_lcd_set_cursor(2.5*6,0);
		nokia_lcd_write_string("Bem vindo",1);
		nokia_lcd_set_cursor(0.5*6,10);
		nokia_lcd_write_string("Digite * ou #",1);
		if(!enabled){
			nokia_lcd_set_cursor(7*6,40);
			nokia_lcd_write_string("unabled",1);
		}
		
		nokia_lcd_render();
		
		nr = KP_GetKey(); //l� constantemente o teclado
		KP_WaitRelease(); // espera a tecla ser solta
		
	    if(nr != Key_None && enabled)
		{	
			if(nr == 'E'){// 'E' equivale a '*'
				
				if(getPassWord(senha,"Senha: ")){ //tenta obter a senha
					if(verifPassWord(senha,pass))//verifica se a senha est� correta
						;//DESTRAVA A PORTA
					_delay_ms(2000);
					}
			}
			else if(nr == 'F'){ // 'F' equivale a '#'
				changePassWord(pass);
				//salva nova senha na eeprom
				EEPROM_write(0, 1); //escreve digito para informar que algo foi escrito na eeprom
				for(i=0;i<TAM;i++)
					EEPROM_write(i+1, pass[i]);
				_delay_ms(2000);
			}
	
	    }
	}
}