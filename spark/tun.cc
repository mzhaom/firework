#include "spark/tun.h"

#include <vector>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "folly/Subprocess.h"
#include "glog/logging.h"

using folly::Subprocess;
namespace spark {

static const char* kDevTun = "/dev/net/tun";

Tun* Tun::Allocate(const char* name) {
  /* open the clone device */
  int fd = open(kDevTun, O_RDWR);
  if (fd <= 0) {
    // It may happen when tun is unsupported or running inside docker
    // container.
    PLOG(INFO) << "Failed to open tun clone device";
    return nullptr;
  }

  /* preparation of the struct ifr, of type "struct ifreq" */
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  /** If flag IFF_NO_PI is not set each frame format is:
   * Flags [2 bytes]
   * Proto [2 bytes]
   * Raw protocol(IP, IPv6, etc) frame.
   */
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  if (name) {
    /* if a device name was specified, put it in the structure;
     * otherwise, the kernel will try to allocate the "next" device of
     * the specified type */
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
  }

  /* try to create the device */
  int err = ioctl(fd, TUNSETIFF, &ifr);
  if (err < 0) {
    PLOG(INFO) << "ioctl tun failed.";
    ::close(fd);
    return nullptr;
  }

  Tun* device = new Tun(ifr.ifr_name, fd);
  if (!device->Init()) {
    LOG(ERROR) << "Tun device failed to initialize, deleting";
    delete device;
    return nullptr;
  }
  return device;
}

Tun::Tun(const char* name, int fd) : name_(name), fd_(fd) { }

bool Tun::Init() {
  Subprocess proc(std::vector<std::string>({
        "/sbin/ip",  "link",  "set", name_, "up"}));
  if (proc.wait().exitStatus() != 0) {
    LOG(ERROR) << "Failed to bring up tun interface";
    return false;
  }
  return true;
}

Tun::~Tun() {
  ::close(fd_);
}

void Tun::Configure(const std::string& my_ip,
                    const std::string& peer_ip) {
  Subprocess proc(std::vector<std::string>({
        "/sbin/ip", "addr", "add", "dev", name_,
"local", my_ip, "peer", peer_ip}));
  CHECK_EQ(0, proc.wait().exitStatus()) << "Failed to configure IP for tun device";
}

}  // namespace spark
