#pragma once

/* To cheese-ily avoid #error No Target Architecture from not including Windows.h before type definitions... */
#define _AMD64_
#include <windef.h>