/* Include libMPSSE header */
#include "stdafx.h"

/* Include D2XX header*/
#include "spixio.h"
#include "WinTypes.h"
#include "ftd2xx.h"
#include "libMPSSE_spi.h"

static FT_HANDLE ftHandle;
static uint8 buffer[SPI_DEVICE_BUFFER_SIZE] = {0};

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);return 0;}else{;}};
#define APP_CHECK_STATUS_INV(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);return 1;}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);return 0;}else{;}};
#define CHECK_NULL_INV(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);return 1;}else{;}};

uint32_t GSPI_INPUTS;
uint32_t GSPI_ADDR;
uint32_t GSPI_OUTPUTS;
uint32_t GSPI_WORD;
uint8_t  GSPI_DONE;
uint32_t GSPI_MAXOBJ;
uint32_t GSPI_SPI_ADDR_BITS;
char GSPI_NAME[500];

int clogb2(int value)
{
	int 	i;
	int clogb = 0;
	for(i = 0; (1<<i) < value; i = i + 1)
		clogb = i + 1;
	return clogb;
}

int read_write_single_word(uint32_t address, uint32_t sizeaddr, uint32_t *data, uint32_t databits)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	FT_STATUS status;
	int i, n, s;

	/* CS_High + Write command + Address */
	sizeToTransfer=GSPI_SPI_COMM_READ_BITS;
	sizeTransfered=0;
	buffer[0] = GSPI_TASK_RW << 6;	/* Write command (2bit, 6-bit displaced)*/
	n = 0; s = 5;
	for(i = ((int)sizeaddr-1); i >= 0; i--)
	{
        buffer[n] |= ((address >> i) & 0x1) << s;
        if(s == 0) {s = 7; n++; buffer[n] = 0;} else s--;
	}

	for(i = ((int)databits-1); i >= 0; i--)
	{
        buffer[n] |= (((*data) >> i) & 0x1) << s;
        if(s == 0) {s = 7; n++; buffer[n] = 0;} else s--;
	}
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);

	/*Read the data*/
	sizeToTransfer=GSPI_SPI_READ_SEND_BITS;
	sizeTransfered=0;
	status = SPI_Read(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);

	n = 0; s = 7;
	(*data) = 0;
	for(i = ((int)databits-1); i >= 0; i--)  /*IS NOT FUCKING LITTLE ENDIAN*/
	{
        (*data) |= ((buffer[n] >> s) & 0x1) << i;
        if(s == 0) {s = 7; n++;} else s--;
	}

	/* Dummy Bits, for issue aditional clk cycles */
	sizeToTransfer=GSPI_SPI_DUMMY_BITS;
	sizeTransfered=0;
	buffer[0] = 0;
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS);
	APP_CHECK_STATUS(status);

	return 1;
}

int write_single_word(uint32_t address, uint32_t sizeaddr, uint32_t data, uint32_t databits)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	FT_STATUS status;
	int i, n, s;

	/* CS_High + Write command + Address */
	sizeToTransfer=GSPI_SPI_COMM_WRITE_BITS;
	sizeTransfered=0;
	buffer[0] = GSPI_TASK_WRITE << 6;	/* Write command (2bit, 6-bit displaced)*/
	n = 0; s = 5;
	for(i = ((int)sizeaddr-1); i >= 0; i--)
	{
        buffer[n] |= ((address >> i) & 0x1) << s;
        if(s == 0) {s = 7; n++; buffer[n] = 0;} else s--;
	}

	for(i = ((int)databits-1); i >= 0; i--)
	{
        buffer[n] |= ((data >> i) & 0x1) << s;
        if(s == 0) {s = 7; n++; buffer[n] = 0;} else s--;
	}
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);

	/* Dummy Bits, for issue aditional clk cycles */
	sizeToTransfer=GSPI_SPI_DUMMY_BITS;
	sizeTransfered=0;
	buffer[0] = 0;
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS);
	APP_CHECK_STATUS(status);

	return 1;
}

