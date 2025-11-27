#include "Imaging.h"
#include "pillow.h"

int** get_image32(void* im) {
    ImagingMemoryInstance* image = (ImagingMemoryInstance*) im;
    return image->image32;
}
