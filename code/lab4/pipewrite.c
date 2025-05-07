// LAB5 CODES for communication
int pipewrite(struct pipe *pi, char* msg, int len)
{
    // w 记录已经写的字节数
    int w = 0;
    while(w < len){
        // 若不可读，写也没有意义
        if(pi->readopen == 0){
            return -1;
        }
        if(pi->nwrite == pi->nread + PIPESIZE){
            // pipe write 端已满，阻塞
            do_yield();
        } else {
            // 一次读的 size 为 min(用户buffer剩余，pipe 剩余写容量，pipe 剩余线性容量)
            int size = MIN3(
                len - w,
                pi->nread + PIPESIZE - pi->nwrite,
                PIPESIZE - (pi->nwrite % PIPESIZE)
            );
            kprintf("size %d\n",size);
            //LAB5 YOUR CODE EX2 -----------------
            for (int x = 0; x < size; x++) {
                pi->addr[pi->nwrite % PIPESIZE + x] = msg[w + x];
            }
            pi->nwrite += size;
            w += size;
            //LAB5 YOUR CODE EX2 -----------------
        }
    }
    return w;
}