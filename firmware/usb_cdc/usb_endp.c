/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t packet_sent;
uint32_t Receive_length;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/

void EP1_IN_Callback (void)
{
  packet_sent = 1;
}

/*******************************************************************************
* Function Name  : EP3_OUT_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
#define PACKET_SIZE 64
#define CIRC_BUF_SIZE 34 /* 62 * 34 = ~2K of data (max. NAND page) */

typedef uint8_t packet_buf_t[PACKET_SIZE];

typedef struct
{
    packet_buf_t pbuf;
    uint32_t len;
} packet_t;

static packet_t circ_buf[CIRC_BUF_SIZE];
static uint8_t head, size, tail = CIRC_BUF_SIZE - 1;

uint32_t USB_Data_Peek(uint8_t **data)
{
  if (!size)
    return 0;
    
  *data = circ_buf[head].pbuf;

  return circ_buf[head].len;
}

uint32_t USB_Data_Get(uint8_t **data)
{
  uint32_t len;

  if (!size)
    return 0;

  *data = circ_buf[head].pbuf;
  len = circ_buf[head].len;
  head = (head + 1) % CIRC_BUF_SIZE;
  __disable_irq();
  size--;
  __enable_irq();

  return len;
}

static inline void USB_DataRx_Sched_Internal(void)
{
  if (size < CIRC_BUF_SIZE)
    SetEPRxValid(ENDP3);
}

void USB_DataRx_Sched(void)
{
  __disable_irq();
  USB_DataRx_Sched_Internal();
  __enable_irq();
}

void EP3_OUT_Callback(void)
{
  Receive_length = GetEPRxCount(ENDP3);
  if (size < CIRC_BUF_SIZE)
  {
    tail = (tail + 1) % CIRC_BUF_SIZE;
    PMAToUserBufferCopy(circ_buf[tail].pbuf, ENDP3_RXADDR, Receive_length);
    circ_buf[tail].len = Receive_length;
    size++;
    USB_DataRx_Sched_Internal();
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
