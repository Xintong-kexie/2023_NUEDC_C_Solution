#ifndef __AD7606_H
#define __AD7606_H
#include "device.h"
#include "driverlib.h"

#define AD7606_OS0_0   GPIO_writePin(18, 0)
#define AD7606_OS0_1   GPIO_writePin(18, 1)
#define AD7606_OS1_0   GPIO_writePin(27, 0)
#define AD7606_OS1_1   GPIO_writePin(27, 1)
#define AD7606_OS2_0   GPIO_writePin(59, 0)
#define AD7606_OS2_1   GPIO_writePin(59, 1)
#define AD7606_SER_0   GPIO_writePin(6, 0)
#define AD7606_SER_1   GPIO_writePin(6, 1)
#define AD7606_RST_0   GPIO_writePin(2, 0)
#define AD7606_RST_1   GPIO_writePin(2, 1)

#define read()  GPIO_readPin(31)

void ad7606_delay(uint32_t length);
void ad7606_init(void);
void ad7606_reset(void);

#endif
