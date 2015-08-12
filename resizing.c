// Last updated 2008/11/04 10:53

// convert logo: -filter lanczos -resize 50% -quality 95 logo_resize.jpg
// Read an image, resize it by 50% and sharpen it, and then save as
// a high quality JPG
// Note that ImageMagick's default quality is 75.

#include <stdio.h>
#include <wand/magick_wand.h>
#include "fileman.h"

int resizing(const char *image_name,const char *new_image_path,int width,int height)
{
	MagickWand *m_wand = NULL;
	
	int old_width, old_height;
	
	MagickWandGenesis();
	
	m_wand = NewMagickWand();
	// Read the image - all you need to do is change "logo:" to some other
	// filename to have this resize and, if necessary, convert a different file
	MagickReadImage(m_wand,image_name);

	// Get the image's width and height
	//--->old_width = MagickGetImageWidth(m_wand);
	//--->old_height = MagickGetImageHeight(m_wand);
	// printf("%d %d", width /= 2,height);	

	// Cut them in half but make sure they don't underflow
	//if(width > 250) width = 250;
	//if(height > 300) height = 300;
	
	// Resize the image using the Lanczos filter
	// The blur factor is a "double", where > 1 is blurry, < 1 is sharp
	// I haven't figured out how you would change the blur parameter of MagickResizeImage
	// on the command line so I have set it to its default of one.
	MagickResizeImage(m_wand,width,height,LanczosFilter,1);
	
	// Set the compression quality to 95 (high quality = low compression)
	MagickSetImageCompressionQuality(m_wand,95);
	
	/* Writethe new image */

	MagickWriteImage(m_wand,new_image_path);

	/* Clean up */
	if(m_wand)m_wand = DestroyMagickWand(m_wand);
	
	MagickWandTerminus();
}