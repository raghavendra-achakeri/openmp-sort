#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "psum.h"

using namespace std;

void psum(int *data, int size, int *data2){
	if (size<2)
		return;

	// #1: Pairwise sum
	int s = size>>1;
	#pragma omp parallel for
	for (int i=0; i< s; i++){
		data2[i] = data[2*i] + data[2*i+1];
	}
	// #2: Recurse
	psum(data2, s, data2+s);

	// #3: Finalize
	#pragma omp parallel for
	for (int i=0; i<s; i++){
		data[2*i] = data2[i] -data[2*i+1];
		data[2*i + 1] = data2[i];
	}

	if (size&1){
		data[size-1] = data[size-1] + data[size -2];
	}
}


void swap(dataType *data, int left, int right){
	dataType temp = data[left];
	data[left] = data[right];
	data[right] = temp;
}

int lpartition(dataType *data, int size){
	if (size==2){
		if (data[0].key>data[1].key){
			swap(data,0,1);
		}
		return 1;
	}
	int pivot = 0;
	int start = 1;
	int end = size-1;
	while (start <end){
		while (start<end && data[start].key<=data[pivot].key){
			start++;
		}
		while (start<end && data[end].key>data[pivot].key){
			end--;
		}
		if (start<end && data[start].key>data[end].key){
			swap(data, start, end);
		}
	}
	if (start==1){
		return 0;
	}else if (end ==size -1){
		return 0;
	}else{	
	swap(data,start-1, pivot);
		return start-1;
	}

/*	
	int pivot = 0;
	int store = size-1;
	for (int i = 0; i<store; ){
		if ((long long)data[i].key>(long long)data[pivot].key){
			swap(data, i, store);
			store--;
		}else
			i++;
	}	
	swap(data, pivot, store-1);
	return store-1;
	*/
}
	
int partition(dataType *data, int size, dataType *data2){
	if (size<2)
		return 0;
	int p = omp_get_num_procs();
	if (size<=p){
		return lpartition(data, size);
	}
	int *lessp;
	int *greaterp;
	lessp = (int*) malloc(p*sizeof(int));
	greaterp = (int*)malloc(p*sizeof(int));
	if( lessp == 0 || greaterp ==0){
		cout<<"OUT OF MEMORY."<<endl;
		exit(0);
	}
	int *lessp2;
	int *greaterp2;
	lessp2 = (int*) malloc(p*sizeof(int));
	greaterp2 = (int*)malloc(p*sizeof(int));
	if( lessp2 == 0 || greaterp2 ==0){
		cout<<"OUT OF MEMORY."<<endl;
		exit(0);
	}
	int pivot = 0;
	int chunk = ceil(((double)size)/p);
	p = ceil(((double)size)/chunk);

	cout<<"No. of processors: "<<p<<"\nsize:"<<size<<endl;
	#pragma omp parallel num_threads(p) 
	{
		int i = omp_get_thread_num();
		int start = chunk*i;
		int end = min((int)chunk*(i+1),size);
		//cout<<i<<" - "<<start<<" -  "<<end<<endl;
		/*
		int store = end-1;
		for (int j = start; j<=store;){
			if ((long long)data[j].key>(long long)data[pivot].key){
				swap(data, j, store);
				store--;
			}else{
				j++;
			}
		}
		*/
		int pivot = lpartition(data+start, end-start);	
		//cout<<"store:"<<store<<" pivot:"<<pivot<<endl;
		int store = start + pivot;
		greaterp[i]=end - store - 1;
		lessp[i] = store - start + 1;
	}
	psum(lessp,p,lessp2);
	psum(greaterp,p,greaterp2);
	free(lessp2);
	free(greaterp2);
/*
	for (int i =0; i<p; i++){
		cout<<lessp[i]<<", ";
	}
	cout<<endl;
	for (int i =0; i<p; i++){
		cout<<greaterp[i]<<", ";
	}
	cout<<endl<<endl;
		
	for (int i =0; i<size; i++){
		cout<<(long long)data[i].key<<", ";
	}
	cout<<endl<<endl;
*/	
	#pragma omp parallel num_threads(p)
	{
		int i = omp_get_thread_num();
		int start = chunk*i;
		int end = min(chunk*(i+1),size);
		int lp = (i>0? lessp[i-1]:0);
		int dp = lessp[i];
		
		for (int j = 0; j<dp-lp; j++){
			data2[lp+j] = data[start+j];
		}
		int gp = (i>0? greaterp[i-1]:0);
		int gd = greaterp[i];
		//cout<<i<<" - "<<lessp[p-1]<<" - "<<gp<<" - "<<gd-gp<<" - "<<dp<<" - "<<start+dp<<endl;
		for (int j =0; j <gd-gp; j++){
			data2[lessp[p-1]+gp+j] = data[start+dp-lp+j];
		}
	}
	
	swap(data2,0,lessp[p-1]-1);	
	
	#pragma omp parallel for num_threads(p)
	for (int i=0; i<size; i++){
		data[i] = data2[i];
	}
	return (lessp[p-1]>0?lessp[p-1]-1:0);
}

