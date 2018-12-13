#include "CImg.h"
#include <iostream>

using namespace cimg_library;
using namespace std;

int clamp(int val,int bornInf,int bornSup){
  return (val<bornInf)?bornInf:(val>bornSup)?bornSup:val;
}

void Seuil(CImg<> imgInit,CImg<>* imgTrait,int seuil,int* dim){
  int valMax = imgInit.max();
  for(int i=0;i<dim[0];i++){
    for(int j=0;j<dim[1];j++){
      for(int k=0;k<dim[2];k++){
        int valAct = (imgInit(i,j,k)>seuil)?imgInit(i,j,k):0;
        (*imgTrait)(i,j,k) = (valAct-seuil)*255/(valMax-seuil);
        //fprintf(stderr,"seuil %d et valMax %d\n",seuil,valMax);
      }
    }
  }
}

void CopyImg(CImg<> imgInit,CImg<>* imgTrait,int* dim){
  int valMax = imgInit.max();
  for(int i=0;i<dim[0];i++){
    for(int j=0;j<dim[1];j++){
      for(int k=0;k<dim[2];k++){
        (*imgTrait)(i,j,k) = (imgInit)(i,j,k);
        //fprintf(stderr,"seuil %d et valMax %d\n",seuil,valMax);
      }
    }
  }
}

void pretraitement(CImg<> imgInit,CImg<>* imgTrait,CImgDisplay* display,int* seuil,int* dim){
  int numeroImage=dim[2]/2;
  for(bool preTrait = false;!(*display).is_closed() && !preTrait;){
    Seuil(imgInit,imgTrait,*seuil,dim);
    if ((*display).wheel())
    {
      *seuil += (*display).wheel();
      *seuil = clamp(*seuil,0,254);
      (*display).set_wheel();
    }
    CImg<> imgAffiche = (*imgTrait).get_slice(numeroImage);
    (*display).display(imgAffiche);
    (*display).wait();
    if((*display).is_keyS()) preTrait=true;
  }
}

int main(int argc, char *argv[])
{
	char cfichierlu[250];

	if (argc != 2)
	{
  		printf("Usage: fichier\n"); 
  		exit (1) ;
  }

  sscanf (argv[1],"%s",cfichierlu);
  CImg<> imgInit, imgTrait, imgAffiche;
  float voxelsize[3];

  imgInit.load_analyze(cfichierlu, voxelsize);
  imgTrait.load_analyze(cfichierlu, voxelsize);

  int dim[] = {imgInit.width(),imgInit.height(),imgInit.depth()};
 	
 	CImgDisplay display(imgInit);

  int seuil = 30;
 	int numeroImage=dim[2]/2;

  pretraitement(imgInit,&imgTrait,&display,&seuil,dim);

  int clicX, clicY, clicZ;
  float Cx = (float) dim[0] / (float) display.width();
  float Cy = (float) dim[1] / (float) display.height();

  const unsigned char gray[] = {255};
  CopyImg(imgTrait,&imgInit,dim);
 	while(!display.is_closed())
 	{
    	if (display.is_keyESC())
    	{
      		break;
    	}
    
    	if (display.wheel())
    	{
      		numeroImage += display.wheel();
          numeroImage = clamp(numeroImage,0,dim[2]);
          display.set_wheel();
    	}

      if (display.button()&1)
      {
        clicX = display.mouse_x() * Cx;
        clicY = display.mouse_y() * Cy;
        clicZ = numeroImage;
        cout << clicX << " " << clicY << " " << clicZ << endl;
        imgTrait.draw_fill(clicX, clicY, clicZ, gray, 1.0, imgInit, 15.0, true);
      }

    	imgAffiche = imgInit.get_slice(numeroImage);
    	display.display(imgAffiche);
  		display.wait();
    }
}