#include "IdService.h"
#include <pd_id.h>
#include <string.h>
#include <stdio.h>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// jenkins_one_at_a_time_hash

uint16_t IdService_register(const char* id)
{
    uint32_t hash, i;
    uint32_t len = (uint32_t)strlen(id);

    for (hash = i = 0; i < len; ++i) {
        hash += id[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    // Limit the range that we use for Ids
    return (hash & 0x7fff) + 0x1000;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDIdFuncs s_funcs[] = {
    { IdService_register },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

extern "C" {
void* get_id_service_1()
{
    return &prodbg::s_funcs;
}
}
