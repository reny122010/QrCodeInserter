#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>

#include <cv.h>
#include <highgui.h>

#include <pthread.h>

int verbose_var = 0;
char temp2[50];
char file[50];

void usage(void){
	printf("Usage: ./qr [OPTIONS]\n");
	printf("       -f <file name>\n");
	printf("       -s <qr pixel s.width/s.height>\n");
	printf("       -l <lines to cut the video>\n");
	printf("       -c <collumns to cut the video>\n");
	printf("       -v <verbose mode>\n");
}

void verbose(const char* str){
	if(verbose_var){
		printf("\e[1;31m>>>%s\e[1;0m\n", str);
	}
}

void createQr1(long frame, int quadrant, int size){
	char toSys[100];
	char toSys2[100];
	char name[50];

	sprintf(name,"f%ldq%d",frame,quadrant);
	sprintf(toSys, "qrencode -s %d -m 1 -o %s.png '%s!'",size/21,name,name);
	system(toSys);
	sprintf(toSys2, "convert %s.png %s.jpg", name,name);
	system(toSys2);
}

char* convertQr(char* qrName){
	strcpy(temp2, qrName);
	char* iterator = temp2;
	while(*iterator != '.'){
		*(iterator)++;
	}
	*iterator = '\0';
	strcat(temp2,".jpg");
	return temp2;
}


char* nameqrjpg(long frame, int quadrant){
	char toSys[100];	
	char args[50];
	sprintf(args, "f%ldq%d", frame, quadrant);
	sprintf(file, "%s.jpg", args);
	return file;
}

void addQrCode(IplImage* image, long* frame, int line, int collumn, int size){
	int i = 0;
	int j = 0;
	int cont = 0;
	double alpha = .1, beta = .9;
	int width = image->width/collumn, height = image->height/line;

	IplImage* qrRead;

	char* qrName;

	while(i < line){
		while(j < collumn){
			qrName = nameqrjpg(*frame,cont);
			qrRead = cvLoadImage(qrName, CV_LOAD_IMAGE_COLOR);
			cvSetImageROI(qrRead, cvRect(0,0,size,size));
			cvSetImageROI(image, cvRect((width/2)-(size/2),(height/2)-(size/2),size,size));
			cvAddWeighted(image, alpha, qrRead, beta,0.0,image);
			cvResetImageROI(image);
			cvReleaseImage(&qrRead);
			j++;
			cont++;
			width += image->width;
		}
		width = image->width/collumn;
		height += image->height;
		i++;
		j = 0;
	}
}

int main(int argc, char** argv){
	IplImage* cvFrame = NULL;
	CvCapture* videoCapture	= NULL;
	CvCapture* videoCapture1	= NULL;
	CvSize frameSize;

	unsigned int size = 0;
	int ch = 0;    

	int lines = 0;
	int collumns = 0;

	long frame = 0;
	short quadrant;

	char* filename = NULL;

	if(argc < 5){
		printf("Unexpected amount of information!\n");
		usage();
		return 1;
	}

	while((ch = getopt(argc, argv, "f:s:l:c:v")) != -1){
		switch(ch){
			case 'f':
				filename = optarg;
				break;
			case 's':
				size = atoi(optarg);
				break;
			case 'l':
				lines = atoi(optarg);
			case 'c':
				collumns = atoi(optarg);
				break;
			case 'v':
				verbose_var = 1;
				break;
		}
	}
	size = (size/21)*21;
	int n =0;
	if(!(videoCapture1 = cvCaptureFromAVI(filename))){
		printf("Failed to open video, verify the path.\n");
		return 1;
	}
	printf("Criando e convertendo QRCode\n");
	int j = 0, cont = 0;

	while(1){
		if(!(cvFrame = cvQueryFrame(videoCapture1))){
			break;
		}
		while(n != lines*collumns){
			createQr1(frame, n, size);			
			n++;
		}
		frame++;
		n = 0; 
		j++;
	}

	frame = 0;

	if(!(videoCapture = cvCaptureFromAVI(filename))){
		printf("Failed to open video, verify the path.\n");
		return 1;
	}

	int flag = 0;
	if(!(cvFrame = cvQueryFrame(videoCapture))){
		return 1;
	}
	frameSize.width = cvFrame->width;
	frameSize.height = cvFrame->height;

	CvVideoWriter *writer = cvCreateVideoWriter("video.avi", CV_FOURCC('M','J','P','G'), 24, frameSize, 1);
	printf("Produzindo video com QrCode\n");
	while(1){
		if(frame != 0){
			if(!(cvFrame = cvQueryFrame(videoCapture))){
				break;
			}
		}
		addQrCode(cvFrame, &frame, lines, collumns, size);
		cvWriteFrame(writer, cvFrame);
		frame++;
	}	

	cvReleaseVideoWriter(&writer);
	system("rm *png");
	system("rm *jpg");
	return 0;
}