int read_single_word(uint32_t address, uint32_t sizeaddr, uint32_t* data, uint32_t databits)
{
	uint32 sizeToTransfer = 0;
	uint32 sizeTransfered;
	FT_STATUS status;
	int i, n, s;

	/* CS_High + Write command + Address */
	sizeToTransfer=GSPI_SPI_COMM_READ_BITS;
	sizeTransfered=0;
	buffer[0] = GSPI_TASK_READ << 6;	/* Write command (2bit, 6-bit displaced)*/
	n = 0; s = 5;
	for(i = ((int)sizeaddr-1); i >= 0; i--)
	{
        buffer[n] |= ((address >> i) & 0x1) << s;
        if(s == 0) {s = 7; n++; buffer[n] = 0;} else s--;
	}

	for(i = ((int)databits-1); i >= 0; i--)
	{
        buffer[n] |= ((0 >> i) & 0x1) << s;
        if(s == 0) {s = 7; n++; buffer[n] = 0;} else s--;
	}
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);

	/*Read the data*/
	sizeToTransfer=GSPI_SPI_READ_SEND_BITS;
	sizeTransfered=0;
	status = SPI_Read(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS|
		SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
	APP_CHECK_STATUS(status);

	n = 0; s = 7;
	(*data) = 0;
	for(i = ((int)databits-1); i >= 0; i--)  /*IS NOT FUCKING LITTLE ENDIAN*/
	{
        (*data) |= ((buffer[n] >> s) & 0x1) << i;
        if(s == 0) {s = 7; n++;} else s--;
	}

	/* Dummy Bits, for issue aditional clk cycles */
	sizeToTransfer=GSPI_SPI_DUMMY_BITS;
	sizeTransfered=0;
	buffer[0] = 0;
	status = SPI_Write(ftHandle, buffer, sizeToTransfer, &sizeTransfered,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BITS);
	APP_CHECK_STATUS(status);

	return 1;
}

int spi_init(void)
{
    strcpy(GSPI_NAME, "No Device");
    GSPI_DONE = 0;
    FT_STATUS status = FT_OK;
	FT_DEVICE_LIST_INFO_NODE devList = {0};
	ChannelConfig channelConf = {0};
	uint32 channels = 0;
	uint8 latency = 255;
	uint32 i;
	uint32 dev_to_open;

    channelConf.ClockRate = SPI_CLOCK_RATE_HZ;
	channelConf.LatencyTimer = latency;
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0x00000000;	/*According to manual, this is not needed because directions of pins are overriden in SPI*/

    /* init library */
#ifdef _MSC_VER
	Init_libMPSSE();
#endif
	status = SPI_GetNumChannels(&channels);
	APP_CHECK_STATUS(status);
	printf("Number of available SPI channels = %d\n",(int)channels);

	if(channels>0)
	{
		// NO, WE ARENT GOING TO FUCKING OPEN THE FIRST AVAILABLE
		// WE'LL SEARCH OUR PROGRAMMER
		dev_to_open = 0xFFFFFFFF;
		for(i = 0; i < channels; i++){
			status = SPI_GetChannelInfo (i,&devList);
			APP_CHECK_STATUS(status);

			if(devList.Flags == 0x2 && devList.Type == 0x8 && devList.ID == 0x4036001)
			{
				dev_to_open = i;
				break;
			}

		}
		if(dev_to_open == 0xFFFFFFFF) return 0;

		status = SPI_OpenChannel(dev_to_open,&ftHandle);
		APP_CHECK_STATUS(status);

		status = SPI_InitChannel(ftHandle,&channelConf);
	}
	else return 0;

    GSPI_DONE = 1;
    sprintf(GSPI_NAME, "SPI(%u) 0x%x 0x%x 0x%x 0x%x (0x%x), %s(%s) \n",
        i,devList.Flags,devList.Type,devList.ID,devList.LocId,devList.ftHandle,devList.Description,devList.SerialNumber);
    return 1;
}

void spi_close(void)
{
    SPI_CloseChannel(ftHandle);

#ifdef _MSC_VER
	Cleanup_libMPSSE();
#endif
}
