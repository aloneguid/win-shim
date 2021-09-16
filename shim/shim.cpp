#include <iostream>
#include "windows.h"
#include "resource.h"
#include "../shmake/resources.h"

using namespace std;

#define MAX_LOADSTRING 1024

int main(int argc, char* argv)
{
    resources res;

    auto cmdline = res.load_string(IDS_TARGET_CMDLINE);

    wcout << "cmdline: " << cmdline << endl;

    return 0;
}