OUFILE fopen(){
  oufs_find_file(cwd, path, &parent, &child, localname);
  switch (mode){
  case 'r':
    if (child == UNALLOCATED_INODE){
      //error
    }
    f->inode_reference = child;
    f->mode = 'r';
    f->offset = 0;
    break;
  case 'w':
    //free all data blocks if allocated
    //then
    child_inode.size = 0;
    //fall through!
  case 'a':
    if (parent == UNALLOCATED_INODE){
      //error
    }
    if (child == UNALLOCATED_INODE){
      //new file
      child = new_inode(IT_FILE, UNALLOCATED_BLOCK);
      //insert child into parent and 
    }
    f->inode_reference = child;
    f->mode = 'a';//or w;
    f->offset = child_inode.size;
    break;

    return f;
  }
