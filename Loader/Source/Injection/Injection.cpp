#include "Injection.hpp"

using namespace Injection;

#define RELOC_FLAG(RelInfo)((RelInfo >> 12) == IMAGE_REL_BASED_DIR64)

void __stdcall Shellcode(ManualMappingData* Data);

bool Injection::ManualMap(HANDLE TargetProcess, const std::string& DllPath) {
    std::ifstream DllFile(DllPath, std::ios::binary | std::ios::ate);
    if (!DllFile) {
        Utils::Log(Utils::LogType::ERR, "Failed to open the DLL file: " + DllPath);
        return false;
    }

    auto FileSize = DllFile.tellg();
    if (FileSize < 0x1000) {
        Utils::Log(Utils::LogType::ERR, "Invalid DLL file size");
        return false;
    }

    std::vector < BYTE > DllData(static_cast <size_t> (FileSize));
    DllFile.seekg(0, std::ios::beg);
    DllFile.read(reinterpret_cast <char*> (DllData.data()), FileSize);
    DllFile.close();

    auto* DosHeader = reinterpret_cast <IMAGE_DOS_HEADER*> (DllData.data());
    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        Utils::Log(Utils::LogType::ERR, "Invalid DLL file!");
        return false;
    }

    auto* NtHeaders = reinterpret_cast <IMAGE_NT_HEADERS*> (DllData.data() + DosHeader->e_lfanew);
    auto* OptHeader = &NtHeaders->OptionalHeader;

    if (NtHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        Utils::Log(Utils::LogType::ERR, "DLL platform mismatch!");
        return false;
    }

    BYTE* RemoteBase = reinterpret_cast <BYTE*> (VirtualAllocEx(TargetProcess, nullptr, OptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!RemoteBase) {
        Utils::Log(Utils::LogType::ERR, "Failed to allocate memory in target process: " + std::to_string(GetLastError()));
        return false;
    }

    ManualMappingData MappingData{
      LoadLibraryA,
      GetProcAddress
    };

    auto* SectionHeader = IMAGE_FIRST_SECTION(NtHeaders);
    for (UINT i = 0; i < NtHeaders->FileHeader.NumberOfSections; ++i, ++SectionHeader) {
        if (SectionHeader->SizeOfRawData) {
            if (!WriteProcessMemory(TargetProcess, RemoteBase + SectionHeader->VirtualAddress, DllData.data() + SectionHeader->PointerToRawData, SectionHeader->SizeOfRawData, nullptr)) {
                Utils::Log(Utils::LogType::ERR, "Failed to write section data: " + std::to_string(GetLastError()));
                VirtualFreeEx(TargetProcess, RemoteBase, 0, MEM_RELEASE);
                return false;
            }
        }
    }

    memcpy(DllData.data(), &MappingData, sizeof(MappingData));
    if (!WriteProcessMemory(TargetProcess, RemoteBase, DllData.data(), 0x1000, nullptr)) {
        Utils::Log(Utils::LogType::ERR, "Failed to write headers and mapping data: " + std::to_string(GetLastError()));
        VirtualFreeEx(TargetProcess, RemoteBase, 0, MEM_RELEASE);
        return false;
    }

    auto RemoteShellcode = VirtualAllocEx(TargetProcess, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!RemoteShellcode) {
        Utils::Log(Utils::LogType::ERR, "Failed to allocate memory for shellcode: " + std::to_string(GetLastError()));
        VirtualFreeEx(TargetProcess, RemoteBase, 0, MEM_RELEASE);
        return false;
    }

    if (!WriteProcessMemory(TargetProcess, RemoteShellcode, Shellcode, 0x1000, nullptr)) {
        Utils::Log(Utils::LogType::ERR, "Failed to write shellcode: " + std::to_string(GetLastError()));
        VirtualFreeEx(TargetProcess, RemoteBase, 0, MEM_RELEASE);
        VirtualFreeEx(TargetProcess, RemoteShellcode, 0, MEM_RELEASE);
        return false;
    }

    auto RemoteThread = CreateRemoteThread(TargetProcess, nullptr, 0, reinterpret_cast <LPTHREAD_START_ROUTINE> (RemoteShellcode), RemoteBase, 0, nullptr);
    if (!RemoteThread) {
        Utils::Log(Utils::LogType::ERR, "Failed to create remote thread: " + std::to_string(GetLastError()));
        VirtualFreeEx(TargetProcess, RemoteBase, 0, MEM_RELEASE);
        VirtualFreeEx(TargetProcess, RemoteShellcode, 0, MEM_RELEASE);
        return false;
    }

    CloseHandle(RemoteThread);

    HINSTANCE CheckModule = nullptr;
    while (!CheckModule) {
        ManualMappingData Data{
          0
        };
        ReadProcessMemory(TargetProcess, RemoteBase, &Data, sizeof(Data), nullptr);
        CheckModule = Data.ModuleHandle;
        Sleep(10);
    }

    VirtualFreeEx(TargetProcess, RemoteShellcode, 0, MEM_RELEASE);

    return true;
    }

