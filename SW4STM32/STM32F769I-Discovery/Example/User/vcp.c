/*
 *  More portable code things
 */

#include "vcp.h"

#define Default_Error_Message() Error_Message("I'm sorry Dave, I'm afraid I can't do that.")

// Uart HAL structure
UART_HandleTypeDef UartHandle;

void Error_Message(char *);

// UART initialization
void vcp_init(void) {
	UartHandle.Instance = USARTx;

	UartHandle.Init.BaudRate = 115200;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits = UART_STOPBITS_1;
	UartHandle.Init.Parity = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode = UART_MODE_TX_RX;
	if (HAL_UART_DeInit(&UartHandle) != HAL_OK) {
		Error_Message("Unable to turn off UART");
	}
	if (HAL_UART_Init(&UartHandle) != HAL_OK) {
		Error_Message("Unable to turn on UART");
	}
}

// Collects characters until size limit or an endline is recieved
// Returns number of characters collected
int vcp_getline(char * input, int max_size) {
	char single;
	int i = 0;

	do {
		single = vcp_getchar(1); // get one character

		if (i >= max_size)
			break;

		if (single != '\n')
			input[i] = single;
		else
			break;

		i++;

	} while (1);

	return i;
}

// Get one character
char vcp_getchar(uint8_t echo) {
	char input[8] = "00000000"; // holdover from previous code
	HAL_StatusTypeDef halstat;

	do {
		// Query UART in blocking mode with timeout of 1s
		halstat = HAL_UART_Receive(&UartHandle, (uint8_t *) input, 1, 1000);
	} while (halstat == HAL_TIMEOUT); // Loop if nothing was recieved

	// Status handling
	if (halstat == HAL_OK) // Good to go
			{
		if (input[0] == '\r' || input[0] == '\n') // handling for dumb windows endlines
				{
			if (echo)
				vcp_puts("\r\n");
			return '\n';
		} else {
			if (echo)
				vcp_print(input, 1);
			return (char) input[0];
		}
	} else if (halstat == HAL_TIMEOUT) // This code should no longer be reached.  If it is the world is broken.
		Error_Message(
				"Magic Bullshit has occurred.  Send Doug a note on Slack.");
	else if (halstat == HAL_BUSY) // Somebody is hogging the channel
		Error_Message(
				"This mission is too important for me to allow you to jeopardize it.");

	Default_Error_Message();  // Go to the error handler if any errors occurred.
	return 255;
}

// Print a string over uart channel
void vcp_puts(char * string) {
	// Find location of null terminator character
	uint8_t length = 0;
	while (length < 255 && string[length] != '\0')
		length++;

	if (length == 255)
		vcp_print("puts() size limit exceeded\r\n", 28);
	else
		vcp_print(string, length);
}

// wrapper for original print command that sets some defaults like UartHandle and a 1s timeout
// useful for strings that are not null terminated but the size is known.
void vcp_print(char * string, uint8_t length) {
	if (length > 0)
		if (HAL_UART_Transmit(&UartHandle, (uint8_t *) string, length, 100)
				!= HAL_OK)
			Default_Error_Message();
}

void Error_Message(char * msg) {
	vcp_puts("\r\n!!! Fatal Error !!!\r\n\r\nHAL says: ");
	vcp_puts(msg);
	vcp_puts("\r\n");
}
