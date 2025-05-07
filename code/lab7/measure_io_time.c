static int calculate_track_to_track_time(int current_block, int next_block) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int current_cyi = current_block / (SPT * HPC);
    int next_cyi = next_block / (SPT * HPC);
    int track_distance = (current_cyi - next_cyi);
    if(track_distance < 0){
        track_distance = -track_distance;
    }
    return track_distance * TRACK_SEEK_TIME;
#endif
}

static int calculate_sector_to_sector_time(int current_block, int next_block) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int current_sector = current_block % SPT;
    int next_sector = next_block % SPT;
    int sector_distance;
    if (current_sector>next_sector){
        sector_distance = next_sector + 50 - current_sector;
    }else{
        sector_distance = next_sector - current_sector;
    }
    return sector_distance * SECTOR_SEEK_TIME;
#endif
}

static int measure_time(int* traversed_blocks, int size, int begin_pos, int direction) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int total_time = 0;
    int current_block = begin_pos;
    for (int i = 0; i < size; i++) {
        int next_block = traversed_blocks[i];
        total_time += calculate_track_to_track_time(current_block, next_block);
        total_time += calculate_sector_to_sector_time(current_block, next_block);
        total_time += TIME_DATA_BLOCK_TRANSFER;
        current_block = next_block;
    }
    return total_time;
#endif
}