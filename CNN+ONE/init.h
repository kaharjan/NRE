#ifndef INIT_H
#define INIT_H
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>
#include <cstdio>
#include <float.h>
#include <cmath>

using namespace std;


string version = "";
int output_model = 0;

//int num_threads = 10;
int num_threads = 1;
int trainTimes = 15;
float alpha = 0.02;
float reduce = 0.98;
int tt,tt1;
int dimensionC = 230;//1000;
int dimensionWPE = 5;//25;
int window = 3;
int limit = 30;
float marginPositive = 2.5;
float marginNegative = 0.5;
float margin = 2;
float Belt = 0.001;
float *matrixB1, *matrixRelation, *matrixW1, *matrixRelationDao, *matrixRelationPr, *matrixRelationPrDao;
float *matrixB1_egs, *matrixRelation_egs, *matrixW1_egs, *matrixRelationPr_egs;
float *matrixB1_exs, *matrixRelation_exs, *matrixW1_exs, *matrixRelationPr_exs;
float *wordVecDao,*wordVec_egs,*wordVec_exs;
float *positionVecE1, *positionVecE2, *matrixW1PositionE1, *matrixW1PositionE2;
float *positionVecE1_egs, *positionVecE2_egs, *matrixW1PositionE1_egs, *matrixW1PositionE2_egs, *positionVecE1_exs, *positionVecE2_exs, *matrixW1PositionE1_exs, *matrixW1PositionE2_exs;
float *matrixW1PositionE1Dao;
float *matrixW1PositionE2Dao;
float *positionVecDaoE1;
float *positionVecDaoE2;
float *matrixW1Dao;
float *matrixB1Dao;
double mx = 0;
int batch = 16;
int npoch;
int len;
float rate = 1;
//for log
FILE *logg;

float *wordVec;
int wordTotal, dimension, relationTotal;
int PositionMinE1, PositionMaxE1, PositionTotalE1,PositionMinE2, PositionMaxE2, PositionTotalE2;
map<string,int> wordMapping;
vector<string> wordList;
map<string,int> relationMapping;
vector<int *> trainLists, trainPositionE1, trainPositionE2;
vector<int> trainLength;
vector<int> headList, tailList, relationList;
vector<int *> testtrainLists, testPositionE1, testPositionE2;
vector<int> testtrainLength;
vector<int> testheadList, testtailList, testrelationList;
vector<std::string> nam;


map<string,vector<int> > bags_train, bags_test;

