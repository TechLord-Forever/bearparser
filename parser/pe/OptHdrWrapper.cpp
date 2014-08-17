#include "OptHdrWrapper.h"
#include "PEFile.h"

using namespace std;

std::map<DWORD, QString> OptHdrWrapper::s_optMagic;
std::map<std::pair<WORD,WORD>, QString> OptHdrWrapper::s_osVersion;
std::map<DWORD, QString> OptHdrWrapper::s_dllCharact;
std::map<DWORD, QString> OptHdrWrapper::s_subsystem;

void OptHdrWrapper::initDllCharact()
{
    if (s_dllCharact.size() == 0) {
        s_dllCharact[IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE] = "DLL can move";
        s_dllCharact[IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY] = "Code Integrity Image";
        s_dllCharact[IMAGE_DLLCHARACTERISTICS_NX_COMPAT] = "Image is NX compatible";
        s_dllCharact[IMAGE_DLLCHARACTERISTICS_NO_ISOLATION] = "Image understands isolation and doesn't want it";
        s_dllCharact[IMAGE_DLLCHARACTERISTICS_NO_SEH] = "Image does not use SEH";
        s_dllCharact[IMAGE_DLLCHARACTERISTICS_NO_BIND] = "Do not bind this image";
        s_dllCharact[IMAGE_DLLCHARACTERISTICS_WDM_DRIVER] = "Driver uses WDM model";
        s_dllCharact[IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE] = "TerminalServer aware";
    }
}

QString OptHdrWrapper::translateDllCharacteristics(DWORD charact)
{
    if (s_dllCharact.size() == 0) initDllCharact();

    if (s_dllCharact.find(charact) == s_dllCharact.end()) return "";
    return s_dllCharact[charact];
}

std::vector<DWORD> OptHdrWrapper::splitDllCharact(DWORD characteristics)
{
    if (s_dllCharact.size() == 0) initDllCharact();

    std::vector<DWORD> chSet;
    map<DWORD, QString>::iterator iter;
    for (iter = s_dllCharact.begin(); iter != s_dllCharact.end(); iter++) {
        if (characteristics & iter->first) {
            chSet.push_back(iter->first);
        }
    }
    return chSet;
}

QString OptHdrWrapper::translateOptMagic(DWORD p)
{
    if (s_optMagic.size() == 0) {
        s_optMagic[IMAGE_NT_OPTIONAL_HDR32_MAGIC] = "NT32";
        s_optMagic[IMAGE_NT_OPTIONAL_HDR64_MAGIC] = "NT64";
        s_optMagic[IMAGE_ROM_OPTIONAL_HDR_MAGIC] = "ROM";
    }
    if (s_optMagic.find(p) == s_optMagic.end()) return "";
    return s_optMagic[p];
}

QString OptHdrWrapper::translateOSVersion(WORD major, WORD minor)
{
    if (s_osVersion.size() == 0) {
        s_osVersion[pair<WORD,WORD>(8, 0)] = "Windows 8";
        s_osVersion[pair<WORD,WORD>(7, 0)] = "Windows 7";
        s_osVersion[pair<WORD,WORD>(6, 0)] = "Windows Vista / Server 2008";

        s_osVersion[pair<WORD,WORD>(5, 2)] = "Windows Server 2003";
        s_osVersion[pair<WORD,WORD>(5, 1)] = "Windows XP";
        s_osVersion[pair<WORD,WORD>(5, 0)] = "Windows 2000 / XP";

        s_osVersion[pair<WORD,WORD>(4, 90)] = "Windows ME";
        s_osVersion[pair<WORD,WORD>(4, 10)] = "Windows 98";
        s_osVersion[pair<WORD,WORD>(4, 0)] = "Windows 95 / NT 4.0";

        s_osVersion[pair<WORD,WORD>(3, 51)] = "Windows NT 4.51";
        s_osVersion[pair<WORD,WORD>(3, 10)] = "Windows NT 3.1";
    }
    pair<WORD,WORD> p(major, minor);
    if (s_osVersion.find(p) == s_osVersion.end()) return "";
    return s_osVersion[p];
}