void pqsort(dataType *data, int start, int end, dataType *data2){
	if (start+1>=end){
		return;
	} else if (start+2==end){
		if ((long long)data[start].key> (long long)data[start+1].key){
			swap(data, start, start+1);
		}
	}
	else if (start+1<end){
		for(int i=0; i<end-start; i++){
			cout<<(long long)data[i].key<<", ";
		}
		cout<<" ("<<end-start<<")"<<endl;
		int store = lpartition(data+start, end-start);//, data2+start);
		for(int i=0; i<end-start; i++){
			cout<<(long long)data[i].key<<", ";
		}
		cout<<":"<<store<<":"<<endl<<endl;
		//cout<<start<<" "<<store<<" "<<end<<endl;
		//for(int i=0; i<end-start; i++){
		//	cout<<(long long)data[i].key<<", ";
		//}
		//cout<<endl;
			if (store<end)
			pqsort(data, start, store, data2);
			if (store+1>start)
			pqsort(data, store+1, end, data2);
	}
}

void pquicksort(dataType *data, int start, int end){
	
	dataType *data2;
	data2 = (dataType*) malloc((end-start)*sizeof(dataType));
	pqsort(data, start, end, data2);
}

int main(){
	int size = 20;
	/*
	int  *data;
	data = (int *) malloc(size*sizeof(int));
	int *data2;
	data2 = (int *) malloc(size*sizeof(int));
	data[0] = 7;
	data[1] = 13;
	data[2] = 18;
	data[3] = 2;
	data[4] = 17;
	data[5] = 1;
	data[6] = 14;
	data[7] = 20;
	data[8] = 6;
	data[9] = 7;
	data[10] = 10;
	data[12] = 15;
	data[13] = 9;
	data[14] = 3;
	data[15] = 16;
	data[16] = 19;
	data[17] = 4;
	data[18] = 11;
	data[19] = 12;
	*/
	dataType *data;
	data = (dataType*) malloc(size*sizeof(dataType));

	
	data[0].key = (long long*) 7;
	data[1].key = (long long*) 13;
	data[2].key = (long long*) 18;
	data[3].key = (long long*) 2;
	data[4].key = (long long*) 17;
	data[5].key = (long long*) 1;
	data[6].key = (long long*) 14;
	data[7].key = (long long*) 20;
	data[8].key = (long long*) 6;
	data[9].key = (long long*) 10;
	data[10].key = (long long*) 15;
	data[11].key = (long long*) 9;
	data[12].key = (long long*) 3;
	data[13].key = (long long*) 16;
	data[14].key = (long long*) 19;
	data[15].key = (long long*) 4;
	data[16].key = (long long*) 11;
	data[17].key = (long long*) 12;
	data[18].key = (long long*) 5;
	data[19].key = (long long*) 8;
	
	for (int i = 0; i<size; i++){
		cout<<(long long)data[i].key<<", ";
	}
	cout<<endl;

	pquicksort(data, 0, size);

	for (int i = 0; i<size; i++){
		cout<<(long long)data[i].key<<", ";
	}
	cout<<endl;
	return 0;
}

