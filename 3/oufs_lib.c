#include <stdlib.h>
#include "oufs_lib.h"
#include "widio.h"
#define debug 1
#define dprintf(...) if(debug){fprintf (stderr, __VA_ARGS__);}

BLOCK get(BLOCK_REFERENCE br){
  //I wrote this function fairly late in the game.
  //we OO now boys
  BLOCK b;
  vdisk_read_block(br, &b);
  return b;
}
int set(BLOCK_REFERENCE br, BLOCK b){
  //a complimentary fn to get, but not quite an oo setter.
  return vdisk_write_block(br, &b);
}
INODE get_inode(INODE_REFERENCE ir){
  INODE i;
  oufs_read_inode_by_reference(ir, &i);
  return i;
}

int string_compare(const void* l, const void* r){
  //a wrapper function to use strcmp with qsort
  //basically each void * is going to be point at our char *,
  //so we jump through the appropriate hoops
  return strcmp(*(char**)l,*(char**)r);
}

INODE new_inode(char inode_type, BLOCK_REFERENCE br){
  //NOTE THAT ATM THIS ONLY CONSTRUCTS DIRECTORY INODES
  //AND SIMPLY ASSIGNS THEM TYPE IT_NONE if necessary.
  INODE new;
  new.type = inode_type;
  new.n_references = 1;
  new.data[0] = br;
  for(int i = 1; i < BLOCKS_PER_INODE; i++){
    new.data[i] = UNALLOCATED_BLOCK;
  }
  new.size = 2;
  return new;
}

INODE_REFERENCE rm_from_block(const char * name, BLOCK* db){
  //Why does this return an ir? because I didn't want to write the logic twice.
  int i = 0;
  while(db->directory.entry[i].inode_reference != UNALLOCATED_INODE){
    i++;
    if(streq(db->directory.entry[i].name, name)){
      db->directory.entry[i].name[0] = '\0';
      INODE_REFERENCE ir = db->directory.entry[i].inode_reference;
      db->directory.entry[i].inode_reference = UNALLOCATED_INODE;
      return ir;
    }
  }
  return UNALLOCATED_INODE;
}

int is_full(BLOCK_REFERENCE dbr){ //true if full false otherwise
  int i = 0;
  BLOCK b;
  vdisk_read_block(dbr, &b);
  while(b.directory.entry[i].inode_reference != UNALLOCATED_INODE){
    i++;
  }
  if(i>=DIRECTORY_ENTRIES_PER_BLOCK){
    return 1;
  } else {
    return 0;
  }
}

int is_empty(BLOCK_REFERENCE dbr){ //true if empty false otherwise
  int i = 0;
  BLOCK b;
  vdisk_read_block(dbr, &b);
  while(b.directory.entry[i].inode_reference != UNALLOCATED_INODE){
    i++;
  }
  dprintf("##is_empty records this many entries: %d\n", i);
  if(i==2){
    return 1;
  } else {
    return 0;
  }
}

int add_inode_to_block(BLOCK *b, INODE_REFERENCE ir, const char * name){
  int i = 0;
  while(b->directory.entry[i].inode_reference != UNALLOCATED_INODE){
    i++;
  }
  if(i>=DIRECTORY_ENTRIES_PER_BLOCK){
    fprintf(stderr,"no more open inode entries");
    return -1;
  } else {
    b->directory.entry[i].inode_reference = ir;
    strcpy(b->directory.entry[i].name, name);
    return 0;
  }
}

/**
 * Read the ZPWD and ZDISK environment variables & copy their values into cwd and disk_name.
 * If these environment variables are not set, then reasonable defaults are given.
 *
 * @param cwd String buffer in which to place the OUFS current working directory.
 * @param disk_name String buffer containing the file name of the virtual disk.
 */
void oufs_get_environment(char *cwd, char *disk_name)
{
  // Current working directory for the OUFS
  char *str = getenv("ZPWD");
  if(str == NULL) {
    // Provide default
    strcpy(cwd, "/");
  }else{
    // Exists
    STRNULLCOPY(cwd, str, MAX_PATH_LENGTH-1);
  }

  // Virtual disk location
  str = getenv("ZDISK");
  if(str == NULL) {
    // Default
    strcpy(disk_name, "vdisk1");
  }else{
    // Exists: copy
    STRNULLCOPY(disk_name, str, MAX_PATH_LENGTH-1);
  }

}

