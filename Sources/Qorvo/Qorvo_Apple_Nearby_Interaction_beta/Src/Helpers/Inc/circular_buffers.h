#ifndef CIRCULAR_BUFFERS_H
#define CIRCULAR_BUFFERS_H
#include <stdint.h>

#ifdef TINY_BUILD
#define UART_RX_BUF_SIZE       0x100 /**< Read buffer for UART reception, shall be 1<<X */
#else
#define UART_RX_BUF_SIZE       0x800 /**< Read buffer for UART reception, shall be 1<<X */
#endif

#ifndef CIRC_CNT
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))
#endif/* Return count in buffer.  */

#ifndef CIRC_SPACE
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))
#endif/* Return space available, 0..size-1 */

#ifdef ETH_REPORT_BUFSIZE
struct eth_circ_buf_s {
    uint16_t    head;
    uint16_t    tail;
    uint8_t     buf[ETH_REPORT_BUFSIZE];
} __attribute__((__packed__));

typedef struct eth_circ_buf_s eth_circ_buf_t;
#endif

#ifdef UART_RX_BUF_SIZE
struct data_circ_buf_s
{
    uint16_t    head;
    uint16_t    tail;
    uint8_t     buf[UART_RX_BUF_SIZE];
};

typedef struct data_circ_buf_s data_circ_buf_t;
#endif

#endif //CIRCULAR_BUFFERS_H
