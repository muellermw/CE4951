/*
 * uart_driver.c
 *
 *      Author: Marcus Mueller
 */
#include "uart_driver.h"
#include "ringbuffer.h"
#include "stm32f4xx.h"
#include "stm32f4xx_nucleo.h"

static RingBuffer RECEIVE_BUFFER = {0,0};
static RingBuffer TRANSMIT_BUFFER = {0,0};
static UART_HandleTypeDef hUART2;


/*
 * Utilizes receive buffer to get incoming characters
 */
char usart2_getch(){
	char read_char = get(&RECEIVE_BUFFER); // get next char from receive buffer
	usart2_putch(read_char); // dump to the transmit buffer for echoing

	if (read_char == '\r'){  // If character is CR
		read_char = '\n';   // Return LF. fgets is terminated by LF
		usart2_putch(read_char); // dump to the transmit buffer for echoing
	} else if (read_char == '\b') {
		// if there is a backspace (ANSI), send characters to make the console behave correctly
		// clear the character space with a 'space', then back up to that space
		usart2_putch(' ');
		usart2_putch('\b');
	}
	return read_char;
}

/*
 * Utilizes the transmit buffer to send a character out
 * Sets TXEIE flag
 */
void usart2_putch(char c){
	put(&TRANSMIT_BUFFER, c);
	__HAL_UART_ENABLE_IT(&hUART2, UART_IT_TXE);
}

void usart2_init(uint32_t baud){
	__USART2_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();

	hUART2.Instance = USART2;
	hUART2.Init.BaudRate = baud;
	hUART2.Init.WordLength = UART_WORDLENGTH_8B;
	// 0 stop bits
	hUART2.Init.StopBits = UART_STOPBITS_1;
	hUART2.Init.Parity = UART_PARITY_NONE;
	hUART2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	hUART2.Init.Mode = UART_MODE_TX_RX;

	// enable PA2 and PA3 in alternate function 7 mode
	GPIO_InitTypeDef hPA2_3;
	hPA2_3.Pin = GPIO_PIN_2;
	hPA2_3.Mode = GPIO_MODE_AF_PP;
	hPA2_3.Alternate = GPIO_AF7_USART2;
	hPA2_3.Speed = GPIO_SPEED_HIGH;
	hPA2_3.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &hPA2_3);

	hPA2_3.Pin = GPIO_PIN_3;
	hPA2_3.Mode = GPIO_MODE_AF_OD;
	HAL_GPIO_Init(GPIOA, &hPA2_3);

	// init UART2 and its interrupt
	HAL_UART_Init(&hUART2);
	__HAL_UART_ENABLE_IT(&hUART2, UART_IT_RXNE);
	NVIC_EnableIRQ(USART2_IRQn);
}

/*
 * IRQ takes care of RXNEIE and TXEIE,
 * adds incoming and outgoing characters to
 * the receive and transmit buffers
 */
void USART2_IRQHandler(void){
	uint32_t status = *(USART_SR);
	if ( ((status >> 5) & 1) == 1 ) { // RXNE
		put(&RECEIVE_BUFFER, (char) *USART_DR);  // Read character from usart
	} else if ( ((status >> 7) & 1) == 1 ) { // TXE
		if (!hasElement(&TRANSMIT_BUFFER)) { // last element? disable interrupt
			*(USART_CR1) &= ~(1<<7);
		} else {
			*(USART_DR) = get(&TRANSMIT_BUFFER); // else keep sending out
		}
	}
}