/**
 * Configure a directory entry so that it has no name and no inode
 *
 * @param entry The directory entry to be cleaned
 */
void oufs_clean_directory_entry(DIRECTORY_ENTRY *entry) 
{
  entry->name[0] = 0;  // No name
  entry->inode_reference = UNALLOCATED_INODE;
}

/**
 * Initialize a directory block as an empty directory
 *
 * @param self Inode reference index for this directory
 * @param self Inode reference index for the parent directory
 * @param block The block containing the directory contents
 *
 */

void oufs_clean_directory_block(INODE_REFERENCE self, INODE_REFERENCE parent, BLOCK *block)
{
  // Debugging output
  if(debug)
    fprintf(stderr, "New clean directory: self=%d, parent=%d\n", self, parent);

  // Create an empty directory entry
  DIRECTORY_ENTRY entry;
  oufs_clean_directory_entry(&entry);

  // Copy empty directory entries across the entire directory list
  for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
    block->directory.entry[i] = entry;
  }

  // Now we will set up the two fixed directory entries

  // Self
  STRNULLCOPY(entry.name, ".", 2);
  entry.inode_reference = self;
  block->directory.entry[0] = entry;

  // Parent (same as self
  STRNULLCOPY(entry.name, "..", 3);
  entry.inode_reference = parent;
  block->directory.entry[1] = entry;
  
}

//WROTE THIS FUNCTION BASED ON MY INFORMED GUESSES
//IT'S PROBABLY RIGHT THO
int oufs_find_open_bit(unsigned char value){
  int i = 0;
  int acc = value;
  while(acc % 2 == 1){
    acc = acc / 2;
    i++;
  }
  return i; // if it's full you get an 8 I guess.
}

/**
 * Allocate a new data block
 *
 * If one is found, then the corresponding bit in the block allocation table is set
 *
 * @return The index of the allocated data block.  If no blocks are available,
 * then UNALLOCATED_BLOCK is returned
 *
 */
BLOCK_REFERENCE oufs_allocate_new_block()
{
  BLOCK block;
  // Read the master block
  vdisk_read_block(MASTER_BLOCK_REFERENCE, &block);

  // Scan for an available block
  int block_byte;
  int flag;

  // Loop over each byte in the allocation table.
  for(block_byte = 0, flag = 1; flag && block_byte < N_BLOCKS_IN_DISK / 8; ++block_byte) {
    if(block.master.block_allocated_flag[block_byte] != 0xff) {
      // Found a byte that has an opening: stop scanning
      flag = 0;
      break;
    };
  };
  // Did we find a candidate byte in the table?
  if(flag == 1) {
    // No
    if(debug)
      fprintf(stderr, "No blocks\n");
    return(UNALLOCATED_BLOCK);
  }

  // Found an available data block 

  // Set the block allocated bit
  // Find the FIRST bit in the byte that is 0 (we scan in bit order: 0 ... 7)
  int block_bit = oufs_find_open_bit(block.master.block_allocated_flag[block_byte]);

  // Now set the bit in the allocation table
  block.master.block_allocated_flag[block_byte] |= (1 << block_bit);

  // Write out the updated master block
  vdisk_write_block(MASTER_BLOCK_REFERENCE, &block);

  if(debug)
    fprintf(stderr, "##Allocating block byte %d bit %d\n", block_byte, block_bit);

  // Compute the block index
  BLOCK_REFERENCE block_reference = (block_byte << 3) + block_bit;

  if(debug)
    fprintf(stderr, "Allocated block at block ref %d\n", block_reference);
  
  // Done
  return(block_reference);
}

BLOCK_REFERENCE ir2br(INODE_REFERENCE i){
  //returns block reference of inode block which holds the referenced inode
  return i / INODES_PER_BLOCK + 1;
}

int ir2ei(INODE_REFERENCE i){
  //returns element reference given inode reference
  return i % INODES_PER_BLOCK;
}


/**
 *  Given an inode reference, read the inode from the virtual disk.
 *
 *  @param i Inode reference (index into the inode list)
 *  @param inode Pointer to an inode memory structure.  This structure will be
 *                filled in before return)
 *  @return 0 = successfully loaded the inode
 *         -1 = an error has occurred
 *
 */
