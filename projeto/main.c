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
		nr = KP_GetKey(); //lê constantemente o teclado
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
	if(getPassWord(new, "Senha atual: ")&& verifPassWord(new, curr)){
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

int main(void)
{
	unsigned char nr;
	int senha[TAM] = {0,0,0,0,0,0};

	DDRB = 0xFF; //porta B como saída

	sei();
	KP_Setup();
	nokia_lcd_init(); //inicia o LCD

    while(1)
    {	
		nokia_lcd_clear();
		nokia_lcd_set_cursor(2.5*6,0);
		nokia_lcd_write_string("Bem vindo",1);
		nokia_lcd_set_cursor(0.5*6,10);
		nokia_lcd_write_string("Digite * ou #",1);
		nokia_lcd_render();
		
		nr = KP_GetKey(); //lê constantemente o teclado
		KP_WaitRelease(); // espera a tecla ser solta
		
	    if(nr != Key_None)
		{	
			if(nr == 'E'){// 'E' equivale a '*'
				
				if(getPassWord(senha,"Senha: ")){ //tenta obter a senha
					if(verifPassWord(senha,pass))//verifica se a senha está correta
						;//DESTRAVA A PORTA
					_delay_ms(3000);
					}
			}
			else if(nr == 'F'){ // 'F' equivale a '#'
				changePassWord(pass);
				_delay_ms(3000);
				//while ((nr = KP_KeyPressed()) == Key_None); //Espera alguma tecla ser informada
			}
	
	    }
	}
}