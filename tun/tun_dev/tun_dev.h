#ifndef __STITCH_TUN_DEV_H__
#define __STITCH_TUN_DEV_H__

#ifdef LINUX
#include "linux/tun.h"
#elif OSX
#include "osx/tun.h"
#endif //OS_LINUX or OS_OSX

#endif//__STITCH_TUN_DEV_H__
