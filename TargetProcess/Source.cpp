
/*#include <Windows.h>
#include <winerror.h>

#include <stdio.h>

int main()
{
    int i = 0;
    while (true)
    {
        printf("Processing - %d\n", i++);
        Sleep(1000);
    }
    return 0;
}*/

#include <Windows.h>
#include <winerror.h>
#include <stdio.h>

int main()
{
    DWORD pid = GetCurrentProcessId();  //пид текущего
    printf("pid = %d\n", pid);
    int i = 0;
    while (true)        //цикл
    {
        printf("Processing - %d\n", i++);
        Sleep(1000);
    }
    return 0;
}