/*

  Copyright (c) 2015 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "../libdill.h"

static coroutine void worker(int ch) {
    int val;
    while(1) {
        int rc = chrecv(ch, &val, sizeof(val), -1);
        if(rc == -1 && errno == EPIPE)
            return;
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("usage: hdone <millions-of-roundtrips>\n");
        return 1;
    }
    long count = atol(argv[1]) * 1000000;

    int64_t start = now();

    int val = 0;
    long i;
    for(i = 0; i != count; ++i) {
        int ch = chmake(sizeof(int));
        int h = go(worker(ch));
        hdone(ch, -1);
        hclose(h);
        hclose(ch);
    }

    int64_t stop = now();
    long duration = (long)(stop - start);
    long ns = (duration * 1000000) / (count * 2);

    printf("done %ldM coroutine/channel cancellations in %f seconds\n",
        (long)(count / 1000000), ((float)duration) / 1000);
    printf("duration of coroutine/channel cancellation: %ld ns\n", ns);
    if (ns > 0) {
      printf("coroutine/channel cancellations per second: %fM\n",
          (float)(1000000000 / ns) / 1000000);
    }

    return 0;
}

