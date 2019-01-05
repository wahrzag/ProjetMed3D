#include "CImg.h"
#include <iostream>
#include <vector>

using namespace cimg_library;
using namespace std;

#define NALDYDONE 0
#define IGNORE -1

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

void regionGrowing(CImg<> imgInit, CImg<>* imgTrait, vector<int> seedPoint, int* threshold)
{
	vector< vector<int> > region;
	region.push_back(seedPoint);

	for(int i = 0; i < region.size(); i++)
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
			if((*imgTrait)(voisins[j][0], voisins[j][1], voisins[j][2]) == 0)
			{
					float distance = (float) sqrt(pow((imgInit(voisins[j][0], voisins[j][1], voisins[j][2]) - imgInit(seedPoint[0], seedPoint[1], seedPoint[2])), 2)); //imgInit(region[i][0], region[i][1], region[i][2])), 2));
					if(distance <= *threshold)
					{
						region.push_back(voisins[j]);
						(*imgTrait)(voisins[j][0], voisins[j][1], voisins[j][2]) = 255;
					}
			}
		}
	}
}

void regionGrowingRec(CImg<> imgInit,CImg<>* labels,int* dim,int* seedPoint,int* threshold,int label){
  int i = seedPoint[0];
  int j = seedPoint[1];
  int k = seedPoint[2];
  (*labels)(i,j,k) = label;
  for(i=-1;i<=1;i++)
  {
    for(j=-1;j<=1;j++)
    {
      for(k=-1;k<=1;k++)
      { 
        bool todo = ((i>0)?i:-i + (j>0)?j:-j + (k>0)?k:-k) == 1;
        if(todo && (*labels)(seedPoint[0]+i,seedPoint[1]+j,seedPoint[2]+k) == NALDYDONE && distanceInt(imgInit(seedPoint[0],seedPoint[1],seedPoint[2]),imgInit(seedPoint[0]+i,seedPoint[1]+j,seedPoint[2]+k))<*threshold){
          int newSeed[3] = {seedPoint[0]+i,seedPoint[1]+j,seedPoint[2]+k};
          regionGrowingRec(imgInit,labels,dim,newSeed,threshold,label);
        }
      }
    }
  }
}

void regionGrowingPreT(CImg<> imgInit,CImg<>* labels,int* dim){
  for(int i = 0; i < dim[0]; i++)
  {
    for(int j = 0; j < dim[1]; j++)
    {
      for(int k = 0; k < dim[2]; k++)
      {
        if ((imgInit)(i,j,k) == 0) (*labels)(i,j,k)=IGNORE;
        else{
          (*labels)(i,j,k)=NALDYDONE;
        } 
      }
    }
  }
}

void PaintWithLabels(CImg<> imgInit,CImg<>* imgTrait,CImg<> labels,int* dim){
  int count=0;
  for(int i = 0; i < dim[0]; i++)
  {
    for(int j = 0; j < dim[1]; j++)
    {
      for(int k = 0; k < dim[2]; k++)
      {
        if (labels(i,j,k) >= 1){
          (*imgTrait)(i,j,k)=imgInit(i,j,k);
          count++;
        }
        else (*imgTrait)(i,j,k)=0;
      }
    }
  }
  fprintf(stderr,"nb voxels : %d\n",count);
}

void firstContact(CImg<> imgInit, CImg<>* imgTrait, CImgDisplay* display,int* dim,int* threshold){
  remiseAZero(imgTrait,dim);
  int numeroImage=dim[2]/2;
  int clicX, clicY, clicZ;
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
        clicX = (*display).mouse_x() * Cx;
        clicY = (*display).mouse_y() * Cy;
        clicZ = numeroImage;
        //int seed[3] = {clicX, clicY, clicZ};
        vector<int> seed = {clicX, clicY, clicZ};
        regionGrowing(imgInit, imgTrait, seed,threshold);
        fprintf(stderr,"Fin du region growing\n");
        //regionGrowingRec(imgInit, labels, dim,seed,threshold,1);
        return;
      }
      CImg<> imgAffiche = imgInit.get_slice(numeroImage);
      (*display).display(imgAffiche);
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
 	CImg<> imgInit, imgTrait, labels;
 	float voxelsize[3];
  imgInit.load_analyze(cfichierlu, voxelsize);
  imgTrait.load_analyze(cfichierlu, voxelsize);
  labels.load_analyze(cfichierlu,voxelsize);

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

  cout << "Veuillez cliquer sur la région à segmenter" << endl;
  firstContact(imgInit,&imgTrait,&display,dim,&threshold);
  //PaintWithLabels(imgInit,&imgTrait,labels,dim);

  /*bool testFini = false;

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
	        vector<int> seed = {clicX, clicY, clicZ};

	        regionGrowing(imgInit, &imgTrait, seed, &testFini);
      	}

      	if(testFini)
      	{
      		imgAffiche = imgTrait.get_slice(numeroImage);
      	}
      	else
      	{
      		imgAffiche = imgInit.get_slice(numeroImage);
      	}
    	
    	display.display(imgAffiche);
  		display.wait();
    }*/

    //imgTrait.save_analyze("result.hdr", voxelsize);

    //const CImg<float> img = CImg<unsigned char>("reference.jpg").resize(-100,-100,20);
	CImgList<unsigned int> faces3d;
	CImgList<unsigned char> colors3d;

	const CImg<float> points3d = imgTrait.get_isosurface3d(faces3d,100);
	CImg<unsigned char>().display_object3d("Isosurface3d",points3d,faces3d,colors3d);

}