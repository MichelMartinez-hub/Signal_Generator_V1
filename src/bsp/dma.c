#include "bsp/dma.h"
#include "stm32f446xx.h"
#include <stddef.h>

static DMA_DAC_Callback dma_callback = NULL;
static uint16_t dma_length = 0U;

#define STREAM5_ALL_CLEAR (DMA_HIFCR_CFEIF5 | DMA_HIFCR_CDMEIF5 | \
                           DMA_HIFCR_CTEIF5 | DMA_HIFCR_CHTIF5 | \
                           DMA_HIFCR_CTCIF5)
#define STREAM5_ERROR_MASK (DMA_HISR_TEIF5 | DMA_HISR_DMEIF5 | DMA_HISR_FEIF5)

void DMA_StopDac(void)
{
    NVIC_DisableIRQ(DMA1_Stream5_IRQn);
    DMA1_Stream5->CR &= ~DMA_SxCR_EN;
    while ((DMA1_Stream5->CR & DMA_SxCR_EN) != 0U) { }
    DMA1->HIFCR = STREAM5_ALL_CLEAR;
    NVIC_ClearPendingIRQ(DMA1_Stream5_IRQn);
    dma_callback = NULL;
    dma_length = 0U;
}

void DMA_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    (void)RCC->AHB1ENR;
    DMA_StopDac();
    DMA1_Stream5->CR = 0U;
    DMA1_Stream5->NDTR = 0U;
    DMA1_Stream5->PAR = (uint32_t)(uintptr_t)&DAC->DHR12R1;
    DMA1_Stream5->M0AR = 0U;
    DMA1_Stream5->FCR = 0U;
}

static bool DMA_StartCommon(const uint16_t *data, uint32_t length,
                            DMA_DAC_Callback callback)
{
    if (data == NULL || length == 0U || length > 65535U)
        return false;
    if (callback != NULL && (length < 4U || (length & 1U) != 0U))
        return false;

    DMA_StopDac();
    DMA1_Stream5->CR =
        (7UL << DMA_SxCR_CHSEL_Pos) |
        DMA_SxCR_DIR_0 | DMA_SxCR_MINC |
        DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0 |
        DMA_SxCR_CIRC;
    DMA1_Stream5->FCR = 0U;  /* Direct mode */
    DMA1_Stream5->PAR = (uint32_t)(uintptr_t)&DAC->DHR12R1;
    DMA1_Stream5->M0AR = (uint32_t)(uintptr_t)data;
    DMA1_Stream5->NDTR = length;
    DMA1->HIFCR = STREAM5_ALL_CLEAR;

    dma_callback = callback;
    dma_length = (uint16_t)length;
    if (callback != NULL)
    {
        /* Error and both buffer-half boundaries. */
        DMA1_Stream5->CR |= DMA_SxCR_HTIE | DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE;
        NVIC_ClearPendingIRQ(DMA1_Stream5_IRQn);
        NVIC_SetPriority(DMA1_Stream5_IRQn, 4U);
        NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    }
    DMA1_Stream5->CR |= DMA_SxCR_EN;
    return true;
}

bool DMA_StartDac(const uint16_t *data, uint32_t length)
{
    return DMA_StartCommon(data, length, NULL);
}

bool DMA_StartDacWithCallback(const uint16_t *data,
                              uint32_t length,
                              DMA_DAC_Callback callback)
{
    if (callback == NULL) return false;
    return DMA_StartCommon(data, length, callback);
}

void DMA1_Stream5_IRQHandler(void)
{
    const uint32_t status = DMA1->HISR;
    const bool ht = (status & DMA_HISR_HTIF5) != 0U;
    const bool tc = (status & DMA_HISR_TCIF5) != 0U;
    const bool error = (status & STREAM5_ERROR_MASK) != 0U;
    DMA1->HIFCR = STREAM5_ALL_CLEAR;

    DMA_DAC_Callback callback = dma_callback;
    if (callback == NULL) return;

    if (error || (ht && tc))
    {
        callback(DMA_DAC_ERROR);
        return;
    }

    if (ht || tc)
    {
        /* A very late IRQ indicates that DMA already reached the half
         * being refilled, so stop rather than corrupt live samples.
         */
        const uint32_t half = (uint32_t)dma_length / 2U;
        const uint32_t remaining_before = DMA1_Stream5->NDTR;
        if ((ht && remaining_before > half) ||
            (tc && remaining_before <= half))
        {
            callback(DMA_DAC_ERROR);
            return;
        }
        callback(ht ? DMA_DAC_HALF : DMA_DAC_FULL);
        if (dma_callback == NULL) return; /* Callback stopped stream. */
        const uint32_t remaining_after = DMA1_Stream5->NDTR;
        if ((ht && remaining_after > half) ||
            (tc && remaining_after <= half))
        {
            dma_callback(DMA_DAC_ERROR);
        }
    }
}
