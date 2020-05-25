#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define BUFF_WIDTH 80
#define CENTER_OFFSET 12
#define LEFT_OFFSET 25
#define REG_SCREEN_SIZE 9

struct VIDEO
{
	unsigned char symb;
	unsigned char attr;
};

int attribute = 0x6e; //цвет

void print(int offset, int value);
void getRegisterValue();


// Указатели на старые обработчики ведущегоконтроллера
void interrupt(*oldHandleC8) (...);
void interrupt(*oldHandleC9) (...);
void interrupt(*oldHandleD0) (...);
void interrupt(*oldHandleD1) (...);
void interrupt(*oldHandleD2) (...);
void interrupt(*oldHandleD3) (...);
void interrupt(*oldHandleD9) (...);
void interrupt(*oldHandleD5) (...);

// Указатели на старые обработчики ведомого контроллера
void interrupt(*oldHandle08) (...);
void interrupt(*oldHandle09) (...);
void interrupt(*oldHandle0A) (...);
void interrupt(*oldHandle0B) (...);
void interrupt(*oldHandle0C) (...);
void interrupt(*oldHandle0D) (...);
void interrupt(*oldHandle0E) (...);
void interrupt(*oldHandle0F) (...);


// Описание новых обработчиков ведущего контроллера
void interrupt newHandleC8(...) { getRegisterValue(); oldHandleC8(); }
void interrupt newHandleC9(...) { attribute++; getRegisterValue(); 	oldHandleC9(); }
void interrupt newHandleD0(...) { getRegisterValue(); oldHandleD0(); }
void interrupt newHandleD1(...) { getRegisterValue(); oldHandleD1(); }
void interrupt newHandleD2(...) { getRegisterValue(); oldHandleD2(); }
void interrupt newHandleD3(...) { getRegisterValue(); oldHandleD3(); }
void interrupt newHandleD9(...) { getRegisterValue(); oldHandleD9(); }
void interrupt newHandleD5(...) { getRegisterValue(); oldHandleD5(); }

// Описание новых обработчиков ведомого контроллера
void interrupt newHandle08(...) { getRegisterValue(); oldHandle08(); }
void interrupt newHandle09(...) { getRegisterValue(); oldHandle09(); }
void interrupt newHandle0A(...) { getRegisterValue(); oldHandle0A(); }
void interrupt newHandle0B(...) { getRegisterValue(); oldHandle0B(); }
void interrupt newHandle0C(...) { getRegisterValue(); oldHandle0C(); }
void interrupt newHandle0D(...) { getRegisterValue(); oldHandle0D(); }
void interrupt newHandle0E(...) { getRegisterValue(); oldHandle0E(); }
void interrupt newHandle0F(...) { getRegisterValue(); oldHandle0F(); }


// Функция отрисовывает значения на экран
void print(int offset, int value)
{
	char temp;

	VIDEO far* screen = (VIDEO far*)MK_FP(0xB800, 0);
	screen += CENTER_OFFSET * BUFF_WIDTH + offset;

	for (int i = 7; i >= 0; i--)
	{
		temp = value % 2;
		value /= 2;
		screen->symb = temp + '0';
		screen->attr = attribute;
		screen++;
	}
}

// Получем значения регистров контроллера
void getRegisterValue()
{
	// Для ведущего
	print(0 + LEFT_OFFSET, inp(0x21));

	outp(0x20, 0x0B);
	print(REG_SCREEN_SIZE + LEFT_OFFSET, inp(0x20));

	outp(0x20, 0x0A);
	print(REG_SCREEN_SIZE * 2 + LEFT_OFFSET, inp(0x20));

	// Для ведомого
	print(BUFF_WIDTH + LEFT_OFFSET, inp(0xA1));

	outp(0xA0, 0x0B);
	print(BUFF_WIDTH + REG_SCREEN_SIZE + LEFT_OFFSET, inp(0xA0));

	outp(0xA0, 0x0A);
	print(BUFF_WIDTH + REG_SCREEN_SIZE * 2 + LEFT_OFFSET, inp(0xA0));
}

void init()
{
	// IRQ 0-7
	// Сохраняем старые значения векторов прерываний ведущего контроллера
	oldHandle90 = getvect(0x08); // Timer
	oldHandle91 = getvect(0x09); // Keyboard
	oldHandle92 = getvect(0x0A); // Slave IRQ
	oldHandle93 = getvect(0x0B); // Random deviece
	oldHandle94 = getvect(0x0C); // Random deviece
	oldHandle95 = getvect(0x0D); // Random deviece
	oldHandle96 = getvect(0x0E); // Random deviece
	oldHandle97 = getvect(0x0F); // Random deviece

	// IRQ 8-15
	// Сохраняем старые значения векторов прерываний ведомого контроллера
	oldHandle08 = getvect(0x70); // Real time clock
	oldHandle09 = getvect(0x71); // Random deviece
	oldHandle0A = getvect(0x72); // Random deviece
	oldHandle0B = getvect(0x73); // Random deviece or timer
	oldHandle0C = getvect(0x74); // PS/2 mouse
	oldHandle0D = getvect(0x75); // FPU error
	oldHandle0E = getvect(0x76); // Random deviece or first ATA controller
	oldHandle0F = getvect(0x77); // Random deviece or second ATA controller

	// Устанавливаем новые значения векторов прерываний для ведущего котроллера
	setvect(0xC8, newHandleC8);
	setvect(0xC9, newHandleC9);
	setvect(0xD0, newHandleD0);
	setvect(0xD1, newHandleD1);
	setvect(0xD2, newHandleD2);
	setvect(0xD3, newHandleD3);
	setvect(0xD9, newHandleD9);
	setvect(0xD5, newHandleD5);

	// Устанавливаем новые значения векторов прерываний для ведомого котроллера
	setvect(0x08, newHandle08);
	setvect(0x09, newHandle09);
	setvect(0x0A, newHandle0A);
	setvect(0x0B, newHandle0B);
	setvect(0x0C, newHandle0C);
	setvect(0x0D, newHandle0D);
	setvect(0x0E, newHandle0E);
	setvect(0x0F, newHandle0F);

	// Запрещаем прерывания (cli)
	_disable();


	// Master
	//Оtправляем значение в порт
	outp(0x20, 0x11); // ICW1
	outp(0x21, 0x90); // ICW2	// номер обработчика
	outp(0x21, 0x04); // ICW3	// Номер выхода ведомого котроллера
	outp(0x21, 0x01); // ICW4	// Тип процессора 8086

	// Slave
	outp(0xA0, 0x11); // ICW1
	outp(0xA1, 0x08); // ICW2	// Номер обработчика ведущего котроллера
	outp(0xA1, 0x02); // ICW3  показывает через какой порт связаны ведущий и ведомый контроллеры
	outp(0xA1, 0x01); // ICW4	// Тип процессора 8086

	// Разрешаем прерывания (sti)
	_enable();
}

int main()
{
	unsigned far* fp;

	init();
	clrscr();
	FP_SEG(fp) = _psp;
	FP_OFF(fp) = 0x2c;
	_dos_freemem(*fp);

	_dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);
	return 0;
}


