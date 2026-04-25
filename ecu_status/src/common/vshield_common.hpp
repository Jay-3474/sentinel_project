#pragma once
#include <vsomeip/vsomeip.hpp>

namespace sentinel {
    static constexpr vsomeip::service_t SERVICE_ID = 0x4000;
    static constexpr vsomeip::instance_t INSTANCE_ID = 0x0001;
    
    // Distinct Method IDs for better organization
    static constexpr vsomeip::method_t TELEMETRY_METHOD = 0x0011;
    static constexpr vsomeip::method_t STATUS_METHOD = 0x0012; 
    
    static constexpr vsomeip::eventgroup_t EVENTGROUP_ID = 0x0044;
    static constexpr vsomeip::event_t EMERGENCY_EVENT = 0x8005;
}
