#pragma once

#include <map>
#include <set>

#include <ares/network>

#include "predeclare.hpp"

#include "session.hpp"
#include "database/database.hpp"
#include "world/world.hpp"

namespace ares {
  namespace zone {
    struct server : ares::network::server<server> {
      server(std::shared_ptr<spdlog::logger> log,
             std::shared_ptr<boost::asio::io_service> io_service,
             const config& conf,
             const size_t num_threads);

      void start();
      void create_session(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

      void add(session_ptr s);
      void remove(session_ptr s);

      session_ptr client_by_aid(const uint32_t aid);

      const config& conf() const;
      database& db();
      const std::map<uint32_t, session_ptr>& clients() const;
      const session_ptr& char_server() const;

      ares::zone::world::state& world();
    private:
      const config& config_;
      database db_;

      std::set<session_ptr> mono_;
      std::map<uint32_t, session_ptr> clients_;
      session_ptr char_server_;

      ares::zone::world::state world_;

    };
  }
}
