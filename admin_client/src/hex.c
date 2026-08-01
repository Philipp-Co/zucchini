#include <zucchini/admin_client/hex.h>

#include <stdlib.h>
#include <errno.h>

int32_t ZUC_ParseHexUint64(const char *text, uint64_t *out_value)
{
    if(text == NULL || *text == '\0')
    {
        return -1;
    }

    errno = 0;
    char *endptr = NULL;
    const uint64_t value = strtoull(text, &endptr, 16);

    if(errno == ERANGE)
    {
        return -1;
    }
    if(endptr == text || *endptr != '\0')
    {
        return -1;
    }

    *out_value = value;
    return 0;
}
