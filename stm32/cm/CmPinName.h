//  Defines the libopencm3 Pin Names that will substituted for the mbed Pin Names.
#ifndef CODAL_CM_PINNAME_H
#define CODAL_CM_PINNAME_H
#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus

enum PinName {
//  PinName should not exceed 254.
//  From https://docs.google.com/spreadsheets/d/1yLciHFyPfhRfwEcG3wfqHDgRFa5OoQYTk63bUmTboa8/edit#gid=1888249278

//  Pin Names are autogenerated. Do not update here
CM_PIN_PA0 = 0,
CM_PIN_PA1 = 1,
CM_PIN_PA2 = 2,
CM_PIN_PA3 = 3,
CM_PIN_PA4 = 4,
CM_PIN_PA5 = 5,
CM_PIN_PA6 = 6,
CM_PIN_PA7 = 7,
CM_PIN_PA8 = 8,
CM_PIN_PA9 = 9,
CM_PIN_PA10 = 10,
CM_PIN_PA11 = 11,
CM_PIN_PA12 = 12,
CM_PIN_PA15 = 13,
CM_PIN_PB0 = 14,
CM_PIN_PB1 = 15,
CM_PIN_PB3 = 16,
CM_PIN_PB4 = 17,
CM_PIN_PB5 = 18,
CM_PIN_PB6 = 19,
CM_PIN_PB7 = 20,
CM_PIN_PB8 = 21,
CM_PIN_PB9 = 22,
CM_PIN_PB10 = 23,
CM_PIN_PB11 = 24,
CM_PIN_PB12 = 25,
CM_PIN_PB13 = 26,
CM_PIN_PB14 = 27,
CM_PIN_PB15 = 28,
CM_PIN_PC10 = 29,
CM_PIN_PC11 = 30,
CM_PIN_PC12 = 31,
CM_PIN_PD2 = 32,
CM_PIN_PD3 = 33,
CM_PIN_PD4 = 34,
CM_PIN_PD5 = 35,
CM_PIN_PD6 = 36,
CM_PIN_PD7 = 37,
CM_PIN_PD12 = 38,
CM_PIN_PD13 = 39,
CM_PIN_PD14 = 40,
CM_PIN_PD15 = 41,
CM_PIN_TIM_CH15 = 42,
CM_PIN_TIM_CH25 = 43,
CM_PIN_TIM_CH35 = 44,
CM_PIN_TIM_CH45 = 45,
CM_PIN_TIM_CH14 = 46,
CM_PIN_TIM_CH24 = 47,
CM_PIN_TIM_CH34 = 48,
CM_PIN_TIM_CH44 = 49,
CM_PIN_TIM_CH14_REMAP = 50,
CM_PIN_TIM_CH24_REMAP = 51,
CM_PIN_TIM_CH34_REMAP = 52,
CM_PIN_TIM_CH44_REMAP = 53,
CM_PIN_TIM_CH13 = 54,
CM_PIN_TIM_CH23 = 55,
CM_PIN_TIM_CH33 = 56,
CM_PIN_TIM_CH43 = 57,
CM_PIN_TIM_CH22 = 58,
CM_PIN_TIM_CH32 = 59,
CM_PIN_TIM_CH42 = 60,
CM_PIN_TIM_ETR1 = 61,
CM_PIN_TIM_CH11 = 62,
CM_PIN_TIM_CH21 = 63,
CM_PIN_TIM_CH31 = 64,
CM_PIN_TIM_CH41 = 65,
CM_PIN_TIM_BKIN1 = 66,
CM_PIN_TIM_CH1N1 = 67,
CM_PIN_TIM_CH2N1 = 68,
CM_PIN_TIM_CH3N1 = 69,
CM_PIN_UART_TX5 = 70,
CM_PIN_UART_RX5 = 71,
CM_PIN_UART_TX4 = 72,
CM_PIN_UART_RX4 = 73,
CM_PIN_USART_TX3 = 74,
CM_PIN_USART_RX3 = 75,
CM_PIN_USART_CK3 = 76,
CM_PIN_USART_CTS3 = 77,
CM_PIN_USART_RTS3 = 78,
CM_PIN_USART_CTS2 = 79,
CM_PIN_USART_RTS2 = 80,
CM_PIN_USART_TX2 = 81,
CM_PIN_USART_RX2 = 82,
CM_PIN_USART_CK2 = 83,
CM_PIN_USART_CTS2_REMAP = 84,
CM_PIN_USART_RTS2_REMAP = 85,
CM_PIN_USART_TX2_REMAP = 86,
CM_PIN_USART_RX2_REMAP = 87,
CM_PIN_USART_CK2_REMAP = 88,
CM_PIN_USART_TX1 = 89,
CM_PIN_USART_RX1 = 90,
CM_PIN_USART_TX1_REMAP = 91,
CM_PIN_USART_RX1_REMAP = 92,
CM_PIN_I2C_SMBAI1 = 93,
CM_PIN_I2C_SCL1 = 94,
CM_PIN_I2C_SDA1 = 95,
CM_PIN_I2C_SMBAI1_REMAP = 96,
CM_PIN_I2C_SCL1_REMAP = 97,
CM_PIN_I2C_SDA1_REMAP = 98,
CM_PIN_I2C_SCL2 = 99,
CM_PIN_I2C_SDA2 = 100,
CM_PIN_I2C_SMBAI2 = 101,
CM_PIN_SPI_NSS1 = 102,
CM_PIN_SPI_SCK1 = 103,
CM_PIN_SPI_MISO1 = 104,
CM_PIN_SPI_MOSI1 = 105,
CM_PIN_SPI_NSS1_REMAP = 106,
CM_PIN_SPI_SCK1_REMAP = 107,
CM_PIN_SPI_MISO1_REMAP = 108,
CM_PIN_SPI_MOSI1_REMAP = 109,
CM_PIN_SPI_NSS2 = 110,
CM_PIN_SPI_SCK2 = 111,
CM_PIN_SPI_MISO2 = 112,
CM_PIN_SPI_MOSI2 = 113,
CM_PIN_SPI_NSS3 = 114,
CM_PIN_SPI_SCK3 = 115,
CM_PIN_SPI_MISO3 = 116,
CM_PIN_SPI_MOSI3 = 117,
CM_PIN_SPI_NSS3_REMAP = 118,
CM_PIN_SPI_SCK3_REMAP = 119,
CM_PIN_SPI_MISO3_REMAP = 120,
CM_PIN_SPI_MOSI3_REMAP = 121,
CM_PIN_LED = 122,
CM_PIN_TEMPERATURE = 123,
CM_PIN_VREF = 124,
CM_PIN_VBAT = 125,

//  End of autogenerated section
CM_PIN_NC = 0xff  //  Not connected
};
typedef enum PinName PinName;

#ifdef __cplusplus
}
#endif  //  __cplusplus
#endif  //  CODAL_CM_PINNAME_H