int oufs_read_inode_by_reference(INODE_REFERENCE i, INODE *inode)
{
  if(debug){
    fprintf(stderr, "Fetching inode %d\n", i);
  }
  // Find the address of the inode block and the inode within the block
  BLOCK_REFERENCE block = i / INODES_PER_BLOCK + 1;
  int element = (i % INODES_PER_BLOCK);

  BLOCK b;
  if(vdisk_read_block(block, &b) == 0) {
    // Successfully loaded the block: copy just this inode
    *inode = b.inodes.inode[element];
    return(0);
  }
  // Error case
  return(-1);
}

int oufs_write_inode_by_reference(INODE_REFERENCE i, INODE *inode){
  BLOCK b;
  int element = (i % INODES_PER_BLOCK);
  if(vdisk_read_block(ir2br(i), &b) == 0) {
    // Successfully loaded the block: copy just this inode
    b.inodes.inode[element] = *inode;
    if(vdisk_write_block(ir2br(i), &b) != 0){
      return EXIT_SUCCESS;
    }
  }
  // Error case
  return EXIT_FAILURE;
}

INODE_REFERENCE oufs_allocate_new_inode(char inode_type, BLOCK_REFERENCE br){
  BLOCK block;
  INODE_REFERENCE ir;
  vdisk_read_block(MASTER_BLOCK_REFERENCE, &block);
  // Read the inode blocks
  for(int i = 0; i < N_INODE_BLOCKS; i++){
    if(block.master.inode_allocated_flag[i]!=0xff){
      int bit = oufs_find_open_bit(block.master.inode_allocated_flag[i]);
      block.master.inode_allocated_flag[i] |= 1<<bit;
      vdisk_write_block(MASTER_BLOCK_REFERENCE, &block);
      //place actual inode
      ir = (i<<3)+bit;
      INODE inode = new_inode(inode_type,br);
      oufs_write_inode_by_reference(ir, &inode);
      return ir;
    }
  }
  if(debug){
    fprintf(stderr, "#No open inodes\n");
  }
  return(UNALLOCATED_INODE);
}

int oufs_format_disk(char * virtual_disk_name){
  if (!vdisk_disk_open(virtual_disk_name)) {
    
    //zero all blocks
    BLOCK z;
    NULLOUT(z);
    for(BLOCK_REFERENCE i = 0; i < N_BLOCKS_IN_DISK; i++){
      vdisk_write_block(i,&z);
    }
    
    //actually do the thing

    //1. write the master block
    BLOCK m = z;
    //I happen to know these magic constants are true:
    m.master.block_allocated_flag[0] = 0xFF; // master block and 7 inode blocks
    m.master.block_allocated_flag[1] = 0x03; // 8th inode block and first data block
    m.master.inode_allocated_flag[0] = 0x01; // first inode (root)
    vdisk_write_block(MASTER_BLOCK_REFERENCE,&m);

    //2a. write all inode blocks to unalloc
    BLOCK block;
    for(int i = 1; i < N_INODE_BLOCKS + 1; i++){
      vdisk_read_block(i, &block);
      for(int j = 0; j < INODES_PER_BLOCK; j++){
	//replace with unallocated inode
	block.inodes.inode[j] = new_inode(IT_NONE, UNALLOCATED_BLOCK);
      }
      vdisk_write_block(i, &block);
    }
    //2. write the first inode block, containing the first inode
    INODE root = new_inode(IT_DIRECTORY, ROOT_DIRECTORY_BLOCK);
    
    BLOCK ib;
    vdisk_read_block(1, &ib);
    ib.inodes.inode[0] = root;
    vdisk_write_block(1, &ib); //1 is the first inode block

    //3. write the dir block representing /
    BLOCK db;
    oufs_clean_directory_block(0, 0, &db); //these inode refs point back to the root inode
    vdisk_write_block(ROOT_DIRECTORY_BLOCK,&db);
    
    vdisk_disk_close(virtual_disk_name);
    return EXIT_SUCCESS;
  } else {
    perror("oufs_format_disk couldn't open vdisk with provided name");
    return EXIT_FAILURE;
  }
}

