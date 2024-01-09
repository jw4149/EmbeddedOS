// change from 4-fb.c: flip around 
// structure assignments so the code
// breaks.
typedef struct {
  unsigned read;
  unsigned padding[3];
  unsigned peek;
  unsigned sender;
  unsigned status;
  unsigned configuration;
  unsigned write;
} mailbox_t;

typedef struct {
        unsigned width;
        unsigned height;
        unsigned virtual_width;
        unsigned virtual_height;
        unsigned pitch;
        unsigned depth;
        unsigned x_offset;
        unsigned y_offset;
        unsigned pointer;
        unsigned size;
} __attribute__ ((aligned(16))) fb_config_t;

#define MAILBOX_EMPTY  (1<<30)
#define MAILBOX_FULL   (1<<31)

static fb_config_t cp;

void write_mailbox_x(mailbox_t *mbox, unsigned channel) {
    while(mbox->status & MAILBOX_FULL)
        ;
    // interesting, if you flip around these assignments
    // controls if the write assigment gets hoisted
    cp.height = cp.virtual_height = 960;
    cp.x_offset = cp.y_offset = 0;
    cp.depth = 32;
    cp.width = cp.virtual_width = 1280;
    cp.pointer = 0;
    asm volatile ("" : : : "memory");
    mbox->write = ((unsigned)(&cp) | channel | 0x40000000);
}
















