#ifndef _LOG_H_
#define _LOG_H_


#ifdef USE_DEBUGGER

#define DPRINTF(...)  e_printf(__VA_ARGS__) 
#else
#define DPRINTF(...)

#endif

#endif