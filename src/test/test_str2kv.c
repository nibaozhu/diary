#include <stdio.h>
#include <string.h>


struct __kv {
  char kv[10][2][1025];
};

void str2kv(const char *str, struct __kv *kv)
{
  int i = 0;

  char tmpstr[1025] = {};
  char tmpkv[10][1025] = {};

  strcpy(tmpstr, str);
  printf("tmpstr = '%s'\n", tmpstr);

  char *saveptr = NULL;
  char *token = strtok_r(tmpstr, "&", &saveptr);
  do {
    strcpy(tmpkv[i], token);
    printf("tmpkv[%d] = '%s'\n", i, tmpkv[i]);

    i++;
    token = strtok_r(NULL, "&", &saveptr);
  } while (token);


  for (i = 0; i < 1024; i++)
  {
    if (strcmp(tmpkv[i], "") == 0)
      break;
    char *pos_and = index(tmpkv[i], '=');
    if (pos_and == NULL)
    {
      strcpy(kv->kv[i][0], tmpkv[i]);
    }
    else
    {
      strncpy(kv->kv[i][0], tmpkv[i], pos_and - tmpkv[i]);
      strncpy(kv->kv[i][1], pos_and + 1, strlen(tmpkv[i]) - (pos_and - tmpkv[i] + 1));
    }
  }

  printf("kv = %p\n", kv);
}

int main(int argc, char **argv)
{
  char url[] = "id=1=====23&name=jack&gender=&male&xxxx=ttttttttt&&&=9999";

  struct __kv kv;
  memset(&kv, 0, sizeof kv);

  str2kv(url, &kv);

  return 0;
}
