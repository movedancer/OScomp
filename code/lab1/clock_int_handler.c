int clock_int_handler(void * data)
{
#ifdef LAB1_EX2
  // LAB1 EXERCISE2: YOUR CODE
  // (1) count ticks here
  ticks++;
#ifdef _SHOW_100_TICKS
  // (2) if ticks % 100 == 0 then call kprintf to print "100 ticks"
  if(ticks % 100 == 0 ){
    kprintf("100 ticks\n");
  }
#endif
#endif
#ifdef LAB4_EX1
  run_timer_list();
#endif
  reload_timer();
  return 0;
}