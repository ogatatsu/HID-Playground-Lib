#pragma once

#include "FreeRTOS.h"
#include "task.h"

namespace hidpg::Internal
{

    inline uint32_t millis()
    {
        return xTaskGetTickCount() * 1000ULL / configTICK_RATE_HZ;
    }

} // namespace hidpg::Internal
