#include "device.h"
#include "driverlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "Myprint.h"
#include "ad9959.h"
#include "ad7606.h"
#include "FFTv2.h"
/**
 * main.c
 */

#define EPWM1_TIMER_TBPRD   5000U
#define EPWM1_MAX_CMPA      2500U
#define EPWM1_MIN_CMPA      2500U

#define EPWM_CMP_UP           1U
#define EPWM_CMP_DOWN         0U

int NPT = 4096;

typedef struct
{
    uint32_t epwmModule;
    uint16_t epwmCompADirection;
    uint16_t epwmTimerIntCount;
    uint16_t epwmMaxCompA;
    uint16_t epwmMinCompA;
}epwmInformation;

// Globals to hold the ePWM information used in this example
epwmInformation epwm1Info;

void Init(void);
void mySCIAconfig(void); //GPIO 28 29 SCIARX中断 波特率115200 接收一位16进制并回送
__interrupt void sciaRxISR(void);
void initEPWM1(void);
__interrupt void epwm1ISR(void);
__interrupt void gpioInterruptHandler(void);
void updateCompare(epwmInformation *epwmInfo);
void SPI_init();
void PinMux_init();

uint16_t msg=0; // 串口接收到的数据
int16_t ad7606_data[8] = { 0 };
uint8_t flag = 0;
int32_t num = 0;

int16_t adc_buf1[NPT_max] = { 0 };
int16_t adc_buf2[NPT_max] = { 0 };
int16_t adc_buf3[200] = { 0 };

FFTresult ch1, ch2;
float32_t phase = 0, dc1 = 0, dc2 = 0, vp1 = 0, vp2 = 0;

float32_t D = 0, C = 0;
float32_t fre = 600000;
float32_t dc3 = 0, dc_store = 0, fre_step = 100000;
float32_t Q = 0, L = 0;

float vin = 0;

float vpp_test = 200;

char mode = 'C';

