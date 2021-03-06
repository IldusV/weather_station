/*!
 *  @brief Example shows basic application to configure and read the temperature.
 */
#include <string.h>
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "temperature.h"


#include "bmp280.h"

//#ifdef __KERNEL__
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
//#endif

void delay_ms(uint32_t period_ms);
int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
void print_rslt(const char api_name[], int8_t rslt);

int8_t fd;
struct bmp280_dev bmp;

int init_temp_sensor(void)
{
    int8_t rslt;
    
    struct bmp280_config conf;
    // struct bmp280_uncomp_data ucomp_data;
    // int32_t temp32;
    double temp;

    // if (argc < 2)
    // {
    //     printf(stderr, "Missing argument for i2c bus.\n");
    //     exit(1);
    // }

    if ((fd = open("/dev/i2c-1", O_RDWR)) < 0)
    {
        printf(stderr, "Failed to open the i2c bus %s\n", "/dev/i2c-1");
        exit(1);
    }

//#ifdef __KERNEL__
    if (ioctl(fd, I2C_SLAVE, 0x76) < 0)
    {
        printf(stderr, "Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }
//#endif

    /* Map the delay function pointer with the function responsible for implementing the delay */
    bmp.delay_ms = delay_ms;

    /* Assign device I2C address based on the status of SDO pin (GND for PRIMARY(0x76) & VDD for SECONDARY(0x77)) */
    bmp.dev_id = BMP280_I2C_ADDR_PRIM;

    /* Select the interface mode as I2C */
    bmp.intf = BMP280_I2C_INTF;

    /* Map the I2C read & write function pointer with the functions responsible for I2C bus transfer */
    bmp.read = i2c_reg_read;
    bmp.write = i2c_reg_write;

    /* To enable SPI interface: comment the above 4 lines and uncomment the below 4 lines */

    /*
     * bmp.dev_id = 0;
     * bmp.read = spi_reg_read;
     * bmp.write = spi_reg_write;
     * bmp.intf = BMP280_SPI_INTF;
     */
    rslt = bmp280_init(&bmp);
    print_rslt(" bmp280_init status", rslt);

    /* Always read the current settings before writing, especially when
     * all the configuration is not modified
     */
    rslt = bmp280_get_config(&conf, &bmp);
    print_rslt(" bmp280_get_config status", rslt);

    /* configuring the temperature oversampling, filter coefficient and output data rate */
    /* Overwrite the desired settings */
    conf.filter = BMP280_FILTER_COEFF_2;

    /* Temperature oversampling set at 4x */
    conf.os_temp = BMP280_OS_4X;

    /* Pressure over sampling none (disabling pressure measurement) */
    conf.os_pres = BMP280_OS_4X;
    //conf.os_pres = BMP280_OS_16X;
    /* Setting the output data rate as 1HZ(1000ms) */
    conf.odr = BMP280_ODR_1000_MS;
    rslt = bmp280_set_config(&conf, &bmp);
    print_rslt(" bmp280_set_config status", rslt);

    /* Always set the power mode after setting the configuration */
    //rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);
    print_rslt(" bmp280_set_power_mode status", rslt);
    // while (1)
    // {
        /* Reading the raw data from sensor */
        // rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);

        // /* Getting the 32 bit compensated temperature */
        // rslt = bmp280_get_comp_temp_32bit(&temp32, ucomp_data.uncomp_temp, &bmp);

        // /* Getting the compensated temperature as floating point value */
        // rslt = bmp280_get_comp_temp_double(&temp, ucomp_data.uncomp_temp, &bmp);
        // printf("UT: %ld, T32: %ld, T: %f \r\n", ucomp_data.uncomp_temp, temp32, temp);

        /* Sleep time between measurements = BMP280_ODR_1000_MS */
    //     bmp.delay_ms(1000);
    // }

    return 0;
}

int bmp280_get_temp(void)
{
    struct bmp280_uncomp_data ucomp_data = {0};
    int32_t temp32 = 0;
    int8_t rslt;

    rslt = bmp280_set_power_mode(BMP280_FORCED_MODE, &bmp);
    delay_ms(20);

    rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);
    rslt = bmp280_get_comp_temp_32bit(&temp32, ucomp_data.uncomp_temp, &bmp);

    printf("UT: %ld, T32: %ld\r\n", ucomp_data.uncomp_temp, temp32);
    print_rslt("bmp280_get_comp_temp_32bit status", rslt);

    return temp32;
}

double bmp280_get_pressure(void)
{
    struct bmp280_uncomp_data ucomp_data = {0};
    int8_t rslt;
    uint32_t pres32, pres64;
    double pres;

    rslt = bmp280_set_power_mode(BMP280_FORCED_MODE, &bmp);
    delay_ms(20);

    /* Reading the raw data from sensor */
    rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);

    /* Getting the compensated pressure using 32 bit precision */
    rslt = bmp280_get_comp_pres_32bit(&pres32, ucomp_data.uncomp_press, &bmp);

    /* Getting the compensated pressure using 64 bit precision */
    rslt = bmp280_get_comp_pres_64bit(&pres64, ucomp_data.uncomp_press, &bmp);

    /* Getting the compensated pressure as floating point value */
    rslt = bmp280_get_comp_pres_double(&pres, ucomp_data.uncomp_press, &bmp);
    printf("UP: %ld, P32: %ld, P64: %ld, P64N: %ld, P: %f\r\n",
           ucomp_data.uncomp_press,
           pres32,
           pres64,
           pres64 / 256,
           pres);
    //bmp.delay_ms(1000); /* Sleep time between measurements = BMP280_ODR_1000_MS */
    return pres;
}

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs such as "bmg250_soft_reset",
 *      "bmg250_set_foc", "bmg250_perform_self_test"  and so on.
 *
 *  @param[in] period_ms  : the required wait time in milliseconds.
 *  @return void.
 *
 */
