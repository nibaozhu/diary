// uuid.h, libuuid.a, libuuid.so.x.x.x
// $ cc -o test_uuid test_uuid.c /path/to/libuuid.a
// $ cc -o test_uuid test_uuid.c /path/to/libuuid.so.x.x.x

#include <stdio.h>
#include <stdlib.h>

#include "uuid.h"

int main()
{

  int i;
  int n = 1024 * 1024;
  for (i = 0; i < n; i++)
  {

  uuid_t *_uuid = NULL;
  uuid_rc_t rc = uuid_create(&_uuid);
  if (rc != UUID_RC_OK) return 0;

  const char *_name = "nil";
  rc = uuid_load( _uuid, _name);
  if (rc != UUID_RC_OK) return 0;

    unsigned int _mode = UUID_MAKE_V1;
    rc = uuid_make(_uuid, _mode);
    if (rc != UUID_RC_OK) return 0;

    uuid_fmt_t _fmt = UUID_FMT_STR;
    void *_data_ptr = NULL;
    size_t _data_len = 0;
    rc = uuid_export(_uuid, _fmt, &_data_ptr, &_data_len);
    if (rc != UUID_RC_OK) return 0;

    fprintf(stdout, "%s\n", _data_ptr);
    free(_data_ptr);
    _data_ptr = NULL;

  rc = uuid_destroy(_uuid);
  if (rc != UUID_RC_OK) return 0;
  }

  return 0;
}

