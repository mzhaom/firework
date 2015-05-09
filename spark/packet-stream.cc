#include "spark/packet-stream.h"

#include <thread>
#include "glog/logging.h"

namespace spark {

PacketStream::PacketStream(Tun* tun, 
                           grpc::ReaderInterface<Bullet>* reader,
                           grpc::WriterInterface<Bullet>* writer)
    : tun_(tun),
      reader_(reader),
      writer_(writer),
      closing(false) {
}
      
PacketStream::~PacketStream() {
}

void PacketStream::DrainStream() {
  LOG(INFO) << "Drain stream";
  Bullet b;
  while (reader_->Read(&b)) {
    if (b.has_packet()) {
      const auto& p = b.packet().payload();
      if (write(tun_->fd(), p.data(), p.size()) <= 0) {
        break;
      }
    }
  }
  LOG(INFO) << "Stream is closed";
  closing_ = true;
}

void PacketStream::Run() {
  std::thread stream_read_thread([this]() {
      DrainTun();
    });
  DrainStream();
  stream_read_thread.join();
  LOG(INFO) << "Exit";
}

void PacketStream::DrainTun() {
  // Block the current thread to read from tun.
  const int kMTU = 1500;
  char buffer[kMTU];
  Bullet b;
  while (!closing_) {
    ssize_t len = read(tun_->fd(), buffer, kMTU);
    DLOG(INFO) << "Read " << len;
    if (len == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        PLOG(INFO) << "Failed to read from tun interface";
        break;
      }
    }
    if (len == 0) {
      break;
    }
    b.mutable_packet()->set_payload(buffer, len);
    if (!writer_->Write(b)) {
      LOG(INFO) << "Stream is closed";
      break;
    }
  }
  closing_ = true;
}

}
