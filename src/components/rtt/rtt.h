#ifndef _SEGGER_RTT_H_
#define _SEGGER_RTT_H_

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <rtthread.h>
#include <sys/param.h>

#define FORMAT_FLAG_LEFT_JUSTIFY   (1u << 0)
#define FORMAT_FLAG_PAD_ZERO       (1u << 1)
#define FORMAT_FLAG_PRINT_SIGN     (1u << 2)
#define FORMAT_FLAG_ALTERNATE      (1u << 3)

#define SEGGER_RTT_MODE_NO_BLOCK_SKIP         (0U)     // Skip. Do not block, output nothing. (Default)
#define SEGGER_RTT_MODE_MASK                  (3U)

#define SEGGER_RTT_MAX_NUM_BUFFERS            (1)

#define BUFFER_SIZE_STDOUT                    (4096u)
#define BUFFER_SIZE_STDIN                     (64u)

// 非阻塞写缓存，但打印Log速度太快超过RTT Viewer读取可能会丢字符
#define SEGGER_RTT_MODE_DEFAULT               SEGGER_RTT_MODE_NO_BLOCK_SKIP

#define SEGGER_RTT_MEMCPY(pDest, pSrc, NumBytes)      rt_memcpy((pDest), (pSrc), (NumBytes))

typedef struct {
    const char *sName; // Optional name. Standard names so far are: "Terminal", "SysView", "J-Scope_t4i4"
    char *pBuffer;       // Pointer to start of buffer
    unsigned SizeOfBuffer; // Buffer size in bytes. Note that one byte is lost, as this implementation does not fill up the buffer in order to avoid the problem of being unable to distinguish between full and empty.
    unsigned WrOff;     // Position of next item to be written by either target.
    volatile unsigned RdOff; // Position of next item to be read by host. Must be volatile since it may be modified by host.
    unsigned Flags;         // Contains configuration flags
} SEGGER_RTT_BUFFER_UP;

typedef struct {
    const char *sName; // Optional name. Standard names so far are: "Terminal", "SysView", "J-Scope_t4i4"
    char *pBuffer;       // Pointer to start of buffer
    unsigned SizeOfBuffer; // Buffer size in bytes. Note that one byte is lost, as this implementation does not fill up the buffer in order to avoid the problem of being unable to distinguish between full and empty.
    volatile unsigned WrOff;     // Position of next item to be written by either target.
    unsigned RdOff; // Position of next item to be read by host. Must be volatile since it may be modified by host.
    unsigned Flags;         // Contains configuration flags
} SEGGER_RTT_BUFFER_DOWN;

// SEGGER_RTT_CB需要保持原样，rtt viewer需要扫描该结构体以识别segger rtt
typedef struct {
    char acID[16];                                // Initialized to "SEGGER RTT"
    int MaxNumUpBuffers; // Initialized to SEGGER_RTT_MAX_NUM_UP_BUFFERS (type. 2)
    int MaxNumDownBuffers; // Initialized to SEGGER_RTT_MAX_NUM_DOWN_BUFFERS (type. 2)
    SEGGER_RTT_BUFFER_UP aUp[SEGGER_RTT_MAX_NUM_BUFFERS]; // Up buffers, transferring information up from target via debug probe to host
    SEGGER_RTT_BUFFER_DOWN aDown[SEGGER_RTT_MAX_NUM_BUFFERS]; // Down buffers, transferring information down from host via debug probe to target
} SEGGER_RTT_CB;

#ifdef __cplusplus
extern "C" {
#endif

int rtt_init(void) ;

unsigned rtt_write(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes);

unsigned rtt_read(unsigned BufferIndex, void* pBuffer, unsigned BufferSize);

#ifdef __cplusplus
}
#endif

#endif
