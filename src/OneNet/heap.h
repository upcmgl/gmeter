#pragma once
#ifndef HEAP_H
#define HEAP_H


#ifdef __cplusplus
extern "C" {
#endif

void *pvPortMalloc(long unsigned int xWantedSize);
void vPortFree(void *pv);

#ifdef __cplusplus
}
#endif
#endif // !HEAP_H
