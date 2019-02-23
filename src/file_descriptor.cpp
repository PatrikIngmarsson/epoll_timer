#include"file_descriptor.hpp"

#include<unistd.h>
#include<stdexcept>

namespace aztrixiania {
namespace base {
namespace dispatcher {

FileDescriptor::FileDescriptor(int fd) : fd_{fd} {
    if(fd < 0) {
        throw std::runtime_error("Filedescriptor is not valid.");
    }
}
FileDescriptor::~FileDescriptor() {
    if(fd_ > -1) { close(fd_); }
}


} // namespace dispatcher
} // namespace base
} // namespace aztrixiania