BLOCK_REFERENCE dirpdir(BLOCK_REFERENCE br, const char * name){
  //Progress from DIRectory block to DIRectory block
  BLOCK b;
  if(vdisk_read_block(br, &b) == 0) {
    // Successfully loaded the block:
    for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; i++){
      dprintf("##looping. inspecting %s\n", b.directory.entry[i].name);
      if(streq(b.directory.entry[i].name, name)){
	dprintf("##found match\n");
	INODE inode;
	oufs_read_inode_by_reference(b.directory.entry[i].inode_reference, &inode);
	return inode.data[0]; //this will be a directory block
      }
    }
  } else {
    dprintf("dirpdir couldn't read referenced block\n");
  }
  //ERROR CASE
  return UNALLOCATED_BLOCK;
}

int oufs_find_file(const char *cwd, const char *path, BLOCK_REFERENCE *parent, BLOCK_REFERENCE *child, char *local_name, INODE_REFERENCE *inode_of_parent){
   //remember, cwd will be / by default, and path will be an empty string by default.
  INODE_REFERENCE iop = 0; //root inode
  BLOCK_REFERENCE br = ROOT_DIRECTORY_BLOCK;
  char fullpath[MAX_PATH_LENGTH*2]; //double-wide path action! consider yourself lucky.
  char * name;
  char * lastname = ""; //this in "last name" in the sense of "name at end" 
  if(path && path[0] == '/'){ //path is absolute
    strcpy(fullpath,path);
  } else { //path is relative to cwd
    strcpy(fullpath,cwd);
    strcat(fullpath,"/"); // extra slash is fine, no slash would be a bug
    strcat(fullpath,path);
  }
  char * p = fullpath;
  BLOCK_REFERENCE pbr = br;
  while( (name = strtok(p, "/")) ){
    if(debug){fprintf(stderr, "##processing this part of path: %s\n", name);}
    iop = get(pbr).directory.entry[0].inode_reference;
    pbr=br;
    lastname=name;
    br = dirpdir(br, name);
    if(pbr == UNALLOCATED_BLOCK){ //a parent doesn't exist, error
      fprintf(stderr,"a parent directory in the path did not exist\n");
      exit(EXIT_FAILURE);
    }
    p = NULL; //this allows strtok to process the same array next time
  }
  if(debug){fprintf(stderr, "##assigning values in find_files\n");}
  if(debug){fprintf(stderr, "##assigning parent in find_files\n");}
  *parent = pbr;
  if(debug){fprintf(stderr, "##assigning child in find_files\n");}
  *child = br;
  if(debug){fprintf(stderr, "##assigning string in find_files\n");}
  strcpy(local_name, lastname);
  if(debug){fprintf(stderr, "##assigning iop in find_files\n");}
  *inode_of_parent = iop;
  return EXIT_SUCCESS;
}

int print_dir(BLOCK_REFERENCE dir){
  if(debug){fprintf(stderr, "##printing dir: %d\n", dir);}

  char *names[DIRECTORY_ENTRIES_PER_BLOCK]; //array of char pointers
  NULLOUT(names);
  
  int i = 0;
  int j = 0;
  BLOCK b;
  if(vdisk_read_block(dir, &b) == 0) {
    // Successfully loaded the block:
    for(; i < DIRECTORY_ENTRIES_PER_BLOCK; i++){
      if(!b.directory.entry[i].name[0]){
	continue;
      } else {
	names[j] = b.directory.entry[i].name;
	j++;
      }
    }
    if(debug){fprintf(stderr, "##time to sort. array size %d\n", i);}
    qsort(&names, j, sizeof(names[0]), &string_compare);
    //note that strcmp takes char *s not void *s so we had to cast it...
    //I have the hunch that __compar_fn_t is specific to gcc, so that type is probably not portable.
    //so we use the ugly (int (*)(const void *, const void *))
    //actually I only need to cast here because I want to avoid a warning from gcc's -Wall option;
    //it would work in any case, probably
    if(debug){fprintf(stderr, "##sorted. time to print. array size %d\n", i);}
    for(int i = 0; i < DIRECTORY_ENTRIES_PER_BLOCK; i++){
      if(names[i]){
	printf("%s%s\n",names[i],"/"); //TODO: actually intelligently detect dirs
	//(currently hard-codes dir assumption)
      }
    }
  }
  //ERROR CASE
  return EXIT_FAILURE;
}

int oufs_list(const char *cwd, const char *path){
  //setbuf(stdout, NULL); //might need this line some day
  //remember, cwd will be / by default, and path will be an empty string by default.
  BLOCK_REFERENCE br;
  BLOCK_REFERENCE pbr;
  INODE_REFERENCE iop;
  char name[FILE_NAME_SIZE];
  oufs_find_file(cwd, path, &pbr, &br, name, &iop);
  print_dir(br);
  return EXIT_SUCCESS;
}

