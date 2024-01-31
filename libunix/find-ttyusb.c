// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "ttyACM",   // linux
    "cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    "cu.usbserial-120",
    // if your system uses another name, add it.
	0
};

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    int i = 0;
    while (ttyusb_prefixes[i] != 0) {
        if (strcmp(ttyusb_prefixes[i], d->d_name) == 0) {
            return 1;
        }
        i++;
    }
    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    const char *dev_prefix = "/dev/";
    struct dirent **fileListTemp;
    int numFiles = scandir(dev_prefix, &fileListTemp, filter, alphasort);
    if (numFiles != 1) {
        panic("panic!\n");
        return 0;
    }
    char *name = (char *)malloc(strlen(fileListTemp[0]->d_name) + strlen(dev_prefix) + 1);
    strncpy(name, dev_prefix, strlen(dev_prefix));
    strcpy(name+strlen(dev_prefix), fileListTemp[0]->d_name);
    return name;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    return find_ttyusb();
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    return find_ttyusb();
}