QString OptHdrWrapper::translateSubsystem(DWORD subsystem)
{
    if (s_subsystem.size() == 0) {
        s_subsystem[IMAGE_SUBSYSTEM_UNKNOWN] = "Unknown subsystem";
        s_subsystem[IMAGE_SUBSYSTEM_NATIVE] = "Driver";
        s_subsystem[IMAGE_SUBSYSTEM_WINDOWS_GUI] = "Windows GUI";
        s_subsystem[IMAGE_SUBSYSTEM_WINDOWS_CUI] = "Windows console";
        s_subsystem[IMAGE_SUBSYSTEM_OS2_CUI] = "OS/2 console";
        s_subsystem[IMAGE_SUBSYSTEM_POSIX_CUI] = "Posix console";
        s_subsystem[IMAGE_SUBSYSTEM_NATIVE_WINDOWS] = "Native Win9x driver";
        s_subsystem[IMAGE_SUBSYSTEM_WINDOWS_CE_GUI] = "Windows CE subsystem";
        s_subsystem[IMAGE_SUBSYSTEM_EFI_APPLICATION] = "EFI_APPLICATION";
        s_subsystem[IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER] = "EFI_BOOT_SERVICE_DRIVER";
        s_subsystem[IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER] = "EFI_RUNTIME_DRIVER";
        s_subsystem[IMAGE_SUBSYSTEM_EFI_ROM] = "EFI_ROM";
        s_subsystem[IMAGE_SUBSYSTEM_XBOX] = "XBOX";
        s_subsystem[IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION] = "WINDOWS_BOOT_APPLICATION";
    }
    if (s_subsystem.find(subsystem) == s_subsystem.end()) return "";
    return s_subsystem[subsystem];
}
//-------------------------------------------
bool OptHdrWrapper::wrap()
{
    opt32 = NULL;
    opt64 = NULL;
    getPtr();
    bool isOk = false;
    DWORD charact = static_cast<DWORD>(this->getNumValue(DLL_CHARACT, &isOk));
    this->dllCharact.clear();
    if (isOk) {
        this->dllCharact = this->splitDllCharact(charact);
    }
    return true;
}

Executable::exe_bits OptHdrWrapper::getHdrBitMode()
{
    PEFile* m_ExeFile = dynamic_cast<PEFile*> (this->m_Exe);
    if (m_ExeFile == NULL) {
        return Executable::BITS_32; // default
    }
    return m_ExeFile->getHdrBitMode();
}

pe::IMAGE_NT_HEADERS32* OptHdrWrapper::nt32()
{
    if (getHdrBitMode() != Executable::BITS_32) return NULL;

    PEFile *pe = dynamic_cast<PEFile*> (m_Exe);
    if (pe == NULL) return NULL;

    offset_t myOff = pe->peNtHdrOffset();
    IMAGE_NT_HEADERS32* hdr = (IMAGE_NT_HEADERS32*) m_Exe->getContentAt(myOff, sizeof(IMAGE_NT_HEADERS32));
    return hdr;
}

pe::IMAGE_NT_HEADERS64* OptHdrWrapper::nt64()
{
    if (getHdrBitMode() != Executable::BITS_64) return NULL;

    PEFile *pe = dynamic_cast<PEFile*> (m_Exe);
    if (pe == NULL) return NULL;

    offset_t myOff = pe->peNtHdrOffset();
    IMAGE_NT_HEADERS64* hdr = (IMAGE_NT_HEADERS64*) m_Exe->getContentAt(myOff, sizeof(IMAGE_NT_HEADERS64));
    return hdr;
}


void* OptHdrWrapper::getPtr()
{
    if (opt32 == NULL && opt64 == NULL) {
        pe::IMAGE_NT_HEADERS32* ntHdr32 = nt32();
        if (ntHdr32 != NULL) {
            this->opt32 = ntHdr32 ? &(ntHdr32->OptionalHeader) : NULL;
        } else {
            pe::IMAGE_NT_HEADERS64* ntHdr64 = nt64();
            this->opt64 = (ntHdr64) ? &(ntHdr64->OptionalHeader) : NULL;
        }
    }
    void *ptr = (opt32) ? (void*) opt32 : (void*) opt64;
    return ptr;
}

