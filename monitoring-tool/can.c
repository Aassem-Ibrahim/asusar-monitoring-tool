/****************************************************************************** 
 * Module      : CAN Bus                                                      * 
 * File Name   : can.c                                                        * 
 * Description : Source file for can module                                   * 
 * Created on  : Mar 23, 2020                                                 * 
 ******************************************************************************/

#include "can.h"

uint8_t g_rxCAN[8];
uint8_t g_txCAN[8];

tCANMsgObject g_rxCANMessage, g_txCANMessage;

volatile bool g_CANRXFlag = false;
volatile bool g_CANTXFlag = true;
volatile bool g_CANErrFlag = false;


void CANIntHandler(void)
{
uint32_t ui32Status;

    /* Get CAN interrupt status. */
    ui32Status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

    /* Check if Error */
    if(ui32Status == CAN_INT_INTID_STATUS)
    {
        /* Get CAN status. */
        ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);

        /* Set CAN Error Flag. */
        g_CANErrFlag = true;
    }
    else if (ui32Status == 1)
    {
        /* Clear interrupt for object #1. */
        CANIntClear(CAN0_BASE, 1);

        /* Set CAN receive flag. */
        g_CANRXFlag = true;

        /* Clear CAN Error Flag. */
        g_CANErrFlag = false;
    }
    else if (ui32Status == 2)
    {
        /* Clear interrupt for object #2. */
        CANIntClear(CAN0_BASE, 2);

        /* Set CAN transmission flag. */
        g_CANTXFlag = true;

        /* Clear CAN Error Flag. */
        g_CANErrFlag = false;
    }
    else
    {
        /* MISRA */
    }
}

void CAN_Init(void)
{
    /* Set ECU clock. */
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    /* Enable PORTB clock. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    /* Configure CAN pins. */
    GPIOPinConfigure(GPIO_PB4_CAN0RX);
    GPIOPinConfigure(GPIO_PB5_CAN0TX);

    /* Select CAN mode for its pins. */
    GPIOPinTypeCAN(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    /* Enable CAN0 peripheral. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    /* Initialize CAN0 to known state. */
    CANInit(CAN0_BASE);

    /* Select CAN bit-rate. */
    CANBitRateSet(CAN0_BASE, SysCtlClockGet(), 500000);

    /* Enable CAN interrupt. */
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);
    IntEnable(INT_CAN0);

    /* Enable CAN0. */
    CANEnable(CAN0_BASE);
}

void CAN_ReceiveConfig(void)
{
    /* Select CAN Message ID. */
    g_rxCANMessage.ui32MsgID = 0;

    /* Select CAN Message Mask. */
    g_rxCANMessage.ui32MsgIDMask = 0;

    /* Set CAN Message flags. */
    g_rxCANMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;

    /* Select CAN Message length. */
    g_rxCANMessage.ui32MsgLen = 8;

    /* Select CAN Message data receiving container size. */
    g_rxCANMessage.pui8MsgData = g_rxCAN;

    /* Set as receiving container. */
    CANMessageSet(CAN0_BASE, 1, &g_rxCANMessage, MSG_OBJ_TYPE_RX);
}

void CAN_TransmitConfig(void)
{
    /* Select CAN Message ID. */
    g_txCANMessage.ui32MsgID = 1;

    /* Select CAN Message Mask. */
    g_txCANMessage.ui32MsgIDMask = 0;

    /* Set CAN Message flags. */
    g_txCANMessage.ui32Flags = MSG_OBJ_TX_INT_ENABLE;

    /* Select CAN Message length. */
    g_txCANMessage.ui32MsgLen = 8;

    /* Select CAN Message transmission data container. */
    g_txCANMessage.pui8MsgData = g_txCAN;
}

uint32_t CAN_GetRXMessageLength()
{
    return g_rxCANMessage.ui32MsgLen;
}

void CAN_SetTXMessageLength(uint8_t message_length)
{
    /* Select CAN Message length. */
    g_txCANMessage.ui32MsgLen = message_length;
}

void CAN_Transmit(void)
{
    /* Check if transmission is allowed. */
    if ((true == g_CANTXFlag) && (false == g_CANErrFlag))
    {
        /* Clear Flag. */
        g_CANTXFlag = false;

        /* Send over CAN. */
        CANMessageSet(CAN0_BASE, 2, &g_txCANMessage, MSG_OBJ_TYPE_TX);
    }
}

void CAN_Receive(void)
{
    /* Clear Flag. */
    g_CANRXFlag = false;

    /* Update Message. */
    CANMessageGet(CAN0_BASE, 1, &g_rxCANMessage, 0);
}
