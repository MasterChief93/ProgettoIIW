#include <wand/magick_wand.h>
#include "fileman.h"
#ifndef FILEMAN_H                         //Include Guards per evitare di includere più volte l'header (ed avere quindi funzioni definite più volte)
#define FILEMAN_H
extern int resizing(const char *image_name,const char *new_image_path,int width,int height,float quality);
#endif