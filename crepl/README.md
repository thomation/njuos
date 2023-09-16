* Create new file /tmp/crepl_lab.c
```
int sum(int  a, int  b)
{
        return a  + b;
}
```
* Compile dynamic lib
```
gcc crepl_lab.c -fPIC -shared -o libcrepl_lab.so
```
* Create new file /tmp/hello.c
```
#include <stdio.h>
extern int sum(int a, int b);
int main()
{
        int ret = sum(1, 2);
        printf("ret=%d\n", ret);
        return 0;
}
```
* Compile and link with dynamic lib
```
gcc hello.c -L. -lcrepl_lab
```
* set search path of dynamic lib
```
export LD_LIBRARY_PATH=/tmp
```
* Check if link is OK
```
ldd ./a.out
```
* Run it
```
./a.out
```