void init() {
	FILE *f = fopen("../data/vec.bin", "rb");
	FILE *fout = fopen("../data/vector1.txt", "w");
	//logg = fopen("./CNN+one.log", "w");
	fscanf(f, "%d", &wordTotal);
	fscanf(f, "%d", &dimension);
	cout<<"wordTotal=\t"<<wordTotal<<endl;
	cout<<"Word dimension=\t"<<dimension<<endl;
	PositionMinE1 = 0;
	PositionMaxE1 = 0;
	PositionMinE2 = 0;
	PositionMaxE2 = 0;
	wordVec = (float *)malloc((wordTotal+1) * dimension * sizeof(float));
	wordList.resize(wordTotal+1);
	wordList[0] = "UNK";
	for (int b = 1; b <= wordTotal; b++) {
		string name = "";
		//write word in vector.bin to vector1.txt,like <s>, bella, one ...
		while (1) {
			char ch = fgetc(f);
			if (feof(f) || ch == ' ') break;
			if (ch != '\n') name = name + ch;
		}
		fprintf(fout, "%s", name.c_str());
		int last = b * dimension;
		float smp = 0;
		for (int a = 0; a < dimension; a++) {
			//read by calling fgetc sizeof(float)*1 time from f, and put to wordVect
			fread(&wordVec[a + last], sizeof(float), 1, f);

			smp += wordVec[a + last]*wordVec[a + last];
		}
		// smp=sqrt(n1*n1+n2*n2+...n50*n50)
		smp = sqrt(smp);
		//n1=n1/smp
		for (int a = 0; a< dimension; a++)
		{
			wordVec[a+last] = wordVec[a+last] / smp;
			fprintf(fout, "\t%f", wordVec[a+last]);
		}
		fprintf(fout,"\n");
		//inverted word table 
		wordMapping[name] = b;
		wordList[b] = name;
	}//for 
	fclose(fout);
	wordTotal+=1;
	fclose(f);
	char buffer[1000];
	f = fopen("../data/RE/relation2id.txt", "r");
	while (fscanf(f,"%s",buffer)==1) {
		int id;
		fscanf(f,"%d",&id);
		relationMapping[(string)(buffer)] = id;
		relationTotal++;
		nam.push_back((std::string)(buffer));
	}
	fclose(f);
	cout<<"relationTotal:\t"<<relationTotal<<endl;
	
	f = fopen("../data/RE/train2.txt", "r");
	while (fscanf(f,"%s",buffer)==1)  {
		string e1 = buffer;
		fscanf(f,"%s",buffer);
		string e2 = buffer;
		fscanf(f,"%s",buffer);
		string head_s = (string)(buffer);
		int head = wordMapping[(string)(buffer)];
		fscanf(f,"%s",buffer);
		int tail = wordMapping[(string)(buffer)];
		string tail_s = (string)(buffer);
		fscanf(f,"%s",buffer);
		//bags_train["m.0ycvs\tm.02pxj3t\tcontatins"]=<0>
		fprintf(logg, "\nheadList.size()= %d\n", headList.size());
		string ktmp = e1 + "\t" + e2 + "\t" + (string)(buffer);
		bags_train[e1+"\t"+e2+"\t"+(string)(buffer)].push_back(headList.size());
		//fprintf(logg, "bags_train[e1+e2+(string)(buffer)]= %d", bags_train[ktmp]);

		//kahar 
		fprintf(logg, "bags_train[ %s],", ktmp.c_str());
		//fprintf(logg, ".size()= %d\t\n", bags_train[ktmp].size());
		for (int i = 0; i < bags_train[ktmp].size(); i++)
			fprintf(logg, "%d\t", bags_train[ktmp][i]);

		//relation id
		int num = relationMapping[(string)(buffer)];
		int len = 0, lefnum = 0, rignum = 0;
		std::vector<int> tmpp;
		while (fscanf(f,"%s", buffer)==1) {
			std::string con = buffer;
			if (con=="###END###") break;
			//gg is word id
			int gg = wordMapping[con];
			if (con == head_s) lefnum = len;
			if (con == tail_s) rignum = len;
			len++;
			tmpp.push_back(gg);
		}//while
		//headList contains head entit's wordvec id
		headList.push_back(head);
		//tailList contains tail entit's wordvec id
		tailList.push_back(tail);
		//tailList contains relation id
		relationList.push_back(num);
		//trainLenght contains number of words in one sentence
		trainLength.push_back(len);
		int *con=(int *)calloc(len,sizeof(int));
		int *conl=(int *)calloc(len,sizeof(int));
		int *conr=(int *)calloc(len,sizeof(int));
		for (int i = 0; i < len; i++) {
			//con is wordvec id in the sentence
			con[i] = tmpp[i];
			conl[i] = lefnum - i;
			conr[i] = rignum - i;
			if (conl[i] >= limit) conl[i] = limit;
			if (conr[i] >= limit) conr[i] = limit;
			if (conl[i] <= -limit) conl[i] = -limit;
			if (conr[i] <= -limit) conr[i] = -limit;
			if (conl[i] > PositionMaxE1) PositionMaxE1 = conl[i];
			if (conr[i] > PositionMaxE2) PositionMaxE2 = conr[i];
			if (conl[i] < PositionMinE1) PositionMinE1 = conl[i];
			if (conr[i] < PositionMinE2) PositionMinE2 = conr[i];
		}
		//trainLists contains word vect id in every sentence
		trainLists.push_back(con);
		//trainPositionE1 contains relitve positon of word corresponding to e1
		trainPositionE1.push_back(conl);
		//trainPositionE1 contains relitve positon of word corresponding to e2
		trainPositionE2.push_back(conr);
	}//while(f

	//for (map<string, vector<int> >::iterator it = bags_train.begin(); it != bags_train.end(); it++)
	//{
	//	fprintf(logg, "first value %s\t", it->first.c_str());
	//	//fprintf(logg, "second value %s\t", it->second.siz());
	//	fprintf(logg, "second value size %s\t\n", it->second.size());

	//}


	fclose(f);

	f = fopen("../data/RE/test2.txt", "r");	
	while (fscanf(f,"%s",buffer)==1)  {
		string e1 = buffer;
		fscanf(f,"%s",buffer);
		string e2 = buffer;
		bags_test[e1+"\t"+e2].push_back(testheadList.size());
		fscanf(f,"%s",buffer);
		string head_s = (string)(buffer);
		int head = wordMapping[(string)(buffer)];
		fscanf(f,"%s",buffer);
		string tail_s = (string)(buffer);
		int tail = wordMapping[(string)(buffer)];
		fscanf(f,"%s",buffer);
		int num = relationMapping[(string)(buffer)];
		int len = 0 , lefnum = 0, rignum = 0;
		std::vector<int> tmpp;
		while (fscanf(f,"%s", buffer)==1) {
			std::string con = buffer;
			if (con=="###END###") break;
			int gg = wordMapping[con];
			if (head_s == con) lefnum = len;
			if (tail_s == con) rignum = len;
			len++;
			tmpp.push_back(gg);
		}
		testheadList.push_back(head);
		testtailList.push_back(tail);
		testrelationList.push_back(num);
		testtrainLength.push_back(len);
		int *con=(int *)calloc(len,sizeof(int));
		int *conl=(int *)calloc(len,sizeof(int));
		int *conr=(int *)calloc(len,sizeof(int));
		for (int i = 0; i < len; i++) {
			con[i] = tmpp[i];
			conl[i] = lefnum - i;
			conr[i] = rignum - i;
			if (conl[i] >= limit) conl[i] = limit;
			if (conr[i] >= limit) conr[i] = limit;
			if (conl[i] <= -limit) conl[i] = -limit;
			if (conr[i] <= -limit) conr[i] = -limit;
			if (conl[i] > PositionMaxE1) PositionMaxE1 = conl[i];
			if (conr[i] > PositionMaxE2) PositionMaxE2 = conr[i];
			if (conl[i] < PositionMinE1) PositionMinE1 = conl[i];
			if (conr[i] < PositionMinE2) PositionMinE2 = conr[i];
		}
		testtrainLists.push_back(con);
		testPositionE1.push_back(conl);
		testPositionE2.push_back(conr);
	}
	fclose(f);
	cout<<PositionMinE1<<' '<<PositionMaxE1<<' '<<PositionMinE2<<' '<<PositionMaxE2<<endl;

	for (int i = 0; i < trainPositionE1.size(); i++) {
		int len = trainLength[i];
		int *work1 = trainPositionE1[i];
		for (int j = 0; j < len; j++)
			work1[j] = work1[j] - PositionMinE1;
		int *work2 = trainPositionE2[i];
		for (int j = 0; j < len; j++)
			work2[j] = work2[j] - PositionMinE2;
	}

	for (int i = 0; i < testPositionE1.size(); i++) {
		int len = testtrainLength[i];
		int *work1 = testPositionE1[i];
		for (int j = 0; j < len; j++)
			work1[j] = work1[j] - PositionMinE1;
		int *work2 = testPositionE2[i];
		for (int j = 0; j < len; j++)
			work2[j] = work2[j] - PositionMinE2;
	}
	PositionTotalE1 = PositionMaxE1 - PositionMinE1 + 1;
	PositionTotalE2 = PositionMaxE2 - PositionMinE2 + 1;
}

float CalcTanh(float con) {
	if (con > 20) return 1.0;
	if (con < -20) return -1.0;
	float sinhx = exp(con) - exp(-con);
	float coshx = exp(con) + exp(-con);
	return sinhx / coshx;
}

float tanhDao(float con) {
	float res = CalcTanh(con);
	return 1 - res * res;
}

float sigmod(float con) {
	if (con > 20) return 1.0;
	if (con < -20) return 0.0;
	con = exp(con);
	return con / (1 + con);
}

int getRand(int l,int r) {
	int len = r - l;
	//from 0 to RAND_MAX   x*x%len
	int res = rand()*rand() % len;
	if (res < 0)
		res+=len;
	return res + l;
}

float getRandU(float l, float r) {
	float len = r - l;
	//RAND_MAX=32767
	float res = (float)(rand()) / RAND_MAX;
	return res * len + l;
}

void norm(float* a, int ll, int rr)
{
	float tmp = 0;
	for (int i=ll; i<rr; i++)
		tmp+=a[i]*a[i];
	if (tmp>1)
	{
		tmp = sqrt(tmp);
		for (int i=ll; i<rr; i++)
			a[i]/=tmp;
	}
}


#endif
