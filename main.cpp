#include "CImg.h"
#include <iostream>
#include <vector>

using namespace cimg_library;
using namespace std;

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

void regionGrowing(CImg<> imgInit, CImg<>* imgTrait, vector<int> seedPoint, bool* fini)
{
	vector< vector<int> > region;
	region.push_back(seedPoint);
	float threshold = 20.f;

	for(int i = 0; i < region.size(); i++)
	{
		vector<int> voisin1 = {region[i][0]-1, region[i][1], region[i][2]};
		vector<int> voisin2 = {region[i][0]-1, region[i][1], region[i][2]};
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
					if(distance <= threshold)
					{
						region.push_back(voisins[j]);
						(*imgTrait)(voisins[j][0], voisins[j][1], voisins[j][2]) = 255;
					}
			}
		}
	}

	cout << "fini" << endl;
	(*fini) = true;
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

  	remiseAZero(&imgTrait, dim);

  	bool testFini = false;

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
    }

    //imgTrait.save_analyze("result.hdr", voxelsize);

    //const CImg<float> img = CImg<unsigned char>("reference.jpg").resize(-100,-100,20);
	CImgList<unsigned int> faces3d;
	CImgList<unsigned char> colors3d;

	const CImg<float> points3d = imgTrait.get_isosurface3d(faces3d,100);
	CImg<unsigned char>().display_object3d("Isosurface3d",points3d,faces3d,colors3d);

}