int main(void)
{
    Init();

    ad9959_init();
    ad7606_init();

    FFT_Init();

    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);  // Push-pull output or floating input
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_PULLUP);  // Push-pull output or floating input
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);



    // Disable global interrupts.
    //初始化电容模式下信号源的激励

    ad9959_write_phase(AD9959_CHANNEL_2, 0); //0°
    ad9959_write_frequency(AD9959_CHANNEL_2, 1000); //1kHz
    ad9959_write_amplitude(AD9959_CHANNEL_2, 512); //1V


    while(1)
    {
        if (GPIO_readPin(14) == 0 && mode == 'L') //L模式初始化
        {
            num = 0;
            mode = 'C';

            print("L.txt=\"0\"\xff\xff\xff", 0);
            print("Q.txt=\"0\"\xff\xff\xff", 0);
        }
        if (GPIO_readPin(14) == 1 && mode == 'C') //C模式初始化
        {
            num = 0;
            fre = 600000;
            fre_step = 100000;
            ad9959_write_phase(AD9959_CHANNEL_3, 0);
            ad9959_write_frequency(AD9959_CHANNEL_3, fre);
            ad9959_write_amplitude(AD9959_CHANNEL_3, 512);
            mode = 'L';

            print("C.txt=\"0\"\xff\xff\xff", 0);
            print("D.txt=\"0\"\xff\xff\xff", 0);
        }
        if (mode == 'C')
        {
            if (num == NPT)
            {
                //stop pwm
                EPWM_setCounterCompareValue(epwm1Info.epwmModule, EPWM_COUNTER_COMPARE_A, 0); //停止PWM，停止AD7606采样

                int j = 0; //CCS抽象循环变量，不能用for(int j = 0; j < NPT; j++)

                DEVICE_DELAY_US(1); 

                FFT_int16(adc_buf1, NPT, &ch1); //通道一：Ua-Ub，
                FFT_int16(adc_buf2, NPT, &ch2); //通道二：Ub
                int x1 = 1, x2 = 1;
                float32_t max1 = Complex_Modular(ch1.result[1]) / NPT * 2, max2 = Complex_Modular(ch2.result[1]) / NPT * 2; //找基频
                for (j = 2; j < NPT / 2; j++)
                {
                    if (Complex_Modular(ch1.result[j]) / NPT * 2 > max1)
                    {
                        x1 = j;
                        max1 = Complex_Modular(ch1.result[j]) / NPT * 2;
                    }
                    if (Complex_Modular(ch2.result[j]) / NPT *2 > max2)
                    {
                        x2 = j;
                        max2 = Complex_Modular(ch2.result[j]) / NPT * 2;
                    }
                }

                phase = (Complex_arg(ch1.result[x1]) - Complex_arg(ch2.result[x2])) / pi * 180; //相位差
                if (phase > 180)
                {
                    phase -= 360;
                }
                if (phase < -180)
                {
                    phase += 360;
                }

                float32_t sum1 = 0, sum2 = 0;
                for (j = 0; j < NPT; j++) //计算直流分量
                {
                    sum1 += (adc_buf1[j] * 5.0f / 32767);
                    sum2 += (adc_buf2[j] * 5.0f / 32767);
                }
                dc1 = sum1 / NPT;
                dc2 = sum2 / NPT;

                float32_t rms1 = 0, rms2 = 0;
                for (j = 0; j < NPT; j++) //计算有效值
                {
                    rms1 += pow(adc_buf1[j] * 5.0f / 32767 - dc1, 2);
                    rms2 += pow(adc_buf2[j] * 5.0f / 32767 - dc2, 2);
                }
                vp1 = sqrt(rms1 / NPT) * sqrt(2);
                vp2 = sqrt(rms2 / NPT) * sqrt(2);
    //            print("phase: {.2}, ", phase);
                D = tanf((phase - 90) / 180 * pi); //计算D
                print("D.txt=\"{.4}\"\xff\xff\xff", D); 
                C = 1000000.0f / (2 * pi * 159.5 * cosf((phase - 90) / 180 * pi)) * (vp2 / vp1); //计算C
                print("C.txt=\"{.2}nF\"\xff\xff\xff", C);
                ad7606_reset(); //复位AD7606
                num = 0;

                //start pwm
                EPWM_setCounterCompareValue(epwm1Info.epwmModule, EPWM_COUNTER_COMPARE_A, EPWM1_MAX_CMPA); //开始PWM，开始AD7606采样
            }
        }
        if (mode == 'L')
        {
            if (num == 200)
            {
                EPWM_setCounterCompareValue(epwm1Info.epwmModule, EPWM_COUNTER_COMPARE_A, 0); //停止PWM，停止AD7606采样
                DEVICE_DELAY_US(1);

                float32_t sum = 0;
                int j = 0;
                for (j = 0; j < 200; j++) 
                {
                    sum += (adc_buf3[j] * 5.0f / 32767);
                }
                dc3 = sum / 200; //AD8307的输出直流分量
                if (dc3 < dc_store)
                {
                    fre -= (2 * fre_step);
                    if (fre_step > 1000)
                    {
                        fre_step /= 10;
                    }
                    else//到达最小分辨率1000Hz
                    {
                        L = 1.0f / (pow(2 * pi * fre, 2) * 420 * pow(10, -18));
                        vin = 2.573 * pow(10, -5) * pow(2.718, 4.69 * dc3);
                        Q = vin / 3.95 / 0.0036 * 1.1;
                        print("L.txt=\"{.2}uF\"\xff\xff\xff", L);
                        print("Q.txt=\"{.2}\"\xff\xff\xff", Q);
                        fre = 600000;
                        fre_step = 100000;
                    }
                    dc_store = 0;
                }
                else
                {
                    dc_store = dc3;
                    fre += fre_step;
                }

                ad9959_write_frequency(AD9959_CHANNEL_3, fre);
                ad7606_reset();
                DEVICE_DELAY_US(10000);
                num = 0;
                EPWM_setCounterCompareValue(epwm1Info.epwmModule, EPWM_COUNTER_COMPARE_A, EPWM1_MAX_CMPA);
            }
        }
    }

}

void Init(void)
{
    Device_init();
    Device_initGPIO();
    Interrupt_initModule();
    Interrupt_initVectorTable();

    GPIO_setPadConfig(34, GPIO_PIN_TYPE_PULLUP);  // Push-pull output or floating input
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_IN);

    DINT;

    // Initialize interrupt controller and vector table.
    Interrupt_initModule();
    Interrupt_initVectorTable();
    IER = 0x0000;
    IFR = 0x0000;
    Interrupt_register(INT_EPWM1, &epwm1ISR);

    // 设置 GPIO0/1 为 ePWM1A/1B
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_0_EPWM1A);

    // 关同步 Disable sync(Freeze clock to PWM as well)
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    initEPWM1();
    // 开同步 Enable sync and clock to PWM
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // 使能中断
    Interrupt_enable(INT_EPWM1);

    mySCIAconfig();

    GPIO_setInterruptType(GPIO_INT_XINT1, GPIO_INT_TYPE_FALLING_EDGE);
    GPIO_setInterruptPin(34, GPIO_INT_XINT1);
    GPIO_enableInterrupt(GPIO_INT_XINT1);

    EALLOW;
    PinMux_init();
    SPI_init();
    EDIS;

    Interrupt_register(INT_XINT1, &gpioInterruptHandler);
    Interrupt_enable(INT_XINT1);
    // Enable global interrupts.
    EINT;
    ERTM;
}

