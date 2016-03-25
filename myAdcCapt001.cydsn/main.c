/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>

uint16 gmuwStrmIn[4096] __attribute__((section(".data")));
volatile uint8 gucFlagDmaFinished=0; //use volatile for safe when it optimized 

void MyInit(void);
void DMA_Config(void);
void MyPrintf(uint16 );

#define TOTAL_SAMPLE_NUMBER 2047
#define DMA_1__DRQ_NUMBER_4_MAIN (uint8)0u //same as cyfitter.h
#define DMA_EN_PRESERVE_TD 1 //1 is to retain TD config


CY_ISR(IrqDmaDone)
{
      gucFlagDmaFinished = 1;  
}

int main()
{
    uint16 uilp;

    CyGlobalIntEnable; /* Enable global interrupts. */
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    MyInit();
    CyDelay(250);

    Control_Reg_1_Write(3);
    ADC_SAR_1_Wakeup();
    CyDelay(250);
    
    gucFlagDmaFinished=0;
    isr_1_Enable();
    CyDelay(250);
    
    DMA_Config();//configure DMA as initialization.
    CyDmaChEnable(DMA_1__DRQ_NUMBER_4_MAIN, DMA_EN_PRESERVE_TD);//Start DMA
    while(!gucFlagDmaFinished)
    {
    }
    gucFlagDmaFinished=0;
    CyDmaTdFree(DMA_1__DRQ_NUMBER_4_MAIN);// to free TD resource for next config DMA
    isr_1_ClearPending();
    ADC_SAR_1_Sleep();
    Control_Reg_1_Write(0);
    for (uilp=0;uilp<TOTAL_SAMPLE_NUMBER;uilp++)
    {
        MyPrintf(gmuwStrmIn[uilp]);
        //printf("%04d\r\n",gmuwStrmIn[uilp]);
    }
    CyDelay(250);
        
   for(;;)
    {
        /* Place your application code here. */
    }
}

void MyInit()
{
    Clock_1_Start();
    Clock_2_Start();
    Clock_3_Start();

    isr_1_StartEx(IrqDmaDone);
    UART_1_Start();
    Control_Reg_1_Write(0);
    
    ADC_SAR_1_SetPower(1);
    ADC_SAR_1_Start();   
    ADC_SAR_1_Sleep();
    
}

#define DMA_1_BYTES_PER_BURST (2u) // 2 is for 16bit ADC
#define DMA_1_REQUEST_PER_BURST (1u) // 1 for each pulse
#define DMA_1_SRC_BASE (CYDEV_PERIPH_BASE)
#define DMA_1_DST_BASE (CYDEV_SRAM_BASE)
#define DMA_1_TRANSFER_COUNT 4095 //full byte
//#define DMA_EN_PRESERVE_TD 1 //1 is to retain TD config // move to above of this file

void DMA_Config()
{
    //ref to http://www.cypress.com/documentation/application-notes/an52705-psoc-3-and-psoc-5lp-getting-started-dma
    /* Variable declarations for DMA_1 */
    uint8 DMA_1_Chan;
    uint8 DMA_1_TD[1] = {0}; // TD_handle

    // step1
    /* Iniitialize DMA channel */
    DMA_1_Chan = DMA_1_DmaInitialize(DMA_1_BYTES_PER_BURST, DMA_1_REQUEST_PER_BURST,
                                     HI16(DMA_1_SRC_BASE), HI16(DMA_1_DST_BASE));

    //step 2
    /* Allocate TD */
    DMA_1_TD[0] = CyDmaTdAllocate();

    //step 3
    /* TD configuration setting */
    CyDmaTdSetConfiguration(DMA_1_TD[0], DMA_1_TRANSFER_COUNT ,
                             DMA_DISABLE_TD, (DMA_1__TD_TERMOUT_EN|TD_INC_DST_ADR) ); //DMA_INVALID_TD, DMA_1__TD_TERMOUT_EN

    /* Set Source and Destination address */
 //   CyDmaTdSetAddress(DMA_1_TD[0], (uint16)((uint32)ADC_DelSig_1_DEC_SAMP_PTR),
 //   (uint16)((uint32)VDAC8_1_Data_PTR));
    CyDmaTdSetAddress(DMA_1_TD[0], 
        (uint16)((uint32)ADC_SAR_1_SAR_WRK0_PTR),
        (uint16)((uint32)gmuwStrmIn));

    /* TD initialization */
    CyDmaChSetInitialTd(DMA_1_Chan, DMA_1_TD[0]);

    /* Enable the DMA channel */
   
    //CyDmaChEnable(DMA_1_Chan, DMA_EN_PRESERVE_TD);
}

void MyPrintf(uint16 uiSample)
{
    char buff[8];
    uint16 uilp;
    
    sprintf(&buff[0],"%04d\r\n",uiSample);
    
    for(uilp=0;uilp<6;uilp++)
    {
        UART_1_PutChar(buff[uilp]);
    }
}

/* [] END OF FILE */
