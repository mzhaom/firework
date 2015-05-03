#pragma once

#include <string>
#include <stdint.h>

namespace spark {

class Tun {
 public:
  // Allocate a tun device with an optional name(can be NULL). This
  // requires root permission.
  static Tun* Allocate(const char* name);

  int fd() const {
    return fd_;
  }

  const std::string& name() const {
    return name_;
  }

  // Setup the local ip and peer ip for
  // the tun device. Example of my_ip: "192.168.1.1" peer_ip:
  // "192.168.1.2"
  void Configure(const std::string& my_ip,
                 const std::string& peer_ip);

  /**
   * Add a route for that IP on the tun interface
   */
  bool AddRoute(const std::string& ip, uint8_t netmask);

  ~Tun();

 private:
  Tun(const char* name, int fd);

  /**
   * Bring up the tun interface and disable rp_filter.
   *  Return true IFF successful
   */
  bool Init();

  const std::string name_;
  const int fd_;
};

}
