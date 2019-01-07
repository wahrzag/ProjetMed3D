#include "CImg.h"
#include <iostream>
#include <vector>

using namespace cimg_library;
using namespace std;

float distanceInt(int p1,int p2){
  return (p1<p2)?p2-p1:p1-p2;
}

int clamp(int val, int bornInf, int bornSup)
{
	return (val<bornInf)?bornInf:(val>bornSup)?bornSup:val;
}

void Seuil(CImg<> imgInit, CImg<>* imgTrait, int seuil, int* dim)
{
	int valMax = imgInit.max();
  	for(int i = 0; i < dim[0]; i++)
  	{
    	for(int j = 0; j < dim[1]; j++)
    	{
      		for(int k = 0; k < dim[2]; k++)
      		{
        		int valAct = (imgInit(i,j,k)>seuil)?imgInit(i,j,k):0;
        		(*imgTrait)(i,j,k) = (valAct-seuil)*255/(valMax-seuil);
        		//fprintf(stderr,"seuil %d et valMax %d\n",seuil,valMax);
      		}
    	}
  	}
}

void remiseAZero(CImg<>* imgTrait, int* dim)
{
  for(int i = 0; i < dim[0]; i++)
  {
    for(int j = 0; j < dim[1]; j++)
    {
      for(int k = 0; k < dim[2]; k++)
      {
        (*imgTrait)(i,j,k) = 0;
      }
    }
  }
}

void CopyImg(CImg<> imgInit, CImg<>* imgTrait, int* dim)
{
  	int valMax = imgInit.max();
  	for(int i = 0; i < dim[0]; i++)
  	{
    	for(int j = 0; j < dim[1]; j++)
    	{
      		for(int k = 0; k < dim[2]; k++)
      		{
      			if((imgInit)(i,j,k) < 0)
        			(*imgTrait)(i,j,k) = 0;
        		//fprintf(stderr,"seuil %d et valMax %d\n",seuil,valMax);
      		}
    	}
  	}
}

void pretraitement(CImg<> imgInit, CImg<>* imgTrait, CImgDisplay* display, int* seuil, int* dim)
{
  	int numeroImage = dim[2]/2;
  	for(bool preTrait = false;!(*display).is_closed() && !preTrait;)
  	{
    	Seuil(imgInit,imgTrait,*seuil,dim);
    	if ((*display).wheel())
    	{
      		*seuil += (*display).wheel();
      		*seuil = clamp(*seuil,0,254);
      		cout << "seuil " << *seuil << endl;
      		(*display).set_wheel();
    	}
    	CImg<> imgAffiche = (*imgTrait).get_slice(numeroImage);
    	(*display).display(imgAffiche);
    	(*display).wait();
    	if((*display).is_keyS()) preTrait = true;
  	}
}

void regionGrowing(CImg<> imgInit, CImg<>* imgTrait, vector<int> seedPoint,int* threshold,int* dim)
{
	vector< vector<int> > region;
	region.push_back(seedPoint);
	for(int i = 0; i < region.size(); i++)
	{  
		if((*imgTrait)(region[i][0], region[i][1], region[i][2]) != 255)
		{
			vector<int> voisin1 = {region[i][0]-1, region[i][1], region[i][2]};
			vector<int> voisin2 = {region[i][0]+1, region[i][1], region[i][2]};
			vector<int> voisin3 = {region[i][0], region[i][1]-1, region[i][2]};
			vector<int> voisin4 = {region[i][0], region[i][1]+1, region[i][2]};
			vector<int> voisin5 = {region[i][0], region[i][1], region[i][2]-1};
			vector<int> voisin6 = {region[i][0], region[i][1], region[i][2]+1};

			vector< vector<int> > voisins = {voisin1, voisin2, voisin3, voisin4, voisin5, voisin6};
			for(int j = 0; j < voisins.size(); j++)
			{  
        if(voisins[j][0]<0 || voisins[j][1]<0 || voisins[j][2]<0) continue;
        if(voisins[j][0]>dim[0]-1 || voisins[j][1]>dim[1]-1 || voisins[j][2]>dim[2]-1) continue;
				if((*imgTrait)(voisins[j][0], voisins[j][1], voisins[j][2]) != 255)
				{
					int a = abs(imgInit(voisins[j][0], voisins[j][1], voisins[j][2]) - imgInit(seedPoint[0], seedPoint[1], seedPoint[2]));
					int b = abs(imgInit(voisins[j][0], voisins[j][1], voisins[j][2]) - imgInit(region[i][0], region[i][1], region[i][2]));
					
					if(a <= *threshold*2 && b <= *threshold)
					{
						region.push_back(voisins[j]);
						(*imgTrait)(region[i][0], region[i][1], region[i][2]) = 255;
					}
				}
			}
		}
		
	}
}

void firstContact(CImg<> imgInit, CImg<>* imgTrait, CImgDisplay* display,int* dim,int* threshold,int* clicX,int* clicY,int* clicZ){
  remiseAZero(imgTrait,dim);
  int numeroImage=dim[2]/2;
  float Cx = (float) dim[0] / (float) (*display).width();
  float Cy = (float) dim[1] / (float) (*display).height();
  while(!(*display).is_closed())
  {
      if ((*display).is_keyESC())
      {
          break;
      }
    
      if ((*display).wheel())
      {
          numeroImage += (*display).wheel();
          numeroImage = clamp(numeroImage,0,dim[2]);
          (*display).set_wheel();
      }
      if ((*display).button()&1)
      { 
        fprintf(stderr,"Debut du region growing\n");
        *clicX = (*display).mouse_x() * Cx;
        *clicY = (*display).mouse_y() * Cy;
        *clicZ = numeroImage;
        vector<int> seed = {*clicX, *clicY, *clicZ};
        regionGrowing(imgInit, imgTrait, seed,threshold,dim);
        fprintf(stderr,"Fin du region growing\n");
        return;
      }
      CImg<> imgAffiche = imgInit.get_slice(numeroImage);
      (*display).display(imgAffiche);
      (*display).wait();
    }
}

