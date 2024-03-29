#include <stdio.h>

int main(int argc, char** argv)
{
  //default bounds:
  //u is the first char, l the last
  //ironically, u has the lower numerical value
  unsigned char u = ' ';
  unsigned char l = '}';

  //read args if appropriate:
  if(argc >  1){
    u = argv[1][0];
  }
  if(argc > 2){
    l = argv[2][0];
  }
  
  //initialize array of char counts:
  int counts[256];
  for(unsigned int i = 0; i < 256; i++){
    counts[i]=0;
  }
  
  //read in chars from stdin and tally them:
  unsigned int c;
  while((c = getchar()) != EOF){
    counts[c]++;
  }

  //print output:
  printf("Range: %c-%c\n", u, l);
  for(unsigned int i = u; i <= l; i++){
    printf("%c: %4d ", i, counts[i]);
    for(unsigned int j = 0; j < counts[i]; j++){
      //this for loop makes the bar in the bar graph
      printf("#");
    }
    puts(""); //newline
  }
}
