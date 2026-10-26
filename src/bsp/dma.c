#include "bsp/dma.h"
#include "stm32f446xx.h"
#include <stddef.h>

void DMA_Init(void)
{
    // Enable DMA1 clock
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    // Configure DMA1 Stream 5 for DAC channel 7
    DMA1_Stream5->CR &= ~DMA_SxCR_EN; // Disable the stream
    while (DMA1_Stream5->CR & DMA_SxCR_EN); // Wait until the stream is disabled

    DMA1_Stream5->CR = 0; // Clear the control register
    DMA1_Stream5->NDTR = 0; // Clear the number of data items to transfer
    DMA1_Stream5->PAR = (uint32_t)&DAC->DHR12R1; // Set peripheral address to DAC channel 1 data holding register
    DMA1_Stream5->M0AR = 0; // Clear memory address register
    DMA1_Stream5->FCR = 0; // Clear FIFO control register

    // Configure the DMA stream
    DMA1_Stream5->CR |=
    (7U << DMA_SxCR_CHSEL_Pos) | // Channel 7
    DMA_SxCR_DIR_0            | // Memory -> Peripheral
    DMA_SxCR_MINC             | // Memory increment
    DMA_SxCR_PSIZE_0          | // Peripheral size = 16 bits
    DMA_SxCR_MSIZE_0          | // Memory size = 16 bits
    DMA_SxCR_CIRC;              // Circular mode
}

void DMA_StopDac(void)
{
    // Disable the DMA stream
    DMA1_Stream5->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream5->CR & DMA_SxCR_EN); // Wait until the stream is disabled
}

bool DMA_StartDac(const uint16_t *data, uint32_t length)
{
        if ((data == NULL) || (length == 0U) || (length > 0xFFFFU))
    {
        return false;
    }
    // Disable the DMA stream before configuring it
    DMA1_Stream5->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream5->CR & DMA_SxCR_EN); // Wait until the stream is disabled

    // Clear all pending DMA1 Stream 5 flags before configuring the new transfer
    DMA1->HIFCR =
    DMA_HIFCR_CFEIF5  |
    DMA_HIFCR_CDMEIF5 |
    DMA_HIFCR_CTEIF5  |
    DMA_HIFCR_CHTIF5  |
    DMA_HIFCR_CTCIF5;

    // Set the number of data items to transfer
    DMA1_Stream5->NDTR = length;

    // Set the memory address to the data buffer
    DMA1_Stream5->M0AR = (uint32_t)data;

    // Enable the DMA stream
    DMA1_Stream5->CR |= DMA_SxCR_EN;
    return true;
}