void MainLoop(CImg<> imgInit, CImg<>* imgTrait, CImgDisplay* display,int* dim,int* threshold,int* clicX,int* clicY,int* clicZ){
  int numeroImage=dim[2]/2;
  float Cx = (float) dim[0] / (float) (*display).width();
  float Cy = (float) dim[1] / (float) (*display).height();
  CImgDisplay displayTrait(*imgTrait);
  int oldTresh = *threshold;
  bool anyChange = true;
  cout << "Si modification similarité entre voxels, fermez visualisation surfacique" << endl;
  cout << "Appuyez sur U (monter) ou D (descendre) pour changer de coupe sur l'axe z" << endl;
  cout << "Utiliser la molette pour changer la similarité" << endl;
  cout << "Appuyez sur A pour valider" << endl;
  while(!(*display).is_closed())
  {   
      if ((*display).is_keyESC())
      {
          break;
      }
      if((*display).is_keyU()){
        numeroImage += 1;
        numeroImage = clamp(numeroImage,0,dim[2]);
        //fprintf(stderr,"z : %d\n",numeroImage);
      }
      if((*display).is_keyD()){
        numeroImage -= 1;
        numeroImage = clamp(numeroImage,0,dim[2]);
        //fprintf(stderr,"z : %d\n",numeroImage);
      }
      if ((*display).wheel())
      {
        *threshold += (*display).wheel();
        *threshold = clamp(*threshold,1,255);
        (*display).set_wheel();
        fprintf(stderr,"seuil d'acceptation : %d\n",*threshold);
      }
      if ((*display).button()&1)
      { 
        int nclicX = (*display).mouse_x() * Cx;
        int nclicY = (*display).mouse_y() * Cy;
        int nclicZ = numeroImage;
        if((*imgTrait)(nclicX,nclicY,nclicZ) || !imgInit(nclicX,nclicY,nclicZ)){
          if((*imgTrait)(nclicX,nclicY,nclicZ)) anyChange = true;
        }
        else{
          fprintf(stderr,"Debut du region growing\n");
          *clicX = nclicX;
          *clicY = nclicY;
          *clicZ = nclicZ;
          remiseAZero(imgTrait,dim);
          vector<int> seed = {*clicX, *clicY, *clicZ};
          regionGrowing(imgInit, imgTrait, seed,threshold,dim);
          fprintf(stderr,"Fin du region growing\n");
          anyChange=true;
        }
      }
      if((*display).is_keyA()){
        if(oldTresh != *threshold){
        	fprintf(stderr,"Debut du region growing\n");
          remiseAZero(imgTrait,dim);
          vector<int> seed = {*clicX, *clicY, *clicZ};
          regionGrowing(imgInit, imgTrait, seed,threshold,dim);
          fprintf(stderr,"Fin du region growing\n");
          anyChange=true;
        } 
        if(oldTresh==*threshold){
          continue;
        }
        oldTresh=*threshold;
      }
      CImg<> imgAffiche2 = (*imgTrait).get_slice(numeroImage);
      displayTrait.display(imgAffiche2);
      CImg<> imgAffiche = imgInit.get_slice(numeroImage);
      (*display).display(imgAffiche);
      if(anyChange){
        CImgList<unsigned int> faces3d;
        CImgList<unsigned char> colors3d;
        const CImg<float> points3d = (*imgTrait).get_isosurface3d(faces3d,100);
        CImg<unsigned char>().display_object3d("Isosurface3d",points3d,faces3d,colors3d);
        anyChange = false;
      }
      (*display).wait();
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
 	CImg<> imgInit, imgTrait;
 	float voxelsize[3];
  imgInit.load_analyze(cfichierlu, voxelsize);
  imgTrait.load_analyze(cfichierlu, voxelsize);

  int dim[] = {imgInit.width(),imgInit.height(),imgInit.depth()};
 	
 	CImgDisplay display(imgInit);

  int seuil = 30;
 	int numeroImage=dim[2]/2;
 	bool seuilFini = false;
 	bool sorti;

while(!seuilFini)
{
	cout << "Choisissez votre seuil avec la molette et appuyer sur S pour valider" << endl;
  	pretraitement(imgInit,&imgTrait,&display,&seuil,dim);
  	fprintf(stderr, "Le filtre passe-haut est choisi à %d\n",seuil);
  	cout << "Êtes vous sûr de votre choix ? (o/n)" << endl;

	for(sorti = false;!display.is_closed() && !sorti;)
  	{
  		if (display.wheel())
    	{
      		numeroImage += display.wheel();
          	numeroImage = clamp(numeroImage,0,dim[2]);
          	display.set_wheel();
    	}

    	CImg<> imgAffiche = imgTrait.get_slice(numeroImage);
    	display.display(imgAffiche);
    	display.wait();

    	if(display.is_keyO()) 
		{
			sorti = true;
			seuilFini = true;
		}

		if(display.is_keyN())
		{
			sorti = true;
			seuilFini = false;
		}
  	}
}
	
  CopyImg(imgTrait,&imgInit,dim);
  int threshold = 5;
  int clicX,clicY,clicZ;
  cout << "Veuillez cliquer sur la région à segmenter" << endl;
  firstContact(imgInit,&imgTrait,&display,dim,&threshold,&clicX,&clicY,&clicZ);
  MainLoop(imgInit,&imgTrait,&display,dim,&threshold,&clicX,&clicY,&clicZ);
}