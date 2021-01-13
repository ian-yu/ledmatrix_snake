#include <avr/io.h>
#include <avr/interrupt.h>
#include "matrix.h"
#include "joystick.h"

#define NULL 0
#define TCD1_AUTOMOVE_PER (6500)

const uint8_t collide_wall_image[8] =
{
	0b11111111,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11111111
};

const uint8_t collide_self_image[8] = 
{
	0b10000001,
	0b10000001,
	0b10000001,
	0b10000001,
	0b10000001,
	0b10000001,
	0b10000001,
	0b10000001	
};

// Matrix values
volatile uint8_t led_matrix[8];
volatile uint8_t row_count = 0;

// ADC values
volatile int16_t x_axis;
volatile int16_t y_axis;
volatile char input = 0;

// Snake
typedef struct point point;
struct point
{
	volatile int8_t x;
	volatile int8_t y;
	volatile point* next;
};

volatile struct point snake[10];
volatile struct point *head;
volatile struct point *tail;
uint8_t length;
volatile char movedir;
volatile uint8_t collide = 0;

// Apple
struct point apple;

// Assembly function
extern void clock_init(void);


// Function declarations
void delay(uint16_t time);
char get_direction(void);
void clear_matrix(void);
void snake_init(void);
void draw_snake(void);
void move_snake(char dir);
void automove_init(void);
void display_image(const uint8_t image[]);
uint8_t in_snake(int8_t x, int8_t y);
void apple_init(void);

int main(void)
{
	// Changes clock speed to 32 MHz
	clock_init();
	
	clear_matrix();

	// Initializing
	matrix_init();	
	refreshmatrix_init();
	joystick_adc_init();
	joystick_tcc1_init();

	// Initializing Snake
	snake_init();
	automove_init();

	// Enabling global interrupts
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
	sei();

	while (1)
	{
		// Game is running
		while (!collide)
		{
			input = get_direction();
		}
		if (collide == 1)
			display_image(collide_wall_image);
		else if (collide == 2)
			display_image(collide_self_image);
		input = get_direction();
		if (input != 0)
		{
			collide = 0;
			snake_init();
		}
	}
}

// Clears LED Matrix
void clear_matrix()
{
	// Initialize all values to 0
	for (uint8_t i = 0; i < 8; i++)
	{
		led_matrix[i] = 0;
	}	
}

// Generic delay (TCD0)
void delay(uint16_t time)
{
	TCD0.PER = time;
	TCD0.CNT = 0;
	TCD0.CTRLA = TC_CLKSEL_DIV1024_gc;
	while(!(TCD0.INTFLAGS & TC0_OVFIF_bm));
	TCD0.INTFLAGS = TC0_OVFIF_bm;
}

// Returns direction that snake is traveling
char get_direction()
// X axis takes priority
{
	// Left
	if (x_axis > 1800)
		return 'l';
	// Right
	else if (x_axis < 200)
		return 'r';
	// Up
	else if (y_axis > 1800)
		return 'u';
	// Down
	else if (y_axis < 200)
		return 'd';
		
	return 0; 
}

// Initializing snake
void snake_init()
{
	// Initialize all values to -1
	for (uint8_t i = 0; i < 10; i++)
	{
		snake[i].x = -1;
		snake[i].y = -1;
		snake[i].next = NULL;
	}
	
	// Initialize starting point of snake
	snake[0].x = 0;
	snake[0].y = 3;
	snake[0].next = &snake[1];
	snake[1].x = 1;
	snake[1].y = 3;
	snake[1].next = &snake[2];
	snake[2].x = 2;
	snake[2].y = 3;
	snake[2].next = &snake[3];
	snake[3].x = 3;
	snake[3].y = 3;
	snake[3].next = &snake[4];
	snake[4].x = 4;
	snake[4].y = 3;
	snake[4].next = &snake[5];
	snake[5].x = 5;
	snake[5].y = 3;

	// Initialize head and tail
	tail = &snake[0];
	head = &snake[5];
	
	length = 6;
	
	movedir = 'r';
}