bufsize_t OptHdrWrapper::getSize()
{
    IMAGE_OPTIONAL_HEADER32* opt32 = (nt32()) ? &(nt32()->OptionalHeader) : NULL;
    IMAGE_OPTIONAL_HEADER64* opt64 = (nt64()) ? &(nt64()->OptionalHeader) : NULL;
    if (opt32 == NULL && opt64 == NULL) return 0;

    bufsize_t size = opt32 ? sizeof(IMAGE_OPTIONAL_HEADER32) : sizeof(IMAGE_OPTIONAL_HEADER64);
    return size;
}

bufsize_t OptHdrWrapper::getFieldSize(size_t fId, size_t subField)
{
    IMAGE_OPTIONAL_HEADER64* opt64 = (nt64()) ? &(nt64()->OptionalHeader) : NULL;
    if (opt64 != NULL) {
        if (fId == CODE_BASE) return sizeof (opt64->BaseOfCode);
        if (fId == DATA_BASE) return 0;
    }
    return ExeElementWrapper::getFieldSize(fId, subField);
}

void* OptHdrWrapper::getFieldPtr(size_t fId, size_t subField)
{
    IMAGE_OPTIONAL_HEADER32* opt32 = (nt32()) ? &(nt32()->OptionalHeader) : NULL;
    IMAGE_OPTIONAL_HEADER64* opt64 = (nt64()) ? &(nt64()->OptionalHeader) : NULL;

    switch (fId) {
        case MAGIC :
            return opt32 ? (void*)&opt32->Magic : (void*)&opt64->Magic;

        case LINKER_MAJOR :
            return opt32 ? (void*)&opt32->MajorLinkerVersion : (void*)&opt64->MajorLinkerVersion;
        case LINKER_MINOR :
            return opt32 ? (void*)&opt32->MinorLinkerVersion : (void*)&opt64->MinorLinkerVersion;

        case CODE_SIZE :
            return opt32 ? (void*)&opt32->SizeOfCode : (void*)&opt64->SizeOfCode;
        case INITDATA_SIZE :
            return opt32 ? (void*)&opt32->SizeOfInitializedData : (void*)&opt64->SizeOfInitializedData;
        case UNINITDATA_SIZE :
            return opt32 ? (void*)&opt32->SizeOfUninitializedData : (void*)&opt64->SizeOfUninitializedData;
        case EP :
            return opt32 ? &opt32->AddressOfEntryPoint : (void*)&opt64->AddressOfEntryPoint;
        case CODE_BASE :
            return opt32 ? (void*)&opt32->BaseOfCode : (void*)&opt64->BaseOfCode;

        case DATA_BASE :
            return opt32 ? (void*)&opt32->BaseOfData : NULL;

        case IMAGE_BASE :
            return opt32 ? (void*)&opt32->ImageBase : (void*)&opt64->ImageBase;
        case SEC_ALIGN :
            return opt32 ? (void*)&opt32->SectionAlignment : (void*)&opt64->SectionAlignment;
        case FILE_ALIGN :
            return opt32 ? (void*)&opt32->FileAlignment : (void*)&opt64->FileAlignment;

        case OSVER_MAJOR :
            return opt32 ? (void*)&opt32->MajorOperatingSystemVersion : (void*)&opt64->MajorOperatingSystemVersion;
        case OSVER_MINOR :
            return opt32 ? (void*)&opt32->MinorOperatingSystemVersion : (void*)&opt64->MinorOperatingSystemVersion;

        case IMGVER_MAJOR :
            return opt32 ? (void*)&opt32->MajorImageVersion : (void*)&opt64->MajorImageVersion;
        case IMGVER_MINOR:
            return opt32 ? (void*)&opt32->MinorImageVersion : (void*)&opt64->MinorImageVersion;

        case SUBSYSVER_MAJOR :
            return opt32 ? (void*)&opt32->MajorSubsystemVersion : (void*)&opt64->MajorSubsystemVersion;
        case SUBSYSVER_MINOR:
            return opt32 ? (void*)&opt32->MinorSubsystemVersion : (void*)&opt64->MinorSubsystemVersion;

        case WIN32_VER :
            return opt32 ? (void*)&opt32->Win32VersionValue : (void*)&opt64->Win32VersionValue;
        case IMAGE_SIZE :
            return opt32 ? (void*)&opt32->SizeOfImage : (void*)&opt64->SizeOfImage;
        case HDRS_SIZE:
            return opt32 ? (void*)&opt32->SizeOfHeaders : (void*)&opt64->SizeOfHeaders;
        case CHECKSUM:
            return opt32 ? (void*)&opt32->CheckSum : (void*)&opt64->CheckSum;
        case SUBSYS:
            return opt32 ? (void*)&opt32->Subsystem : (void*)&opt64->Subsystem;
        case DLL_CHARACT :
            return opt32 ? (void*)&opt32->DllCharacteristics : (void*)&opt64->DllCharacteristics;
        case STACK_RSRV_SIZE :
            return opt32 ? (void*)&opt32->SizeOfStackReserve : (void*)&opt64->SizeOfStackReserve;
        case STACK_COMMIT_SIZE :
            return opt32 ? (void*)&opt32->SizeOfStackCommit : (void*)&opt64->SizeOfStackCommit;
        case HEAP_RSRV_SIZE:
            return opt32 ? (void*)&opt32->SizeOfHeapReserve : (void*)&opt64->SizeOfHeapReserve;
        case HEAP_COMMIT_SIZE :
            return opt32 ? (void*)&opt32->SizeOfHeapCommit : (void*)&opt64->SizeOfHeapCommit;
        case LDR_FLAGS :
            return opt32 ? (void*)&opt32->LoaderFlags : (void*)&opt64->LoaderFlags;
        case RVAS_SIZES_NUM:
            return opt32 ? (void*)&opt32->NumberOfRvaAndSizes : (void*)&opt64->NumberOfRvaAndSizes;
        case DATA_DIR:
            return opt32 ? (void*)&opt32->DataDirectory : (void*)&opt64->DataDirectory;
    }
    return this->getPtr();
}

