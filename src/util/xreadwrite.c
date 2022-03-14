#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include "xreadwrite.h"

ssize_t xread(int fd, void *buf, size_t count)
{
    ssize_t r;
    do {
        r = read(fd, buf, count);
    } while (unlikely(r < 0 && errno == EINTR));
    return r;
}

ssize_t xwrite(int fd, const void *buf, size_t count)
{
    ssize_t r;
    do {
        r = write(fd, buf, count);
    } while (unlikely(r < 0 && errno == EINTR));
    return r;
}

ssize_t xread_all(int fd, void *buf, size_t count)
{
    char *b = buf;
    size_t pos = 0;
    do {
        ssize_t rc = read(fd, b + pos, count - pos);
        if (unlikely(rc == -1)) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (rc == 0) {
            // eof
            break;
        }
        pos += rc;
    } while (count - pos > 0);
    return pos;
}

ssize_t xwrite_all(int fd, const void *buf, size_t count)
{
    const char *b = buf;
    const size_t count_save = count;
    do {
        ssize_t rc = write(fd, b, count);
        if (unlikely(rc == -1)) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        b += rc;
        count -= rc;
    } while (count > 0);
    return count_save;
}

int xclose(int fd)
{
    int r = close(fd);
    if (likely(r == 0 || (errno != EINTR && errno != EINPROGRESS))) {
        return r;
    }

    // Treat EINPROGRESS the same as r == 0
    // (https://git.musl-libc.org/cgit/musl/commit/?id=82dc1e2e783815e00a90cd)
    if (errno == EINPROGRESS) {
        return 0;
    }

    // If the first close() call failed with EINTR, retry until
    // it succeeds or fails with a different error.
    do {
        r = close(fd);
    } while (r && errno == EINTR);

    // On some systems, when close() fails with EINTR, the descriptor
    // still gets closed anyway and calling close() again then fails
    // with EBADF. This is not really an "error" that can be handled
    // meaningfully, so we just return as if successful.
    //
    // Note that this is only safe to do in a single-threaded context,
    // where there's no risk of a concurrent open() call reusing the
    // same file descriptor.
    //
    // * http://www.daemonology.net/blog/2011-12-17-POSIX-close-is-broken.html
    // * https://ewontfix.com/4/
    // * https://sourceware.org/bugzilla/show_bug.cgi?id=14627
    // * https://www.austingroupbugs.net/view.php?id=529#c1200
    if (r && errno == EBADF) {
        r = 0;
    }

    return r;
}
