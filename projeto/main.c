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
int pass[TAM] = {1,2,3,4,5,6};

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

void showArray(int s[],int n){ //exibe a senha no display (teste)
	int i;
	for(i=0; i<n;i++){
		nokia_lcd_set_cursor(6*i,40);
		nokia_lcd_write_char((char)(s[i]+0x30),1);
	}
	nokia_lcd_render(); //atualiza o display
}

int main(void)
{
	unsigned char nr;
	int senha[TAM] = {0,0,0,0,0,0}, i = 0;

	DDRB = 0xFF; //porta B como saída

	sei();
	KP_Setup();
	nokia_lcd_init(); //inicia o LCD

    while(1)
    {	
		//####### obtem a senha ###############
		nr = KP_GetKey(); //lê constantemente o teclado
		KP_WaitRelease(); // espera a tecla ser solta
			
	    if(nr!= Key_None)
		{
			senha[i] = (int)(nr-0x30); // nr(hexa) to nr(int)
			showArray(senha,i+1);
			i++;
	    }
		//###################################
		
		//####### Verifica a senha #########
		if(i == TAM)
		{
			nokia_lcd_set_cursor(0,0);
			if(compArray(senha,pass) == 1)
				nokia_lcd_write_string("pass correta  ",1);//escreve no buffer do LCD
			else nokia_lcd_write_string("pass incorreta",1);//escreve no buffer do LCD
			nokia_lcd_render();
			setArray(senha,0); //inicializa a senha
			i = 0;
		}
		//###################
	}
}