QString OptHdrWrapper::translateFieldContent(size_t fieldId)
{
    bool isOk = false;
    uint32_t num = -1;

    switch (fieldId) {
        case MAGIC :
            num = static_cast<uint32_t>(this->getNumValue(fieldId, &isOk));
            if (!isOk) return "";
            return this->translateOptMagic(num);
        case SUBSYS :
            num = static_cast<uint32_t>(this->getNumValue(fieldId, &isOk));
            if (!isOk) return "";
            return this->translateSubsystem(num);
    }
    return "";
}

QString OptHdrWrapper::getFieldName(size_t fieldId)
{
    switch (fieldId) {
        case MAGIC: return ("Magic");
        case LINKER_MAJOR: return ("Linker Ver. (Major)");
        case LINKER_MINOR: return ("Linker Ver. (Minor)");

        case CODE_SIZE: return ("Size of Code");
        case INITDATA_SIZE: return ("Size of Initialized Data");
        case UNINITDATA_SIZE: return ("Size of Uninitialized Data");
        case EP: return ("Entry Point");
        case CODE_BASE: return ("Base of Code");
        case DATA_BASE:
        {
            if (getHdrBitMode() == Executable::BITS_32) return ("Base of Data");
            return "";
        }
        case IMAGE_BASE: return ("Image Base");
        case SEC_ALIGN: return ("Section Alignment");
        case FILE_ALIGN: return ("File Alignment");

        case OSVER_MAJOR: return ("OS Ver. (Major)");
        case OSVER_MINOR: return ("OS Ver. (Minor)");

        case IMGVER_MAJOR: return ("Image Ver. (Major)");
        case IMGVER_MINOR: return ("Image Ver. (Minor)");

        case SUBSYSVER_MAJOR: return ("Subsystem Ver. (Major)");
        case SUBSYSVER_MINOR: return ("Subsystem Ver. Minor)");

        case WIN32_VER: return ("Win32 Version Value");
        case IMAGE_SIZE: return ("Size of Image");
        case HDRS_SIZE: return ("Size of Headers");
        case CHECKSUM: return ("Checksum");
        case SUBSYS: return ("Subsystem");

        case DLL_CHARACT: return ("Dll Characteristics");

        case STACK_RSRV_SIZE: return ("Size of Stack Reserve");
        case STACK_COMMIT_SIZE: return ("Size of Stack Commit");
        case HEAP_RSRV_SIZE: return ("Size of Heap Reserve");
        case HEAP_COMMIT_SIZE: return ("Size of Heap Commit");
        case LDR_FLAGS: return ("Loader Flags");
        case RVAS_SIZES_NUM: return ("Number of RVAs and Sizes");
        case DATA_DIR: return "Data Dir.";
    }
    return "";
}

