#ifndef HANDLE_H
#define HANDLE_H
namespace reactor {

class Handle {
 public:
  Handle() = default;
  Handle(int fd) : file_descriptor_(fd) {}
  int GetFileDescriptor() const { return file_descriptor_; }
  int SetFileDescriptor(int fd) {
    file_descriptor_ = fd;
    return fd;
  }

  // for map sorting
  bool operator<(const Handle& rhs) const {
    return this->GetFileDescriptor() > rhs.GetFileDescriptor();
  }

 private:
  int file_descriptor_;
};

}  // namespace reactor
#endif