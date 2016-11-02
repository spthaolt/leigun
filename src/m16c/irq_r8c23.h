#include "m16c_cpu.h"
BusDevice * R8C23IntCo_New(const char *name);
void R8C23_AckIrq(BusDevice *intco_bdev,uint32_t intno);

#define R8C_INT_BRK			(0)
#define R8C_INT_CAN0_WAKE_UP		(3)
#define R8C_INT_CAN0_RX			(4)
#define R8C_INT_CAN0_TX			(5)
#define R8C_INT_CAN0_ERR		(6)
#define R8C_INT_TIMER_RD0		(8)
#define R8C_INT_TIMER_RD1		(9)
#define R8C_INT_TIMER_RE		(10)
#define R8C_INT_KEY			(13)
#define R8C_INT_AD			(14)
#define R8C_INT_SSI			(15)
#define R8C_INT_UART0_TX		(17)
#define R8C_INT_UART0_RX		(18)
#define R8C_INT_UART1_TX		(19)
#define R8C_INT_UART1_RX		(20)
#define R8C_INT_INT2			(21)
#define R8C_INT_TIMER_RA		(22)
#define R8C_INT_TIMER_RB		(24)
#define R8C_INT_INT1			(25)
#define R8C_INT_INT3			(26)
#define R8C_INT_INT0			(29)

