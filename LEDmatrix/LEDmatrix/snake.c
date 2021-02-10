#include <avr/io.h>
#include <avr/interrupt.h>
#include "matrix.h"
#include "joystick.h"
#include "display_consts.h"

#define NULL 0
#define TCD1_AUTOMOVE_PER (6500)



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

volatile struct point snake[64];
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
void apple_new(void);
void display_score(uint8_t score);

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
		/*
		if (collide == 1)
			display_image(collide_wall_image);
		else if (collide == 2)
			display_image(collide_self_image);
		input = get_direction();
		*/
		
		// Disabling automove
		TCD1.INTCTRLA = TC_OVFINTLVL_OFF_gc;
		
		display_score(length-3);

		input = get_direction();
		
		if (input != 0)
		{
			collide = 0;
			snake_init();
			automove_init();
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
	for (uint8_t i = 0; i < 64; i++)
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

	// Initialize head and tail
	tail = &snake[0];
	head = &snake[2];
	
	length = 3;
	
	movedir = 'r';
	
	apple_init();
}

// Draw snake on LED Matrix
void draw_snake()
{	
	// Draw snake
	for (uint8_t i = 0; i < length; i++)
	{
		// If snake value exists then set value
		if (snake[i].x != -1 && snake[i].y != -1)
		{
			led_matrix[7-snake[i].y] |= (0x80 >> snake[i].x);
		}
	}

	// Draw apple
	led_matrix[7-apple.y] |= (0x80 >> apple.x);
}

// Move snake in certain direction and marks
void move_snake(char dir)
{
	uint8_t apple_eaten = 0;
	
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
		// Collide into apple
		else if (head->x == apple.x && head->y+1 == apple.y)
		{
			// Add apple to the head of the snake
			snake[length].x = apple.x;
			snake[length].y = apple.y;
			
			// Previous head will point to new head
			head->next = &snake[length];
			
			// Set new head
			head = &snake[length];
		
			length++;
			
			movedir = 'u';
			
			apple_eaten = 1;		
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
		// Collide into apple
		else if (head->x == apple.x && head->y-1 == apple.y)
		{
			// Add apple to the head of the snake
			snake[length].x = apple.x;
			snake[length].y = apple.y;
			
			// Previous head will point to new head
			head->next = &snake[length];
			
			// Set new head
			head = &snake[length];
		
			length++;
			
			movedir = 'd';
			
			apple_eaten = 1;		
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
		// Collide into apple
		else if (head->x+1 == apple.x && head->y == apple.y)
		{
			// Add apple to the head of the snake
			snake[length].x = apple.x;
			snake[length].y = apple.y;
			
			// Previous head will point to new head
			head->next = &snake[length];
			
			// Set new head
			head = &snake[length];
		
			length++;
			
			movedir = 'r';
			
			apple_eaten = 1;		
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
		// Collide into apple
		else if (head->x-1 == apple.x && head->y == apple.y)
		{
			// Add apple to the head of the snake
			snake[length].x = apple.x;
			snake[length].y = apple.y;
			
			// Previous head will point to new head
			head->next = &snake[length];
			
			// Set new head
			head = &snake[length];
		
			length++;
			
			movedir = 'l';
			
			apple_eaten = 1;		
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

	if (apple_eaten)
	{
		apple_new();
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
	apple.y = 3;
}

// Generates location of new apple
void apple_new()
{
	uint8_t x = TCC0.CNTL & (0b00000111);
	uint8_t y = (TCC0.CNTL >> 1) & (0b00000111);
	
	while ((in_snake(x,y)))
	{
		x++;
		if (x == 8)
			x = 0;
		y++;
		if (y == 8)
			y = 0;
	}
	
	apple.x = x;
	apple.y = y;
}

void display_score(uint8_t score)
{
	uint8_t ones = score%10;
	uint8_t tens = (score/10);
	
	uint8_t both_digits[8];
	
	for (uint8_t i = 0; i < 8; i++)
	{
		uint8_t tens_display = digits[tens][i] << 5;
		both_digits[i] = tens_display | digits[ones][i];
	}
	
	display_image(both_digits);
			
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
	// Preventing from moving into itself
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