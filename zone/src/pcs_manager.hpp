#pragma once

#include <ares/model>

#include "pc.hpp"
#include "session.hpp"

namespace ares {
  namespace zone {
    struct pcs_manager {
      pcs_manager(std::shared_ptr<spdlog::logger> log);

      void add(const uint32_t char_id, std::shared_ptr<zone::pc> pc, std::shared_ptr<session> s);
               
    private:
      // Char name index
      std::unordered_map<std::string, std::weak_ptr<session>> name_to_pc_;

      std::set<std::shared_ptr<zone::pc>> pcs_;

      std::map<uint32_t, std::weak_ptr<session>> id_to_session_;

      std::mutex mutex_;
      std::shared_ptr<spdlog::logger> log_;

    };
  }
}
