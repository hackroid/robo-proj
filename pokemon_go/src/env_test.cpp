#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;

int main()
{
    char *env_val = getenv("CMAKE_PREFIX_PATH");
    char *pathx = strstr(env_val, "/");
    char *pathy = strstr(env_val, "/devel");
    int len = pathy - pathx + 1;
    char *fpath = (char*)malloc((len) * sizeof(char));
    memcpy(fpath, pathx, len - 1);
    fpath[len-1] = '\0';
    if (env_val != NULL)
        cout << "find = " << fpath << endl;
    else
        cout << "doesn't exist" << endl;
    string s(fpath);
    cout << s << endl;
    return 0;
}
