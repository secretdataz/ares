#include "session.hpp"

#include <utility>

#include "server.hpp"
#include "mono/state.hpp"

ares::character::session::session(character::server& serv,
                                  const std::optional<asio::ip::tcp::endpoint> connect_ep,
                                  std::shared_ptr<asio::ip::tcp::socket> socket,
                                  const std::chrono::seconds idle_timer_timeout) :
  ares::network::session<session, character::server>(serv, connect_ep, socket, idle_timer_timeout),
  session_state_(std::in_place_type<mono::state>, serv, *this) {
}

auto ares::character::session::variant() -> state_variant& {
  return session_state_;
}

bool ares::character::session::is_mono() const {
  return std::holds_alternative<mono::state>(session_state_);
}

auto ares::character::session::as_mono() -> mono::state& {
  return std::get<mono::state>(session_state_);
}

bool ares::character::session::is_client() const {
  return std::holds_alternative<client::state>(session_state_);
}

auto ares::character::session::as_client() -> client::state& {
  return std::get<client::state>(session_state_);
}

bool ares::character::session::is_zone_server() const {
  return std::holds_alternative<zone_server::state>(session_state_);
}

auto ares::character::session::as_zone_server() -> zone_server::state& {
  return std::get<zone_server::state>(session_state_);
}

bool ares::character::session::is_account_server() const {
  return std::holds_alternative<account_server::state>(session_state_);
}

auto ares::character::session::as_account_server() -> account_server::state& {
  return std::get<account_server::state>(session_state_);
}

#define ARES_VARIANT_EVENT_DISPATCHER(NAME)             \
  void ares::character::session::NAME() {               \
    struct visitor {                                    \
      visitor(session& s) :                             \
        s(s) {};                                        \
                                                        \
      void operator()(const mono::state&) {             \
        s.as_mono().NAME();                             \
      }                                                 \
                                                        \
      void operator()(const client::state&) {           \
        s.as_client().NAME();                           \
      }                                                 \
                                                        \
      void operator()(const account_server::state&) {   \
        s.as_account_server().NAME();                   \
      }                                                 \
                                                        \
      void operator()(const zone_server::state&) {      \
        s.as_zone_server().NAME();                      \
      }                                                 \
    private:                                            \
    session& s;                                         \
    };                                                  \
    std::visit(visitor(*this), variant());              \
  }                                                     \
  

ARES_VARIANT_EVENT_DISPATCHER(on_connect);
ARES_VARIANT_EVENT_DISPATCHER(on_connection_reset);
ARES_VARIANT_EVENT_DISPATCHER(on_operation_aborted);
ARES_VARIANT_EVENT_DISPATCHER(on_eof);
ARES_VARIANT_EVENT_DISPATCHER(on_socket_error);
ARES_VARIANT_EVENT_DISPATCHER(on_packet_processed);
ARES_VARIANT_EVENT_DISPATCHER(defuse_asio);

#undef ARES_VARIANT_EVENT_DISPATCHER

auto ares::character::session::allocate(const uint16_t packet_id) -> packet::alloc_info {
  struct visitor {
    visitor(session& s, const uint16_t packet_id) :
      s(s), packet_id(packet_id) {};

    packet::alloc_info operator()(const mono::state&) {
      return s.as_mono().allocate(packet_id);
    }

    packet::alloc_info operator()(const zone_server::state&) {
      return s.as_zone_server().allocate(packet_id);
    }
    
    packet::alloc_info operator()(const account_server::state&) {
      return s.as_account_server().allocate(packet_id);
    }
    
    packet::alloc_info operator()(const client::state&) {
      return s.as_client().allocate(packet_id);
    }

  private:
    session& s;
    const uint16_t packet_id;
  };

  return std::visit(visitor(*this, packet_id), variant());
}

void ares::character::session::dispatch_packet(const uint16_t packet_id, void* buf, std::function<void(void*)> deallocator) {
  struct visitor {
    visitor(session& s, const uint16_t packet_id, void* buf, std::function<void(void*)> deallocator) :
      s(s), packet_id(packet_id), buf(buf), deallocator(deallocator) {};

    void operator()(const mono::state&) {
      s.as_mono().dispatch_packet(packet_id, buf, deallocator);
    }

    void operator()(const zone_server::state&) {
      s.as_zone_server().dispatch_packet(packet_id, buf, deallocator);
    }
    
    void operator()(const account_server::state&) {
      s.as_account_server().dispatch_packet(packet_id, buf, deallocator);
    }
    
    void operator()(const client::state&) {
      s.as_client().dispatch_packet(packet_id, buf, deallocator);
    }

  private:
    session& s;
    const uint16_t packet_id;
    void* buf;
    std::function<void(void*)> deallocator;
  };

  return std::visit(visitor(*this, packet_id, buf, deallocator), variant());
}
