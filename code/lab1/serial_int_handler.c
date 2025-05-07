void serial_int_handler(void *opaque)
{
    unsigned char id = inb(COM1+COM_IIR);
    if(id & 0x01)
        return ;
    //int c = serial_proc_data();
    int c = cons_getc();
#if defined(LAB1_EX3) && defined(_SHOW_SERIAL_INPUT)
    // LAB1 EXERCISE3: YOUR CODE
    kprintf("got input %c\n", c);
#endif
#ifdef LAB4_EX2
    extern void dev_stdin_write(char c);
    dev_stdin_write(c);
#endif
}