int oufs_mkdir(const char *cwd, const char *path){
  //remember, by default cwd will be "/" and path will be ""
  BLOCK_REFERENCE br;
  BLOCK_REFERENCE pbr;
  INODE_REFERENCE iop;
  char name[FILE_NAME_SIZE];
  oufs_find_file(cwd, path, &pbr, &br, name, &iop);
  if(br != UNALLOCATED_BLOCK){
    fprintf(stderr,"path already exists\n");
    return EXIT_FAILURE;    
  }
 
  //check if pbr is full
  //a bit awkward because we don't want to actually do any operations yet
  if(is_full(pbr)){
    fprintf(stderr,"dir full\n");
    return EXIT_FAILURE;    
  }
  dprintf("everything valid, so now we have to perform the operations\n");
  //first, allocate the new block
  br = oufs_allocate_new_block();
  if(br == UNALLOCATED_BLOCK){
    return EXIT_FAILURE;
  }
  INODE_REFERENCE ir = oufs_allocate_new_inode(IT_DIRECTORY, br);
  if(ir == UNALLOCATED_INODE){
    //must deflag the block we just flagged
    BLOCK b;
    vdisk_read_block(MASTER_BLOCK_REFERENCE, &b);
    b.master.block_allocated_flag[br/8] ^= (1 << (br % 8)); //ooo so much arithmetic! magic
    vdisk_write_block(MASTER_BLOCK_REFERENCE, &b);
    return EXIT_FAILURE;
  }
  //next, make the inode that will point to the block, and insert a reference to it in the last block.
  BLOCK b;
  BLOCK lastb;
  vdisk_read_block(pbr, &lastb);
  add_inode_to_block(&lastb, ir, name);
  //adjust inode size
  INODE i;
  oufs_read_inode_by_reference(iop, &i);
  i.size += 1;
  oufs_write_inode_by_reference(iop, &i);
  //hardcoded location of .. in next line
  oufs_clean_directory_block(ir,lastb.directory.entry[0].inode_reference,&b);
  vdisk_write_block(pbr, &lastb);
  vdisk_write_block(br, &b);
  return EXIT_SUCCESS;
}

int oufs_rmdir(const char *cwd, const char *path){
    //remember, cwd will be / by default, and path will be an empty string by default.
  BLOCK_REFERENCE br;
  BLOCK_REFERENCE pbr;
  INODE_REFERENCE iop;
  char name[FILE_NAME_SIZE];
  oufs_find_file(cwd, path, &pbr, &br, name, &iop);
  if(br == UNALLOCATED_BLOCK){
    fprintf(stderr,"path doesn't exist\n");
    return EXIT_FAILURE;    
  }
 
  //check if br is empty. a bit awkward because we don't want to actually do any operations yet
  if(!is_empty(br)){
    fprintf(stderr,"dir nonempty");
    return EXIT_FAILURE;    
  }
  dprintf("everything valid, so now we have to perform the operations\n");
  BLOCK b;
  BLOCK lastb;
  INODE_REFERENCE ir;
  vdisk_read_block(pbr, &lastb);
  ir = rm_from_block(name, &lastb);
  vdisk_write_block(pbr, &lastb);
  //deallocate the inode (possibly gratuitous)
  vdisk_read_block(ir2br(ir), &b);
  b.inodes.inode[ir2ei(ir)] = new_inode(IT_NONE, UNALLOCATED_BLOCK); 
  vdisk_write_block(ir2br(ir), &b);
  ////deallocate the block (poss. grat.)
  //vdisk_read_block(br, &b);
  //eh... decided not to do it

  //adjust inode size
  INODE i;
  oufs_read_inode_by_reference(iop, &i);
  i.size -= 1;
  oufs_write_inode_by_reference(iop, &i);
  
  //deallocate master block bits
  vdisk_read_block(MASTER_BLOCK_REFERENCE, &b);
  b.master.inode_allocated_flag[ir / 8] ^= (1 << (ir % 8)); //ooo so much arithmetic! magic!
  b.master.block_allocated_flag[br/8] ^= (1 << (br % 8));
  vdisk_write_block(MASTER_BLOCK_REFERENCE, &b);
  return EXIT_SUCCESS;
}