void mySCIAconfig(void)
{
    /*
    SCIA配置函数，GPIO28为Rx，GPIO29为Tx
        波特率115200，映射并启用SCIA_RX中断
    */
    // GPIO28 is the SCI Rx pin.
    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCIRXDA, GPIO_CORE_CPU1);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCIRXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCIRXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_QUAL_ASYNC);

    // GPIO29 is the SCI Tx pin.
    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCITXDA, GPIO_CORE_CPU1);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCITXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCITXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_QUAL_ASYNC);

    // Map the ISR to the wake interrupt.
    Interrupt_register(INT_SCIA_RX, sciaRxISR);

    // Initialize SCIA and its FIFO.
    SCI_performSoftwareReset(SCIA_BASE);

    // 波特率115200 8位数据 1位停止 无校验位
    SCI_setConfig(SCIA_BASE, 25000000, 115200, (SCI_CONFIG_WLEN_8 |
            SCI_CONFIG_STOP_ONE |
            SCI_CONFIG_PAR_NONE));

    SCI_resetChannels(SCIA_BASE);
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXRDY_BRKDT);
    SCI_enableModule(SCIA_BASE);
    SCI_performSoftwareReset(SCIA_BASE);

    // Enable the TXRDY and RXRDY interrupts.
    SCI_enableInterrupt(SCIA_BASE, SCI_INT_RXRDY_BRKDT);

    // Clear the SCI interrupts before enabling them.
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXRDY_BRKDT);

    // Enable the interrupts in the PIE: Group 9 interrupts 1 & 2.
    Interrupt_enable(INT_SCIA_RX);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}


// sciaRxISR - Read the character from the RXBUF and echo it back.
__interrupt void sciaRxISR(void)
{
    uint16_t receivedChar;

    // Read a character from the RXBUF.
    receivedChar = SCI_readCharBlockingNonFIFO(SCIA_BASE);
    msg=receivedChar; // 更新全局变量

    // Echo back the character.
    SCI_writeCharBlockingNonFIFO(SCIA_BASE, receivedChar);

    // Acknowledge the PIE interrupt.
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}

__interrupt void epwm1ISR(void) // 中断服务程序
{
    // 更新比较值 Update the CMPA and CMPB values
    //updateCompare(&epwm1Info);

    // Clear INT flag for this timer
    EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);

    // Acknowledge interrupt group
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

