/*
*/

#ifndef HR_GETTTIME_H

#define HR_GETTTIME_H

/* STM32F4xx: Timer2 initialization function */
void vStartHighResolutionTimer( void );

/* Get a high-resolution time: number of uS since booting the device. */
uint64_t ullGetHighResolutionTime( void );

#endif /* HR_GETTTIME_H */