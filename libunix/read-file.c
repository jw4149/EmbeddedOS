#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    // How: 
    //    - use stat() to get the size of the file.
    //    - round up to a multiple of 4.
    //    - allocate a buffer
    //    - zero pads to a multiple of 4.
    //    - read entire file into buffer (read_exact())
    //    - fclose() the file descriptor
    //    - make sure any padding bytes have zeros.
    //    - return it.   
    struct stat sb;

    int fd = open(name, O_RDONLY);

    int ret = stat(name, &sb);
    if (ret != 0) {
        close(fd);
        return NULL;
    }
    
    *size = sb.st_size;

    unsigned buf_size = sb.st_size/4;

    if (sb.st_size % 4 != 0)
        buf_size += 1;

    int *buf = malloc(buf_size * sizeof(int));
    memset(buf, 0, buf_size * sizeof(int));

    if (*size == 0) {
        close(fd);
        return buf;
    }

    read_exact(fd, buf, *size);
    close(fd);

    return buf;
}
