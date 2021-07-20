#ifdef __cplusplus
extern "C" {
#endif

int bmp280_get_temp(void);
double bmp280_get_pressure(void);
int init_temp_sensor(void);
void delay_ms(unsigned int period_ms);
void close_temp(void);

#ifdef __cplusplus
}
#endif /* End of CPP guard */
