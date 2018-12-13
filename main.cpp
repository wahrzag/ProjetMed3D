#include "CImg.h"

using namespace cimg_library;

int main(int argc, char *argv[])
{
	char cfichierlu[250];

	if (argc != 2)
	{
   		printf("Usage: fichier\n"); 
   		exit (1) ;
  	}

  	sscanf (argv[1],"%s",cfichierlu);
  	CImg<> img, imgAffiche;
  	float voxelsize[3];

  	img.load_analyze(cfichierlu, voxelsize);

  	int width = img.width();
 	int height = img.height();
 	int depth = img.depth();
 	

 	CImgDisplay display(1024, 1024, cfichierlu);

 	int numeroImage;

 	while(!display.is_closed())
 	{
    	if (display.is_keyESC())
    	{
      		break;
    	}
    
    	if (display.wheel())
    	{
      		numeroImage = abs(display.wheel()%depth);
    	}

    	imgAffiche = img.get_slice(numeroImage);
    	display.display(imgAffiche);
  		display.wait();
    }
}