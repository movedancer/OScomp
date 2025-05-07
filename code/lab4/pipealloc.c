int pipealloc(struct pipe_port *p0, struct pipe_port *p1)
{
    pipe_enty = (struct pipe*)kmalloc(sizeof(struct pipe));
    // LAB5 YOUR CODE EX1 -----------------
    // use kmalloc to alloc space for pipe_port
    ppt0 = (struct pipe_port*)kmalloc(sizeof(struct pipe_port));
    ppt1 = (struct pipe_port*)kmalloc(sizeof(struct pipe_port));
    // LAB5 YOUR CODE EX1 -----------------
    pipe_enty->addr= kmalloc(128);
    pipe_enty->readopen = 1;
    pipe_enty->writeopen = 1;
    pipe_enty->nwrite = 0;
    pipe_enty->nread = 0;

    ppt0->readable = 1;
    ppt0->writable = 0;
    ppt1->readable = 0;
    ppt1->writable = 1;
    ppt0->pipe=pipe_enty;
    ppt1->pipe=pipe_enty;

    return 0;
}