void delay_ms(uint32_t period_ms)
{
    usleep(period_ms*2000);
    printf("delay %d", period_ms);
}

/*!
 *  @brief Function for writing the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[in] reg_data : Pointer to the data buffer whose value is to be written.
 *  @param[in] length   : No of bytes to write.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
    uint8_t *buf;
    //struct identifier id;

    //id = *((struct identifier *)intf_ptr);

    buf = malloc(length + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, reg_data, length);
    if (write(fd, buf, length + 1) < (uint16_t)length)
    {
        return BMP280_E_COMM_FAIL;
    }
    delay_ms(15);
    printf(">>> write %x", reg_addr);
    free(buf);

    return BMP280_OK;

    /* Implement the I2C write routine according to the target machine. */
    //return -1;
}

/*!
 *  @brief Function for reading the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : Sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[out] reg_data    : Pointer to the data buffer to store the read data.
 *  @param[in] length   : No of bytes to read.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
    reg_data[1] = 0;
    reg_data[0] = reg_addr; 
    write(fd, reg_data, 1);
    delay_ms(1);
    uint8_t nn = read(fd, reg_data, length);

    //printf("\nread addr:%x value:%x, bytes:%d \n", reg_addr, reg_data[0], nn);
    printf("\nread addr: %x value: %x %x %x %x %x %x, bytes:%d \n", reg_addr, reg_data[0],reg_data[1],reg_data[2],reg_data[3],reg_data[4],reg_data[5], nn);
    if(nn==length)
       return 0;
    else
       return -1;
    /* Implement the I2C read routine according to the target machine. */
    //return -1;
}

void close_temp(void)
{
    printf("closing temp...\n\r");
    close(fd);
}

/*!
 *  @brief Function for writing the sensor's registers through SPI bus.
 *
 *  @param[in] cs           : Chip select to enable the sensor.
 *  @param[in] reg_addr     : Register address.
 *  @param[in] reg_data : Pointer to the data buffer whose data has to be written.
 *  @param[in] length       : No of bytes to write.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{

    /* Implement the SPI write routine according to the target machine. */
    return -1;
}

/*!
 *  @brief Function for reading the sensor's registers through SPI bus.
 *
 *  @param[in] cs       : Chip select to enable the sensor.
 *  @param[in] reg_addr : Register address.
 *  @param[out] reg_data    : Pointer to the data buffer to store the read data.
 *  @param[in] length   : No of bytes to read.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{

    /* Implement the SPI read routine according to the target machine. */
    return -1;
}

/*!
 *  @brief Prints the execution status of the APIs.
 *
 *  @param[in] api_name : name of the API whose execution status has to be printed.
 *  @param[in] rslt     : error code returned by the API whose execution status has to be printed.
 *
 *  @return void.
 */
void print_rslt(const char api_name[], int8_t rslt)
{
    if (rslt != BMP280_OK)
    {
        printf("%s\t", api_name);
        if (rslt == BMP280_E_NULL_PTR)
        {
            printf("Error [%d] : Null pointer error\r\n", rslt);
        }
        else if (rslt == BMP280_E_COMM_FAIL)
        {
            printf("Error [%d] : Bus communication failed\r\n", rslt);
        }
        else if (rslt == BMP280_E_IMPLAUS_TEMP)
        {
            printf("Error [%d] : Invalid Temperature\r\n", rslt);
        }
        else if (rslt == BMP280_E_DEV_NOT_FOUND)
        {
            printf("Error [%d] : Device not found\r\n", rslt);
        }
        else
        {
            /* For more error codes refer "*_defs.h" */
            printf("Error [%d] : Unknown error code\r\n", rslt);
        }
    }
}
