#include "CImg.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
using namespace cimg_library;
using namespace std;

const unsigned char gray[] = {255};

main(int argc, char *argv[])
{
    //LECTURE ET STOKAGE D'UNE IMAGE 3D
  int width, height, depth, D = 0;
  float voxelsize[3], Teta = 0;
  char cfichierlu[250];
  float X = -1,Y = -1, Z = -1;
  float Xmax1, Ymax1, Xmin1, Ymin1;
  float Xmax2, Ymax2, Xmin2, Ymin2;
  FILE *fichierCourbe = NULL;

  if (argc != 2){
   printf("Usage: fichier\n"); 
   exit (1) ;
  }

 sscanf (argv[1],"%s",cfichierlu);

 //fichierCourbe = fopen("courbe.dat","w");

 CImg<> img, img0, img1, img2, img3, img4, img5, imgAffiche;
 img.load_analyze(cfichierlu, voxelsize);
 img0 = img;

 width = img.width();
 height = img.height();
 depth = img.depth();

 int numeroImage, NbrVoxel,NbrVoxelTot = 0;

 CImgDisplay disp2(1024,1024,cfichierlu);
 float Cx = (float)width/disp2.width();
 float Cy = (float)height/disp2.height();

 while(!disp2.is_closed()){
    if (disp2.is_keyESC()){
      break;
    }
    
    if (disp2.wheel()){
      numeroImage = abs(disp2.wheel()%depth);
    }

    if (D == 0){
      if (disp2.button()&1){
        X = disp2.mouse_x()*Cx;
        Y = disp2.mouse_y()*Cy;
        Z = numeroImage%depth;
        Teta = img0(X,Y,Z)-1.0;
        //printf(" X %f Y %f Z %f val %f\n",X,Y,Z,Teta);
        D = 1;
      }
    }

    if (X != -1 && Y != -1){
 	    if (D == 1){
        if(fichierCourbe != NULL){
		      for (int i = Teta; i >= Teta-60; i--){
			       img2 = img0;
			       img2.threshold(i,false,false);


			       for(int z = 0; z < img2.depth(); z++)
				        for(int y = 0; y < img2.height(); y++) 
					         for(int x = 0; x < img2.width(); x++){
						          if (img2(x,y,z) != 0)
							           img2(x,y,z) = img0(x,y,z);
						          else
							           img2(x,y,z) = 0;
					         }
			       
             img2.draw_fill(X,Y,Z,gray,1.0,img4,25.0,false);
			       NbrVoxel = 0;
			       
             for(int z = 0; z < img2.depth(); z++)
				        for(int y = 0; y < img2.height(); y++) 
					         for(int x = 0; x < img2.width(); x++){
						          if(img2(x,y,z) != 0)
							           NbrVoxel += 1;
					           }

          // fprintf(fichierCourbe, "%d ", i);
          // fprintf(fichierCourbe, "%d\n", NbrVoxel);

			if (i == Teta-5){
				Xmax1 = Teta-5;
				Ymax1 = NbrVoxel;
			}
			else if (i == Teta-15) {
				Xmin1 = Teta-15;
				Ymin1 = NbrVoxel;
			}
			else if (i == Teta-45) {
				Xmax2 = Teta-45;
				Ymax2 = NbrVoxel;
			}
			else if (i == Teta-55) {
				Xmin2 = Teta-55;
				Ymin2 = NbrVoxel;
			}
		}
		D = 2;
	}
}

	img4.save_analyze("resultBrain.hdr",voxelsize);


	float P1 = (Ymax1-Ymin1)/(Xmax1-Xmin1);
	float P2 = (Ymax2-Ymin2)/(Xmax2-Xmin2);

	float B1 = -P1*(Teta-5)+Ymax1;
	float B2 = -P2*(Teta-55)+Ymax2;

	int seuilOpti = (B2-B1)/(P1-P2);
	printf("SEUIL OPTIMUM : %d", seuilOpti);

	img2 = img0;
	img2.threshold(seuilOpti,false,false);

	for(int z = 0; z < img2.depth(); z++)
		for(int y = 0; y < img2.height(); y++) 
			for(int x = 0; x < img2.width(); x++){
				if (img2(x,y,z) != 0){
					img2(x,y,z) = img0(x,y,z);
				}
				else {
					img2(x,y,z) = 0;
				}
			}
	img2.draw_fill(X,Y,Z,gray,1.0,img5,25.0,false);
	img5.save_analyze("result2Brain.hdr",voxelsize);
	imgAffiche = img5.get_slice(numeroImage);

  }
  else {
	imgAffiche = img0.get_slice(numeroImage);
  }
  disp2.display(imgAffiche);
  disp2.wait();
  }
fclose(fichierCourbe);
}


