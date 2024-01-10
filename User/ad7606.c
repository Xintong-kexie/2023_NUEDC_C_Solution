#include "ad7606.h"

void ad7606_delay(uint32_t length) {
    //length = length;
    while (length--);
}

void ad7606_init(void)
{
    int pin[5] = {2, 6, 18, 27, 59};
    int i;
    for (i = 0; i < 5; i++)
    {
        GPIO_setPadConfig(pin[i], GPIO_PIN_TYPE_STD);  // Push-pull output or floating input
        GPIO_setDirectionMode(pin[i], GPIO_DIR_MODE_OUT);
    }

    AD7606_OS0_0;
    AD7606_OS1_0;
    AD7606_OS2_0;
    AD7606_SER_1;
    ad7606_reset();
}

void ad7606_reset(void)
{
    AD7606_RST_0;
    DEVICE_DELAY_US(1000);
    AD7606_RST_1;
    DEVICE_DELAY_US(1000);
    AD7606_RST_0;
}
