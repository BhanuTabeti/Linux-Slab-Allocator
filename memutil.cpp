#include <bits/stdc++.h>
#include "libmymem.hpp"
using namespace std;

std::vector<std::thread> TH;

void do_join(std::thread& t)
{
    t.join();
}


void threadFun(int n){
	char *arr[n];
	int randVal[n];

	for (int i = 0; i < n; ++i)
	{
		randVal[i] = rand()%8193;
		
		if (randVal[i] == 0)
		{
			randVal[i]++;
		}
		arr[i] = (char *)mymalloc(randVal[i]);
	}

	

	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < randVal[i]; ++j)
		{
			arr[i][j] = 97 + j%26;
		}
	}

	

	for (int i = 0; i < n; ++i)
	{
		myfree(arr[i]);
	}

}

int main(int argc, char const *argv[])
{
	if (argc < 5)
	{
		cout<<"format : memutil -n 10 -k 10\n";
		return 1;
	}
	int n = atoi(argv[2]);
	int k = atoi(argv[4]);

	if (n <= 0 || n > 10000)
	{
		cout<<"n from 1 to 10000\n";
		return 1;
	}

	if (k <= 0 || k > 64)
	{
		cout<<"k from 1 to 64\n";
		return 1;
	}

	

	for (int i = 0; i < k; ++i)
	{
	
		TH.push_back(thread(threadFun,n));
	}
	
	std::for_each(TH.begin(),TH.end(),do_join);

	return 0;
}