Executable::addr_type OptHdrWrapper::containsAddrType(size_t fieldId, size_t subField)
{
    switch (fieldId) {
        case EP:
        case CODE_BASE:
        case DATA_BASE:
            return Executable::RVA;
    }
    return Executable::NOT_ADDR;
}

//-----------------------

void* DataDirWrapper::getPtr()
{
    PEFile *pe = dynamic_cast<PEFile*> (m_Exe);
    if (pe == NULL) return NULL;

    offset_t myOff = pe->peDataDirOffset();
    pe::IMAGE_DATA_DIRECTORY* ptr = (pe::IMAGE_DATA_DIRECTORY*) m_Exe->getContentAt(myOff, getSize());
    return ptr;
}

bufsize_t DataDirWrapper::getSize()
{
    size_t size = sizeof(pe::IMAGE_DATA_DIRECTORY) * DIRECTORY_ENTRIES_NUM;
    return (uint32_t) size;
}

void* DataDirWrapper::getFieldPtr(size_t fieldId, size_t subField)
{
    if (fieldId >= DIRECTORY_ENTRIES_NUM) {
        return getPtr(); // invalid fieldID!
    }

    IMAGE_DATA_DIRECTORY* dataDir = (IMAGE_DATA_DIRECTORY*) getPtr();
    if (dataDir == NULL) return NULL;

    switch (subField) {
        case ADDRESS :
            return (void*)(&dataDir[fieldId].VirtualAddress);
        case SIZE :
            return (void*)(&dataDir[fieldId].Size);
    }
    return (void*)(&dataDir[fieldId].VirtualAddress);
}

bufsize_t DataDirWrapper::getFieldSize(size_t fieldId, size_t subField)
{
    if (fieldId >= DIRECTORY_ENTRIES_NUM) return getSize();

    pe::IMAGE_DATA_DIRECTORY* dir = (IMAGE_DATA_DIRECTORY*) getPtr();
    if (dir == NULL) return 0;

    pe::IMAGE_DATA_DIRECTORY record = dir[fieldId];

    switch (subField) {
        case ADDRESS :
            return sizeof(record.VirtualAddress);
        case SIZE :
            return sizeof(record.Size);
    }
    return sizeof(record.VirtualAddress) + sizeof(record.Size);
}

QString DataDirWrapper::getFieldName(size_t fieldId)
{
    switch (fieldId) {
        case DIR_EXPORT: return "Export Directory";
        case DIR_IMPORT: return "Import Directory";
        case DIR_RESOURCE : return "Resource Directory";
        case DIR_EXCEPTION: return "Exception Directory";
        case DIR_SECURITY: return "Security Directory";
        case DIR_BASERELOC: return "Base Relocation Table";
        case DIR_DEBUG : return "Debug Directory";
        case DIR_ARCHITECTURE: return "Architecture Specific Data";
        case DIR_GLOBALPTR : return "RVA of GlobalPtr";
        case DIR_TLS: return "TLS Directory";
        case DIR_LOAD_CONFIG: return "Load Configuration Directory";
        case DIR_BOUND_IMPORT: return "Bound Import Directory in headers";
        case DIR_IAT: return "Import Address Table";
        case DIR_DELAY_IMPORT: return "Delay Load Import Descriptors";
        case DIR_COM_DESCRIPTOR : return "COM Runtime descriptor";
    }
    return "";
}

Executable::addr_type DataDirWrapper::containsAddrType(size_t fieldId, size_t subField)
{
    if (subField != ADDRESS) return Executable::NOT_ADDR;

    if (fieldId == DIR_SECURITY) return Executable::RAW;
    return Executable::RVA;
}
