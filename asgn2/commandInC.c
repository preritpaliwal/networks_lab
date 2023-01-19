#include <stdio.h>
#include <unistd.h>
int main()
{
    char cwd[256];

    if (chdir("..") != 0)
        perror("chdir() error()");
    else
    {
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            perror("getcwd() error");
        else
            printf("current working directory is: %s\n", cwd);
    }
}

// #include <stdio.h>
// #include <dirent.h>
// #include <string.h>

// int main(void)
// {
//     struct dirent *de; // Pointer for directory entry

//     // opendir() returns a pointer of DIR type.
//     DIR *dr = opendir(".");

//     if (dr == NULL) // opendir returns NULL if couldn't open directory
//     {
//         printf("Could not open current directory");
//         return 0;
//     }

//     // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
//     // for readdir()
//     while ((de = readdir(dr)) != NULL)
//         printf("%s%ld\t", de->d_name,strlen(de->d_name));

//     closedir(dr);
//     return 0;
// }