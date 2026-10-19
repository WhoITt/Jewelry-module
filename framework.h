#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Исключите редко используемые компоненты из заголовков Windows
#endif

#include "targetver.h"

#include <windows.h>

// The SDK headers export classes with STL members; MFC used to silence this warning.
#pragma warning(disable: 4251)

