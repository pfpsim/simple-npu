//
// Created by kamil on 05/05/15.
//

#ifndef BEHAVIOURAL_COMMON_APPLICATIONREGISTRY_HPP
#define BEHAVIOURAL_COMMON_APPLICATIONREGISTRY_HPP

#include "Packet.h"
#include "PacketDescriptor.h"
#include <functional>
#include <map>
#include <string>

typedef std::function<Packet&(uint32_t&, PacketDescriptor&, Packet&, void*)>
    application_t;

inline std::map<std::string, application_t>::mapped_type& get_map_ref
    (const std::string &name) {
  static std::map<std::string, application_t> s_functions;
  return s_functions[name];
}

inline int register_application(const std::string &name, application_t app) {
    get_map_ref(name) = app;
    return 0;
}

#define call_application(app_name) get_map_ref(app_name)

#endif  // BEHAVIOURAL_COMMON_APPLICATIONREGISTRY_HPP
