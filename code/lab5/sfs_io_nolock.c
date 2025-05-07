if((blkoff = offset % SFS_BLKSIZE)!= 0)//如果文件起始位置不对齐
    { 
        size = (nblks != 0) ? (SFS_BLKSIZE - blkoff) : (endpos - offset);//计算起始块需要读取的块长度
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//计算文件索引号
            goto out;
        if ((ret = sfs_buf_op(sfs, buf, size, ino, blkoff)) != 0)//读文件
            goto out;
        
        alen += size;//更新已读取块长度
        if (nblks == 0)
            goto out;
        
        buf += size, blkno ++, nblks --;//更新缓冲区已读数据规模,当前块,剩余读取块数量
    }
    size = SFS_BLKSIZE;//对齐部分单次读取块长度恒定
    while(nblks != 0)//循环处理对齐部分
    {
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//同上
            goto out;
        if((ret = sfs_block_op(sfs, buf, ino, 1)) != 0)
            goto out;
        
        alen += size, buf += size, blkno ++, nblks --;
    }
    if((size = endpos % SFS_BLKSIZE) != 0)//计算结束块读取的块长度,如果文件结束位置不对齐
    {
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//同上
            goto out;
        if((ret = sfs_buf_op(sfs, buf, size, ino, 0)) != 0)
            goto out;
        alen += size;//同上
    }