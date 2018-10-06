ReadME
--------------------------------------------


Note: Run The codes using std=c++11

--------------------------------------------------------------------------
Compilation:

g++ -c -Wall -Werror -fpic libmymem.cpp -std=c++11 -pthread 

g++ -shared -o libmymem.so libmymem.o

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.  

g++ -I . -L . -Wall -o memutil memutil.cpp -l mymem -std=c++11 -lpthread



--------------------------------------------------------------------------

Execution:

./memutil -n 10 -k 4
