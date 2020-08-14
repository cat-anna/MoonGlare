

#include <svfs/vfs_container.hpp>

namespace MoonGlare::StarVfs {

iVfsContainer::iVfsContainer(iFileTableInterface *fti) : file_table_interface(fti) {}

} // namespace MoonGlare::StarVfs
