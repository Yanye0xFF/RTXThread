#include "rtt.h"
#include "rtconfig.h"
#include "rtthread.h"

SEGGER_RTT_CB _SEGGER_RTT;

static char _acUpBuffer[BUFFER_SIZE_STDOUT];
static char _acDownBuffer[BUFFER_SIZE_STDIN];

/**
 * @brief 向stdout缓冲区写数据，超出的部分将被截断
 * */
static void _WriteNoCheck(SEGGER_RTT_BUFFER_UP* pRing, const char* pData, unsigned NumBytes) {
  unsigned NumBytesAtOnce;
  unsigned WrOff;
  unsigned Rem;

  WrOff = pRing->WrOff;
  Rem = pRing->SizeOfBuffer - WrOff;
  if (Rem > NumBytes) {
    // All data fits before wrap around
    SEGGER_RTT_MEMCPY(pRing->pBuffer + WrOff, pData, NumBytes);
    pRing->WrOff = WrOff + NumBytes;
  } else {
    // We reach the end of the buffer, so need to wrap around
    NumBytesAtOnce = Rem;
    SEGGER_RTT_MEMCPY(pRing->pBuffer + WrOff, pData, NumBytesAtOnce);
    NumBytesAtOnce = NumBytes - Rem;
    SEGGER_RTT_MEMCPY(pRing->pBuffer, pData + Rem, NumBytesAtOnce);
    pRing->WrOff = NumBytesAtOnce;
  }
}

/**
 * @brief 获取stdout环形缓冲区剩余容量
 * */
static unsigned _GetAvailWriteSpace(SEGGER_RTT_BUFFER_UP* pRing) {
  unsigned RdOff;
  unsigned WrOff;
  unsigned r;
  // Avoid warnings regarding volatile access order.  It's not a problem
  // in this case, but dampen compiler enthusiasm.
  RdOff = pRing->RdOff;
  WrOff = pRing->WrOff;
  if (RdOff <= WrOff) {
    r = pRing->SizeOfBuffer - 1u - WrOff + RdOff;
  } else {
    r = RdOff - WrOff - 1u;
  }
  return r;
}

/**
 * @brief 写stdout的封装，做必要的检查后调用_WriteNoCheck
 * */
unsigned SEGGER_RTT_WriteNoLock(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes) {
  unsigned              Status;
  unsigned              Avail;
  const char*           pData;
  SEGGER_RTT_BUFFER_UP* pRing;

  pData = (const char *)pBuffer;
  // Get "to-host" ring buffer.

  pRing = &_SEGGER_RTT.aUp[BufferIndex];
  // How we output depends upon the mode...

  if(pRing->Flags == SEGGER_RTT_MODE_NO_BLOCK_SKIP) {

	// If we are in skip mode and there is no space for the whole
	// of this output, don't bother.
	Avail = _GetAvailWriteSpace(pRing);
	if (Avail < NumBytes) {
		Status = 0u;
	} else {
		Status = NumBytes;
		_WriteNoCheck(pRing, pData, NumBytes);
	}
  }else {
	    Status = 0u;
  }
  // Finish up.
  return Status;
}

/**
 * @brief 写stdout导出函数（原版还有其他的实现，未用到已删除）
 * */
unsigned rtt_write(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes) {
  unsigned Status;
  // Call the non-locking write function
  Status = SEGGER_RTT_WriteNoLock(BufferIndex, pBuffer, NumBytes);
  return Status;
}

/**
 * @breif 读stdin缓冲区
 * */
unsigned SEGGER_RTT_ReadNoLock(unsigned BufferIndex, void* pData, unsigned BufferSize) {
  unsigned                NumBytesRem;
  unsigned                NumBytesRead;
  unsigned                RdOff;
  unsigned                WrOff;
  unsigned char*          pBuffer;
  SEGGER_RTT_BUFFER_DOWN* pRing;

  pRing = &_SEGGER_RTT.aDown[BufferIndex];
  pBuffer = (unsigned char*)pData;
  RdOff = pRing->RdOff;
  WrOff = pRing->WrOff;
  NumBytesRead = 0u;
  // Read from current read position to wrap-around of buffer, first
  if (RdOff > WrOff) {
    NumBytesRem = pRing->SizeOfBuffer - RdOff;
    NumBytesRem = MIN(NumBytesRem, BufferSize);

    SEGGER_RTT_MEMCPY(pBuffer, pRing->pBuffer + RdOff, NumBytesRem);
    NumBytesRead += NumBytesRem;
    pBuffer      += NumBytesRem;
    BufferSize   -= NumBytesRem;
    RdOff        += NumBytesRem;

    // Handle wrap-around of buffer
    if (RdOff == pRing->SizeOfBuffer) {
      RdOff = 0u;
    }
  }
  // Read remaining items of buffer
  NumBytesRem = WrOff - RdOff;
  NumBytesRem = MIN(NumBytesRem, BufferSize);
  if (NumBytesRem > 0u) {
    SEGGER_RTT_MEMCPY(pBuffer, pRing->pBuffer + RdOff, NumBytesRem);
    NumBytesRead += NumBytesRem;
    pBuffer      += NumBytesRem;
    BufferSize   -= NumBytesRem;
    RdOff        += NumBytesRem;

  }
  if (NumBytesRead) {
    pRing->RdOff = RdOff;
  }
  return NumBytesRead;
}

unsigned rtt_read(unsigned BufferIndex, void* pBuffer, unsigned BufferSize) {
  unsigned NumBytesRead;
  // Call the non-locking read function
  NumBytesRead = SEGGER_RTT_ReadNoLock(BufferIndex, pBuffer, BufferSize);
  // Finish up.
  return NumBytesRead;
}

/**
 * @brief 初始化_SEGGER_RTT结构，使用前必须先调用rtt_init
 * */
int rtt_init(void) {
    SEGGER_RTT_CB *p;

    p = &_SEGGER_RTT;
    strcpy(&p->acID[0], "SEGGER RTT");
    p->MaxNumUpBuffers = SEGGER_RTT_MAX_NUM_BUFFERS;
    p->MaxNumDownBuffers = SEGGER_RTT_MAX_NUM_BUFFERS;

    p->aUp[0].sName = "Terminal";
    p->aUp[0].pBuffer = _acUpBuffer;
    p->aUp[0].SizeOfBuffer = BUFFER_SIZE_STDOUT;
    p->aUp[0].RdOff = 0u;
    p->aUp[0].WrOff = 0u;
    p->aUp[0].Flags = SEGGER_RTT_MODE_DEFAULT;

    p->aDown[0].sName = "Terminal";
    p->aDown[0].pBuffer = _acDownBuffer;
    p->aDown[0].SizeOfBuffer = BUFFER_SIZE_STDIN;
    p->aDown[0].RdOff = 0u;
    p->aDown[0].WrOff = 0u;
    p->aDown[0].Flags = SEGGER_RTT_MODE_DEFAULT;

    return 0;
}
// rtt_init放在board_init中,此时线程环境还未创建
INIT_BOARD_EXPORT(rtt_init);

/*************************** End of file ****************************/
