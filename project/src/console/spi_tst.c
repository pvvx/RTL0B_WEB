/*
 * spi_test.c
 */
#include <device.h>
#include "spi_api.h"
#include "spi_ex_api.h"
#include "console_api.h"
#include "rtl_lib.h"

#define SPI0_MOSI  PA_23
#define SPI0_MISO  PA_22
#define SPI0_SCLK  PA_18
#define SPI0_CS    PA_19

#define error_printf rtl_printf
#define LOCAL static

spi_t spi_master;


#define HAL_SSI_READ32(i, x) HAL_READ32(SPI0_REG_BASE + i*0x400, x)

LOCAL void show_reg_spi(int i) {
	rtl_printf("Regs SPI:\n");
	for(int x = 0; x < 64 ; x += 4) {
		rtl_printf("0x%08x ", HAL_SSI_READ32(i, x));
		if((x & 0x0F) == 0x0C) rtl_printf("\n");
	}
}

//"ATSSI", 0, fATSSI, "=[clk[,count[,len]]: Spi test"
// Support maximum 2 SPI port
// Support Master (SPI1 only), and Slave(SPI0 only) mode
// high speed SPI with baud rate up to 31.25MHz
// atssi 31250000 1000 131072
void fATSSI(int argc, char *argv[])
{
    int len = 128;
    int count = 32;
    int clk = 10000000;
    int ssn = 0;
    if(argc > 1) {
        clk = atoi(argv[1]);
        if(clk <= 1000 || clk > 31250000) {
        	clk = 10000000;
            error_printf("%s: clk = %u!\n", __func__, clk);
            return;
        };
    };
    if(argc > 2) {
        count = atoi(argv[2]);
        if(count > 10000 || count <= 0) {
        	count = 32;
            error_printf("%s: count = %u!\n", __func__, count);
        };
    };
    if(argc > 3) {
        len = atoi(argv[3]);
        if(len > 128*1024 || len <= 0) {
        	len = 128;
            error_printf("%s: len = %u!\n", __func__, len);
        };
    };
    rtl_printf("Set Test SPI1:  clk = %u, blkcount = %u, blklen = %u\r\n", clk, count, len);
    char* buff = pvPortMalloc(len);
    if(buff) {
    	memset(buff, 0x55, len);
    	spi_master.spi_idx=MBED_SPI1;
    	spi_init(&spi_master, SPI0_MOSI, SPI0_MISO, SPI0_SCLK, SPI0_CS); // заданные ноги тут не используются!
        spi_format(&spi_master, 16, 3, 0);
        spi_frequency(&spi_master, clk);
       	for(int i = 0; i < len; i++) buff[i] = (char)i;
        while(count--) {
            spi_master_write_stream(&spi_master, buff, len);
            while(spi_busy(&spi_master));
            rtl_printf("Master write: %d\n", count);
        };
		show_reg_spi(1);
        spi_free(&spi_master);
        free(buff);
    }
    else {
        error_printf("%s: error malloc!\n", __func__);
    };
}

MON_RAM_TAB_SECTION MON_COMMAND_TABLE console_commands_spitst[] = {
		{"ATSSI", 0, fATSSI, "=[clk[,count[,len]]: Spi test"}
};
