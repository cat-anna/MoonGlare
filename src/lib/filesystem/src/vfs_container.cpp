

#include <svfs/vfs_container.h>

namespace MoonGlare::StarVfs {

iVfsContainer::iVfsContainer(iFileTableInterface *fti) : file_table_interface(fti) {}

// iContainer::~iContainer() {
// }

// bool iContainer::SetFileData(FileID ContainerFID, const ByteTable & in) const {
// 	return false;
// }

// bool iContainer::CreateFile(FileID ContainerBaseFID, ConstCString Name, FileID * GlobalFIDOutput) {
// 	return false;
// }

// bool iContainer::CreateDirectory(FileID ContainerBaseFID, ConstCString Name, FileID * GlobalFIDOutput) {
// 	return false;
// }

// ConstCString iContainer::GetFileName(FileID ContainerFID) const  {
// 	return nullptr;
// }

// bool iContainer::EnumerateFiles(ContainerFileEnumFunc filterFunc) const {
// 	return false;
// }

} // namespace MoonGlare::StarVfs
