#pragma once

namespace aztrixiania {
namespace base {
namespace dispatcher {

class FileDescriptor {
public:
    explicit FileDescriptor(const int fd); // Make implicit conversions "illegal"
    ~FileDescriptor();

    FileDescriptor(const FileDescriptor&) = delete; // Delete Copy constructor
    FileDescriptor(FileDescriptor&&) = delete; // Delete Move constructor

    operator int() const { return fd_; } // Make int conversions possible

private:
    int fd_;

};


} // dispatcher
} // base
} // aztrixiania
