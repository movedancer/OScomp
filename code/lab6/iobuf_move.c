int iobuf_move(struct iobuf *iob, void *data, size_t len, bool m2b, size_t *copiedp) {
    size_t alen;
    if ((alen = iob->io_resid) > len) {
        alen = len;
    }
    if (alen == 0) {
        if (copiedp != NULL) {
            *copiedp = 0;
        }
        return 0;
    }
    if (m2b) {
        memmove(iob->io_base, data, alen);
        iobuf_skip(iob, alen);
        len -= alen;
    } else {
        memmove(data, iob->io_base, alen);
        iobuf_skip(iob, alen);
        len -= alen;
    }
    if (copiedp != NULL) {
        *copiedp = alen;
    }
    return (len == 0) ? 0 : -E_NO_MEM;
}