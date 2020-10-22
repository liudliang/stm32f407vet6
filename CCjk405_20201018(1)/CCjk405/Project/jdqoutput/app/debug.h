//debug.h
//debug.h
#ifndef _DEGUG_H_0001
#define _DEGUG_H_0001

//patli 20200204 #define DEBUG


//inline void Assert(char * filename, unsigned int lineno)
//{
//    while(1);
//}

#ifdef DEBUG
	#define ASSERT(condition)  \
	do{   \
    if(condition)\
       NULL; \
    else\
      while(1); \
	}while(0)
	
#else
	 #define ASSERT(condition) 
#endif



#endif
	
	
	
	
	
	

	
