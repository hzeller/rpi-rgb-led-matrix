#ifndef SHIMS_PIL_H
#define SHIMS_PIL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ImagingMemoryInstance ImagingMemoryInstance;

int** get_image32(void* im);

#ifdef __cplusplus
}
#endif

#endif