void __stdcall Shellcode(ManualMappingData * Data) {
    if (!Data) return;

    auto* Base = reinterpret_cast <BYTE*> (Data);
    auto* OptHeader = &reinterpret_cast <IMAGE_NT_HEADERS*> (Base + reinterpret_cast <IMAGE_DOS_HEADER*> (Data)->e_lfanew)->OptionalHeader;

    auto LoadLibraryA = Data->LoadLibraryA;
    auto GetProcAddress = Data->GetProcAddress;
    auto DllMain = reinterpret_cast <DllEntryPointType> (Base + OptHeader->AddressOfEntryPoint);

    BYTE* LocationDelta = Base - OptHeader->ImageBase;
    if (LocationDelta) {
        if (!OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) return;

        auto* RelocData = reinterpret_cast <IMAGE_BASE_RELOCATION*> (Base + OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        while (RelocData->VirtualAddress) {
            UINT EntriesCount = (RelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            auto* RelativeInfo = reinterpret_cast <WORD*> (RelocData + 1);

            for (UINT i = 0; i < EntriesCount; ++i, ++RelativeInfo) {
                if (RELOC_FLAG(*RelativeInfo)) {
                    auto* PatchAddr = reinterpret_cast <UINT_PTR*> (Base + RelocData->VirtualAddress + ((*RelativeInfo) & 0xFFF));
                    *PatchAddr += reinterpret_cast <UINT_PTR> (LocationDelta);
                }
            }

            RelocData = reinterpret_cast <IMAGE_BASE_RELOCATION*> (reinterpret_cast <BYTE*> (RelocData) + RelocData->SizeOfBlock);
        }
    }

    if (OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        auto* ImportDesc = reinterpret_cast <IMAGE_IMPORT_DESCRIPTOR*> (Base + OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        while (ImportDesc->Name) {
            char* ModuleName = reinterpret_cast <char*> (Base + ImportDesc->Name);
            HINSTANCE Dll = LoadLibraryA(ModuleName);

            ULONG_PTR* ThunkRef = reinterpret_cast <ULONG_PTR*> (Base + ImportDesc->OriginalFirstThunk);
            ULONG_PTR* FuncRef = reinterpret_cast <ULONG_PTR*> (Base + ImportDesc->FirstThunk);

            if (!ThunkRef) ThunkRef = FuncRef;

            for (; *ThunkRef; ++ThunkRef, ++FuncRef) {
                if (IMAGE_SNAP_BY_ORDINAL(*ThunkRef)) {
                    *FuncRef = reinterpret_cast <ULONG_PTR> (GetProcAddress(Dll, reinterpret_cast <char*> (*ThunkRef & 0xFFFF)));
                }
                else {
                    auto* Import = reinterpret_cast <IMAGE_IMPORT_BY_NAME*> (Base + (*ThunkRef));
                    *FuncRef = reinterpret_cast <ULONG_PTR> (GetProcAddress(Dll, Import->Name));
                }
            }
            ++ImportDesc;
        }
    }

    if (OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
        auto* TLS = reinterpret_cast <IMAGE_TLS_DIRECTORY*> (Base + OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        auto* Callback = reinterpret_cast <PIMAGE_TLS_CALLBACK*> (TLS->AddressOfCallBacks);
        for (; Callback && *Callback; ++Callback) {
            (*Callback)(Base, DLL_PROCESS_ATTACH, nullptr);
        }
    }

    DllMain(Base, DLL_PROCESS_ATTACH, nullptr);
    Data->ModuleHandle = reinterpret_cast <HINSTANCE> (Base);
}