__interrupt void gpioInterruptHandler(void)
{
    int i = 0;

    for (i = 0; i < 8; i++)
    {
        SPI_writeDataNonBlocking(SPIB_BASE, 0x00);
        ad7606_data[i] = (int16_t)SPI_readDataNonBlocking(SPIB_BASE);
    }

    if (mode == 'C')
    {
        if (num < NPT)
        {
            adc_buf1[num] = ad7606_data[0];
            adc_buf2[num++] = ad7606_data[1];
        }
    }
    if (mode == 'L')
    {
        if (num < 200)
        {
            adc_buf3[num++] = ad7606_data[2];
        }
    }

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

// 初始化 initEPWM1 - Configure ePWM1

void initEPWM1()
{
    // 设置ePWM的时钟TBCLK 在100MHz的主频下，每一个TBCLK对应 10ns
    EPWM_setTimeBasePeriod(EPWM1_BASE, EPWM1_TIMER_TBPRD);
    EPWM_setPhaseShift(EPWM1_BASE, 0U);
    EPWM_setTimeBaseCounter(EPWM1_BASE, 0U);

    // 设置CMPA和CMPB
    EPWM_setCounterCompareValue(EPWM1_BASE,
                                EPWM_COUNTER_COMPARE_A,
                                EPWM1_MIN_CMPA);

    // 设置为增减计数模式，所以PWM的周期是 10ns*2000*2=40us，对应25kHz
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_disablePhaseShiftLoad(EPWM1_BASE);
    EPWM_setClockPrescaler(EPWM1_BASE,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

    // 设置影子寄存器，在计数为0时装载
    EPWM_setCounterCompareShadowLoadMode(EPWM1_BASE,
                                         EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);

    // 设置ePWM动作方式
    // 例:对于EPWM1A,当增计数==CMPA时输出高,当减计数==CMPA时输出低
    EPWM_setActionQualifierAction(EPWM1_BASE,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(EPWM1_BASE,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    // 设置触发中断的时刻,这里设置为计数器为0时触发。
    // 使能中断。
    // 设置每隔3次触发一次中断，上限15次。
    EPWM_setInterruptSource(EPWM1_BASE, EPWM_INT_TBCTR_ZERO);
    EPWM_enableInterrupt(EPWM1_BASE);
    EPWM_setInterruptEventCount(EPWM1_BASE, 3U);

    //
    // Information this example uses to keep track of the direction the
    // CMPA/CMPB values are moving, the min and max allowed values and
    // a pointer to the correct ePWM registers
    //
    epwm1Info.epwmCompADirection = EPWM_CMP_UP;
    epwm1Info.epwmTimerIntCount = 0U;
    epwm1Info.epwmModule = EPWM1_BASE;
    epwm1Info.epwmMaxCompA = EPWM1_MAX_CMPA;
    epwm1Info.epwmMinCompA = EPWM1_MIN_CMPA;
}

// 更新比较值
void updateCompare(epwmInformation *epwmInfo)
{
    uint16_t compAValue;

    compAValue = EPWM_getCounterCompareValue(epwmInfo->epwmModule,
                                             EPWM_COUNTER_COMPARE_A);

    //
    //  Change the CMPA/CMPB values every 10th interrupt.
    //
    if(epwmInfo->epwmTimerIntCount == 10U)
    {
        epwmInfo->epwmTimerIntCount = 0U;

        //
        // If we were increasing CMPA, check to see if we reached the max
        // value.  If not, increase CMPA else, change directions and decrease
        // CMPA
        //
        if(epwmInfo->epwmCompADirection == EPWM_CMP_UP)
        {
            if(compAValue < (epwmInfo->epwmMaxCompA))
            {
                EPWM_setCounterCompareValue(epwmInfo->epwmModule,
                                            EPWM_COUNTER_COMPARE_A,
                                            ++compAValue);
            }
            else
            {
                epwmInfo->epwmCompADirection = EPWM_CMP_DOWN;
                EPWM_setCounterCompareValue(epwmInfo->epwmModule,
                                            EPWM_COUNTER_COMPARE_A,
                                            --compAValue);
            }
        }
        //
        // If we were decreasing CMPA, check to see if we reached the min
        // value.  If not, decrease CMPA else, change directions and increase
        // CMPA
        //
        else
        {
            if( compAValue == (epwmInfo->epwmMinCompA))
            {
                epwmInfo->epwmCompADirection = EPWM_CMP_UP;
                EPWM_setCounterCompareValue(epwmInfo->epwmModule,
                                            EPWM_COUNTER_COMPARE_A,
                                            ++compAValue);
            }
            else
            {
                EPWM_setCounterCompareValue(epwmInfo->epwmModule,
                                            EPWM_COUNTER_COMPARE_A,
                                            --compAValue);
            }
        }

        //
        // If we were increasing CMPB, check to see if we reached the max
        // value.  If not, increase CMPB else, change directions and decrease
        // CMPB
        //

    }
    else
    {
        epwmInfo->epwmTimerIntCount++;
    }
}

void PinMux_init()
{
    //
    // SPIB -> SPIB_master Pinmux
    //
    GPIO_setPinConfig(GPIO_24_SPIB_SIMO);
    GPIO_setPinConfig(GPIO_31_SPIB_SOMI);
    GPIO_setPinConfig(GPIO_22_SPIB_CLK);
    GPIO_setPinConfig(GPIO_27_SPIB_STE);

}

void SPI_init()
{
    //SPIB_master initialization
    SPI_disableModule(SPIB_BASE);
    SPI_setConfig(SPIB_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL1PHA1,
                  SPI_MODE_MASTER, 2000000,  16);
    SPI_disableLoopback(SPIB_BASE);
    SPI_enableFIFO(SPIB_BASE);
    SPI_enableModule(SPIB_BASE);
}
