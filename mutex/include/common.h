#include <stdbool.h>

#include <sys/cond.h>
#include <sys/mutex.h>
#include <sys/thread.h>

#include "rsxutil.h"
#include "pad.h"

#define MAX_BUFFERS 2

#define TIMEOUT_COND 0
#define TIMEOUT_MUTEX 0