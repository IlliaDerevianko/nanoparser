#include "cmsg_handlers.hpp"

#include <cstring>
#include <linux/errqueue.h>
#include "../time.hpp"


namespace nanoparser {
    bool TimestampExtractor::operator()(const cmsghdr* cmsg) {
        if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMPING) {
            scm_timestamping scm_ts{};
            memcpy(&scm_ts, CMSG_DATA(cmsg), sizeof(scm_ts));
            timestamp = time::timespec_to_ns(scm_ts.ts[2]);
            return true;
        }
        return false;
    }
}
