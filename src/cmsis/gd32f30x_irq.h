/*
 * gd32f30x_irq.h
 * @brief 
 * Created on: Apr 24, 2022
 * Author: Yanye
 */

#ifndef CMSIS_GD32F30X_IRQ_H_
#define CMSIS_GD32F30X_IRQ_H_

/* configuration of the Cortex-M4 processor and core peripherals */
#define __CM4_REV                 0x0001   /*!< Core revision r0p1                                       */
#define __MPU_PRESENT             1        /*!< GD32F30x provide MPU                                     */
#define __NVIC_PRIO_BITS          4        /*!< GD32F30x uses 4 bits for the priority levels             */
#define __Vendor_SysTickConfig    0        /*!< set to 1 if different sysTick config is used             */
#define __FPU_PRESENT             1        /*!< FPU present                                              */
#define __VFP_FP__                1

/* define interrupt number */
typedef enum IRQn
{
    /* Cortex-M4 processor exceptions numbers */
    NonMaskableInt_IRQn          = -14,    /*!< 2 non maskable interrupt                                 */
    MemoryManagement_IRQn        = -12,    /*!< 4 Cortex-M4 memory management interrupt                  */
    BusFault_IRQn                = -11,    /*!< 5 Cortex-M4 bus fault interrupt                          */
    UsageFault_IRQn              = -10,    /*!< 6 Cortex-M4 usage fault interrupt                        */
    SVCall_IRQn                  = -5,     /*!< 11 Cortex-M4 SV call interrupt                           */
    DebugMonitor_IRQn            = -4,     /*!< 12 Cortex-M4 debug monitor interrupt                     */
    PendSV_IRQn                  = -2,     /*!< 14 Cortex-M4 pend SV interrupt                           */
    SysTick_IRQn                 = -1,     /*!< 15 Cortex-M4 system tick interrupt                       */
    /* interruput numbers */
    WWDGT_IRQn                   = 0,      /*!< window watchDog timer interrupt                          */
    LVD_IRQn                     = 1,      /*!< LVD through EXTI line detect interrupt                   */
    TAMPER_IRQn                  = 2,      /*!< tamper through EXTI line detect                          */
    RTC_IRQn                     = 3,      /*!< RTC through EXTI line interrupt                          */
    FMC_IRQn                     = 4,      /*!< FMC interrupt                                            */
    RCU_CTC_IRQn                 = 5,      /*!< RCU and CTC interrupt                                    */
    EXTI0_IRQn                   = 6,      /*!< EXTI line 0 interrupt                                    */
    EXTI1_IRQn                   = 7,      /*!< EXTI line 1 interrupt                                    */
    EXTI2_IRQn                   = 8,      /*!< EXTI line 2 interrupt                                    */
    EXTI3_IRQn                   = 9,      /*!< EXTI line 3 interrupt                                    */
    EXTI4_IRQn                   = 10,     /*!< EXTI line 4 interrupt                                    */
    DMA0_Channel0_IRQn           = 11,     /*!< DMA0 channel0 interrupt                                  */
    DMA0_Channel1_IRQn           = 12,     /*!< DMA0 channel1 interrupt                                  */
    DMA0_Channel2_IRQn           = 13,     /*!< DMA0 channel2 interrupt                                  */
    DMA0_Channel3_IRQn           = 14,     /*!< DMA0 channel3 interrupt                                  */
    DMA0_Channel4_IRQn           = 15,     /*!< DMA0 channel4 interrupt                                  */
    DMA0_Channel5_IRQn           = 16,     /*!< DMA0 channel5 interrupt                                  */
    DMA0_Channel6_IRQn           = 17,     /*!< DMA0 channel6 interrupt                                  */
    ADC0_1_IRQn                  = 18,     /*!< ADC0 and ADC1 interrupt                                  */
#ifdef GD32F30X_HD
    USBD_HP_CAN0_TX_IRQn         = 19,     /*!< CAN0 TX interrupts                                       */
    USBD_LP_CAN0_RX0_IRQn        = 20,     /*!< CAN0 RX0 interrupts                                      */
    CAN0_RX1_IRQn                = 21,     /*!< CAN0 RX1 interrupt                                       */
    CAN0_EWMC_IRQn               = 22,     /*!< CAN0 EWMC interrupt                                      */
    EXTI5_9_IRQn                 = 23,     /*!< EXTI[9:5] interrupts                                     */
    TIMER0_BRK_IRQn              = 24,     /*!< TIMER0 break interrupt                                   */
    TIMER0_UP_IRQn               = 25,     /*!< TIMER0 update interrupt                                  */
    TIMER0_TRG_CMT_IRQn          = 26,     /*!< TIMER0 trigger and commutation interrupt                 */
    TIMER0_Channel_IRQn          = 27,     /*!< TIMER0 channel capture compare interrupt                 */
    TIMER1_IRQn                  = 28,     /*!< TIMER1 interrupt                                         */
    TIMER2_IRQn                  = 29,     /*!< TIMER2 interrupt                                         */
    TIMER3_IRQn                  = 30,     /*!< TIMER3 interrupt                                         */
    I2C0_EV_IRQn                 = 31,     /*!< I2C0 event interrupt                                     */
    I2C0_ER_IRQn                 = 32,     /*!< I2C0 error interrupt                                     */
    I2C1_EV_IRQn                 = 33,     /*!< I2C1 event interrupt                                     */
    I2C1_ER_IRQn                 = 34,     /*!< I2C1 error interrupt                                     */
    SPI0_IRQn                    = 35,     /*!< SPI0 interrupt                                           */
    SPI1_IRQn                    = 36,     /*!< SPI1 interrupt                                           */
    USART0_IRQn                  = 37,     /*!< USART0 interrupt                                         */
    USART1_IRQn                  = 38,     /*!< USART1 interrupt                                         */
    USART2_IRQn                  = 39,     /*!< USART2 interrupt                                         */
    EXTI10_15_IRQn               = 40,     /*!< EXTI[15:10] interrupts                                   */
    RTC_Alarm_IRQn               = 41,     /*!< RTC alarm interrupt                                      */
    USBD_WKUP_IRQn               = 42,     /*!< USBD Wakeup interrupt                                    */
    TIMER7_BRK_IRQn              = 43,     /*!< TIMER7 break interrupt                                   */
    TIMER7_UP_IRQn               = 44,     /*!< TIMER7 update interrupt                                  */
    TIMER7_TRG_CMT_IRQn          = 45,     /*!< TIMER7 trigger and commutation interrupt                 */
    TIMER7_Channel_IRQn          = 46,     /*!< TIMER7 channel capture compare interrupt                 */
    ADC2_IRQn                    = 47,     /*!< ADC2 global interrupt                                    */
    EXMC_IRQn                    = 48,     /*!< EXMC global interrupt                                    */
    SDIO_IRQn                    = 49,     /*!< SDIO global interrupt                                    */
    TIMER4_IRQn                  = 50,     /*!< TIMER4 global interrupt                                  */
    SPI2_IRQn                    = 51,     /*!< SPI2 global interrupt                                    */
    UART3_IRQn                   = 52,     /*!< UART3 global interrupt                                   */
    UART4_IRQn                   = 53,     /*!< UART4 global interrupt                                   */
    TIMER5_IRQn                  = 54,     /*!< TIMER5 global interrupt                                  */
    TIMER6_IRQn                  = 55,     /*!< TIMER6 global interrupt                                  */
    DMA1_Channel0_IRQn           = 56,     /*!< DMA1 channel0 global interrupt                           */
    DMA1_Channel1_IRQn           = 57,     /*!< DMA1 channel1 global interrupt                           */
    DMA1_Channel2_IRQn           = 58,     /*!< DMA1 channel2 global interrupt                           */
    DMA1_Channel3_Channel4_IRQn  = 59,     /*!< DMA1 channel3 and channel4 global Interrupt              */
#endif /* GD32F30X_HD */

#ifdef GD32F30X_XD
    USBD_HP_CAN0_TX_IRQn         = 19,     /*!< CAN0 TX interrupts                                       */
    USBD_LP_CAN0_RX0_IRQn        = 20,     /*!< CAN0 RX0 interrupts                                      */
    CAN0_RX1_IRQn                = 21,     /*!< CAN0 RX1 interrupt                                       */
    CAN0_EWMC_IRQn               = 22,     /*!< CAN0 EWMC interrupt                                      */
    EXTI5_9_IRQn                 = 23,     /*!< EXTI[9:5] interrupts                                     */
    TIMER0_BRK_TIMER8_IRQn       = 24,     /*!< TIMER0 break and TIMER8 interrupt                        */
    TIMER0_UP_TIMER9_IRQn        = 25,     /*!< TIMER0 update and TIMER9 interrupt                       */
    TIMER0_TRG_CMT_TIMER10_IRQn  = 26,     /*!< TIMER0 trigger and commutation and TIMER10 interrupt     */
    TIMER0_Channel_IRQn          = 27,     /*!< TIMER0 channel capture compare interrupt                 */
    TIMER1_IRQn                  = 28,     /*!< TIMER1 interrupt                                         */
    TIMER2_IRQn                  = 29,     /*!< TIMER2 interrupt                                         */
    TIMER3_IRQn                  = 30,     /*!< TIMER3 interrupt                                         */
    I2C0_EV_IRQn                 = 31,     /*!< I2C0 event interrupt                                     */
    I2C0_ER_IRQn                 = 32,     /*!< I2C0 error interrupt                                     */
    I2C1_EV_IRQn                 = 33,     /*!< I2C1 event interrupt                                     */
    I2C1_ER_IRQn                 = 34,     /*!< I2C1 error interrupt                                     */
    SPI0_IRQn                    = 35,     /*!< SPI0 interrupt                                           */
    SPI1_IRQn                    = 36,     /*!< SPI1 interrupt                                           */
    USART0_IRQn                  = 37,     /*!< USART0 interrupt                                         */
    USART1_IRQn                  = 38,     /*!< USART1 interrupt                                         */
    USART2_IRQn                  = 39,     /*!< USART2 interrupt                                         */
    EXTI10_15_IRQn               = 40,     /*!< EXTI[15:10] interrupts                                   */
    RTC_Alarm_IRQn               = 41,     /*!< RTC alarm interrupt                                      */
    USBD_WKUP_IRQn               = 42,     /*!< USBD wakeup interrupt                                    */
    TIMER7_BRK_TIMER11_IRQn      = 43,     /*!< TIMER7 break and TIMER11 interrupt                       */
    TIMER7_UP_TIMER12_IRQn       = 44,     /*!< TIMER7 update and TIMER12 interrupt                      */
    TIMER7_TRG_CMT_TIMER13_IRQn  = 45,     /*!< TIMER7 trigger and commutation and TIMER13 interrupt     */
    TIMER7_Channel_IRQn          = 46,     /*!< TIMER7 channel capture compare interrupt                 */
    ADC2_IRQn                    = 47,     /*!< ADC2 global interrupt                                    */
    EXMC_IRQn                    = 48,     /*!< EXMC global interrupt                                    */
    SDIO_IRQn                    = 49,     /*!< SDIO global interrupt                                    */
    TIMER4_IRQn                  = 50,     /*!< TIMER4 global interrupt                                  */
    SPI2_IRQn                    = 51,     /*!< SPI2 global interrupt                                    */
    UART3_IRQn                   = 52,     /*!< UART3 global interrupt                                   */
    UART4_IRQn                   = 53,     /*!< UART4 global interrupt                                   */
    TIMER5_IRQn                  = 54,     /*!< TIMER5 global interrupt                                  */
    TIMER6_IRQn                  = 55,     /*!< TIMER6 global interrupt                                  */
    DMA1_Channel0_IRQn           = 56,     /*!< DMA1 channel0 global interrupt                           */
    DMA1_Channel1_IRQn           = 57,     /*!< DMA1 channel1 global interrupt                           */
    DMA1_Channel2_IRQn           = 58,     /*!< DMA1 channel2 global interrupt                           */
    DMA1_Channel3_Channel4_IRQn  = 59,     /*!< DMA1 channel3 and channel4 global interrupt              */
#endif /* GD32F30X_XD */

#ifdef GD32F30X_CL
    CAN0_TX_IRQn                 = 19,     /*!< CAN0 TX interrupt                                        */
    CAN0_RX0_IRQn                = 20,     /*!< CAN0 RX0 interrupt                                       */
    CAN0_RX1_IRQn                = 21,     /*!< CAN0 RX1 interrupt                                       */
    CAN0_EWMC_IRQn               = 22,     /*!< CAN0 EWMC interrupt                                      */
    EXTI5_9_IRQn                 = 23,     /*!< EXTI[9:5] interrupts                                     */
    TIMER0_BRK_TIMER8_IRQn       = 24,     /*!< TIMER0 break and TIMER8 interrupt                        */
    TIMER0_UP_TIMER9_IRQn        = 25,     /*!< TIMER0 update and TIMER9 interrupt                       */
    TIMER0_TRG_CMT_TIMER10_IRQn  = 26,     /*!< TIMER0 trigger and commutation and TIMER10 interrupt     */
    TIMER0_Channel_IRQn          = 27,     /*!< TIMER0 channel capture compare interrupt                 */
    TIMER1_IRQn                  = 28,     /*!< TIMER1 interrupt                                         */
    TIMER2_IRQn                  = 29,     /*!< TIMER2 interrupt                                         */
    TIMER3_IRQn                  = 30,     /*!< TIMER3 interrupt                                         */
    I2C0_EV_IRQn                 = 31,     /*!< I2C0 event interrupt                                     */
    I2C0_ER_IRQn                 = 32,     /*!< I2C0 error interrupt                                     */
    I2C1_EV_IRQn                 = 33,     /*!< I2C1 event interrupt                                     */
    I2C1_ER_IRQn                 = 34,     /*!< I2C1 error interrupt                                     */
    SPI0_IRQn                    = 35,     /*!< SPI0 interrupt                                           */
    SPI1_IRQn                    = 36,     /*!< SPI1 interrupt                                           */
    USART0_IRQn                  = 37,     /*!< USART0 interrupt                                         */
    USART1_IRQn                  = 38,     /*!< USART1 interrupt                                         */
    USART2_IRQn                  = 39,     /*!< USART2 interrupt                                         */
    EXTI10_15_IRQn               = 40,     /*!< EXTI[15:10] interrupts                                   */
    RTC_Alarm_IRQn               = 41,     /*!< RTC alarm interrupt                                      */
    USBFS_WKUP_IRQn              = 42,     /*!< USBFS wakeup interrupt                                   */
    TIMER7_BRK_TIMER11_IRQn      = 43,     /*!< TIMER7 break and TIMER11 interrupt                       */
    TIMER7_UP_TIMER12_IRQn       = 44,     /*!< TIMER7 update and TIMER12 interrupt                      */
    TIMER7_TRG_CMT_TIMER13_IRQn  = 45,     /*!< TIMER7 trigger and commutation and TIMER13 interrupt     */
    TIMER7_Channel_IRQn          = 46,     /*!< TIMER7 channel capture compare interrupt                 */
    EXMC_IRQn                    = 48,     /*!< EXMC global interrupt                                    */
    TIMER4_IRQn                  = 50,     /*!< TIMER4 global interrupt                                  */
    SPI2_IRQn                    = 51,     /*!< SPI2 global interrupt                                    */
    UART3_IRQn                   = 52,     /*!< UART3 global interrupt                                   */
    UART4_IRQn                   = 53,     /*!< UART4 global interrupt                                   */
    TIMER5_IRQn                  = 54,     /*!< TIMER5 global interrupt                                  */
    TIMER6_IRQn                  = 55,     /*!< TIMER6 global interrupt                                  */
    DMA1_Channel0_IRQn           = 56,     /*!< DMA1 channel0 global interrupt                           */
    DMA1_Channel1_IRQn           = 57,     /*!< DMA1 channel1 global interrupt                           */
    DMA1_Channel2_IRQn           = 58,     /*!< DMA1 channel2 global interrupt                           */
    DMA1_Channel3_IRQn           = 59,     /*!< DMA1 channel3 global interrupt                           */
    DMA1_Channel4_IRQn           = 60,     /*!< DMA1 channel3 global interrupt                           */
    ENET_IRQn                    = 61,     /*!< ENET global interrupt                                    */
    ENET_WKUP_IRQn               = 62,     /*!< ENET Wakeup interrupt                                    */
    CAN1_TX_IRQn                 = 63,     /*!< CAN1 TX interrupt                                        */
    CAN1_RX0_IRQn                = 64,     /*!< CAN1 RX0 interrupt                                       */
    CAN1_RX1_IRQn                = 65,     /*!< CAN1 RX1 interrupt                                       */
    CAN1_EWMC_IRQn               = 66,     /*!< CAN1 EWMC interrupt                                      */
    USBFS_IRQn                   = 67,     /*!< USBFS global interrupt                                   */
#endif /* GD32F30X_CL */

} IRQn_Type;

#endif /* CMSIS_GD32F30X_IRQ_H_ */