// Draw snake on LED Matrix
void draw_snake()
{	
	// Draw snake
	for (uint8_t i = 0; i < 10; i++)
	{
		// If snake value exists then set value
		if (snake[i].x != -1 && snake[i].y != -1)
		{
			led_matrix[7-snake[i].y] |= (0x80 >> snake[i].x);
		}
	}

	// Draw apple
	// led_matrix[7-apple.y] |= (0x80 >> apple.x);
}

// Move snake in certain direction and marks
void move_snake(char dir)
{
	// Move up
	if (dir == 'u' && movedir != 'd')
	{
		// Collide into wall
		if (head->y >= 7)
		{
			collide = 1;
		}
		// Collide into self
		else if (in_snake(head->x, head->y+1))
		{
			collide = 2;
		}
		else
		{
			// Replace tail with point above head
			struct point *temp = (struct point*)tail->next;
		
			tail->x = head->x;
			tail->y = head->y + 1;
			tail->next = NULL;
			head->next = tail;
			head = tail;
			tail = temp;
			movedir = 'u';
		}
	}
	// Move down
	else if (dir == 'd' && movedir != 'u')
	{
		if (head->y <= 0)
		{
			collide = 1;
		}
		else if (in_snake(head->x, head->y - 1))
		{
			collide = 2;
		}
		else
		{
			// Replace tail with point below head
			struct point *temp = (struct point*)tail->next;
		
			tail->x = head->x;
			tail->y = head->y - 1;
			tail->next = NULL;
			head->next = tail;
			head = tail;
			tail = temp;
			movedir = 'd';
		}
	}
	// Move right
	else if (dir == 'r' && movedir != 'l')
	{
		if (head->x >= 7)
		{
			collide = 1;
		}
		else if (in_snake(head->x+1, head->y))
		{
			collide = 2;
		}
		else
		{
			// Replace tail with point below head
			struct point *temp = (struct point*)tail->next;
		
			tail->x = head->x + 1;
			tail->y = head->y;
			tail->next = NULL;
			head->next = tail;
			head = tail;
			tail = temp;
			movedir = 'r';
		}
	}
	// Move left
	else if (dir == 'l' && movedir != 'r')
	{
		if (head->x <= 0)
		{
			collide = 1;
		}
		else if (in_snake(head->x-1, head->y))
		{
			collide = 2;
		}
		else
		{
			// Replace tail with point below head
			struct point *temp = (struct point*)(tail->next);
		
			tail->x = head->x - 1;
			tail->y = head->y;
			tail->next = NULL;
			head->next = tail;
			head = tail;
			tail = temp;
			movedir = 'l';
		}
	}

	// Update image
	clear_matrix();
	draw_snake();	
}

// Initialize automove timer (TCD1)
void automove_init()
{
	TCD1.PER = TCD1_AUTOMOVE_PER;
	TCD1.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCD1.INTCTRLA = TC_OVFINTLVL_MED_gc;
}

// Display particular image to LED Matrix
void display_image(const uint8_t image[])
{
	for (uint8_t i = 0; i < 8; i++)
	{
		led_matrix[i] = image[i];
	}
}

// Checks if point is in the snake
uint8_t in_snake(int8_t x, int8_t y)
{
	for (uint8_t i = 0; i < length; i++)
	{
		if (x == snake[i].x && y == snake[i].y)
			return 1;
	}
	return 0;
}

// Initializes apple creation
void apple_init()
{
	apple.x = 6;
	apple.y = 4;
}

// Interrupt service routines

// Refresh matrix
ISR(TCC0_OVF_vect)
{
	// Displays next row
	matrix_turnon(led_matrix[row_count],row_count);
	row_count++;
	if (row_count == 8)
	{
		row_count = 0;
	}
}

// Auto-move snake
ISR(TCD1_OVF_vect)
{
	if ((input == 'u' && movedir != 'd') || (input == 'd' && movedir != 'u') || (input == 'r' && movedir != 'l') || (input == 'l' && movedir != 'r'))
		move_snake(input);
	else
	{
		move_snake(movedir);
	}
}

// X axis ADC
ISR(ADCA_CH0_vect)
{
	x_axis = ADCA.CH0.RES;
}

// Y axis ADC
ISR(ADCA_CH1_vect)
{
	y_axis = ADCA.CH1.RES;
}