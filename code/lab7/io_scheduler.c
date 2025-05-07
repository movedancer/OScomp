int* look_schedule(int *begin_pos, int *begin_direction, int *size) {
    struct bio* current;
    int* traversed_blocks = (int*)kmalloc(request_count * sizeof(int));
    int count = 0;

    *begin_pos = disk_head_look->block_num;
    *begin_direction = direction_look;
    kprintf("begin_pos = %d, begin_direction = %d\n", *begin_pos, *begin_direction);
#ifdef LAB10_EX1
    // LAB10 EXERCISE1: YOUR CODE
    int flag = disk_head_look->block_num;
    while (1) {
        if (*begin_direction == 1 && request_count){
            current = disk_head_look->next;
            while(current != list_tail_look && current->block_num != -1){
                if (current->block_num == flag){
                    current = current->next;
                    continue;
                }
                traversed_blocks[count] = current->block_num;
                count++;
                kprintf("Serving block: %d\n", current->block_num);
                *begin_pos = current->block_num;
                disk_head_look->next = current->next;
                if(current->next != NULL){
                    current->next->prev = disk_head_look;
                }
                kfree(current);
                request_count--;
                current = disk_head_look->next;
            }
            *begin_direction = -1;
        }
        if (*begin_direction == -1 && request_count){
            current = disk_head_look->prev;
            while(current != list_head_look && current->block_num != -1){
                if (current->block_num == flag){
                    current = current->prev;
                    continue;
                }
                traversed_blocks[count] = current->block_num;
                count++;
                kprintf("Serving block: %d\n", current->block_num);
                *begin_pos = current->block_num;
                disk_head_look->prev = current->prev;
                if(current->prev != NULL){
                    current->prev->next = disk_head_look;
                }
                kfree(current);
                request_count--;
                current = disk_head_look->prev;
            }
            *begin_direction = 1;
        }
        break;
    }
    *begin_pos = flag;
    *size = count;
#endif
    request_count = 0;
    return traversed_blocks;
}