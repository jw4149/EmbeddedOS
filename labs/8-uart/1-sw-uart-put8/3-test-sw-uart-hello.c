// test that sw uart works.
#include "rpi.h"
#include "sw-uart.h"

void notmain(void) {
    // see that uart works at all.
    trace("about to print `hello world` 10x\n");
    trace("if your pi locks up, it means you are not transmitting\n");
    uart_flush_tx();
#if 0
    // turn off the hw UART so can use the same device.
    uart_disable();
#endif

    // use pin 14 for tx, 15 for rx
    sw_uart_t u = sw_uart_init(14,15, 115200);


    sw_uart_putk(&u, "TRACE: from sw-uart: WORKED!!\n");
    for(int i = 0; i < 10; i++)
        sw_uart_printk(&u, "TRACE: hello from sw-uart %d!\n",i);

    // reset to using the hardware uart.
    uart_init();
    trace("if you see `WORKED` above, sw uart worked!\n");
}
