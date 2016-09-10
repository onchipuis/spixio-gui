#ifndef SPIXIO_H
#define SPIXIO_H

extern uint32_t GSPI_INPUTS;
extern uint32_t GSPI_OUTPUTS;
extern uint32_t GSPI_WORD;
extern uint32_t GSPI_ADDR;
extern uint32_t GSPI_MAXOBJ;
extern uint8_t GSPI_DONE;
extern char GSPI_NAME[500];

/* Application specific macro definations */
#define CHANNEL_TO_OPEN				0	/*0 for first available channel, 1 for next... */
#define SPI_DEVICE_BUFFER_SIZE		256
#define SPI_CLOCK_RATE_HZ			10000
//#define GSPI_INPUTS					6
//#define GSPI_OUTPUTS				8
//#define GSPI_WORD					8
#define FUNC_GSPI_MAXOBJ			(GSPI_OUTPUTS>GSPI_INPUTS?GSPI_OUTPUTS:GSPI_INPUTS);
#define GSPI_SPI_DUMMY_BITS			3
#define GSPI_SPI_TASK_BITS			2
#define FUNC_GSPI_SPI_ADDR_BITS 	clogb2(GSPI_MAXOBJ);
#define GSPI_SPI_DATA_BITS 			databits
#define GSPI_SPI_ADDR_BITS          sizeaddr
#define GSPI_SPI_COMM_WRITE_BITS	(GSPI_SPI_TASK_BITS+GSPI_SPI_ADDR_BITS+GSPI_SPI_DATA_BITS)
#define GSPI_SPI_COMM_READ_BITS		(GSPI_SPI_TASK_BITS+GSPI_SPI_ADDR_BITS+GSPI_SPI_DATA_BITS+1)
#define GSPI_SPI_READ_SEND_BITS		GSPI_SPI_DATA_BITS
#define GSPI_TASK_READ				0x1
#define GSPI_TASK_WRITE				0x2
#define GSPI_TASK_RW				0x3

int read_write_single_word(uint32_t address, uint32_t sizeaddr, uint32_t* data, uint32_t databits);
int write_single_word(uint32_t address, uint32_t sizeaddr, uint32_t data, uint32_t databits);
int read_single_word(uint32_t address, uint32_t sizeaddr, uint32_t* data, uint32_t databits);
int spi_init(void);
void spi_close(void);
int clogb2(int value);

#endif // SPIXIO_H
