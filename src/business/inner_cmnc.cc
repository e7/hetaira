#include "inner_cmnc.h"
#include "protocol.h"


using e7::common::proto_sjsonb;
using e7::common::smart_pointer;


namespace hetaira {
void inner_service(connection *conn)
{
    cJSON *req;

    if (-1 == proto_sjsonb::decode(&conn->rbuf, &req)) {
        smart_pointer<connection> auto_close_conn(conn);
        return;
    }

    cJSON *itfc = cJSON_GetObjectItem(req, "interface");
    fprintf(stderr, "%s\n", itfc->valuestring);

    cJSON_Delete(req);
    fprintf(stderr, "ok\n");

    return;
}
}
