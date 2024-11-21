#include "Injection.hpp"

#define RELOC_FLAG(RelInfo) ((RelInfo >> 12) == IMAGE_REL_BASED_DIR64)

using NtCreateThreadEx_t = NTSTATUS(WINAPI*)(PHANDLE, ACCESS_MASK, PVOID, HANDLE, PVOID, PVOID, ULONG, SIZE_T, SIZE_T, SIZE_T, PVOID);

bool CInjector::ManualMap(HANDLE Process, const std::string& DllPath) {
    std::ifstream DllFile(DllPath, std::ios::binary | std::ios::ate);
    if (!DllFile.is_open()) return false;

    auto FileSize = DllFile.tellg();
    if (FileSize < 0x1000) return false;

    std::vector<BYTE> DllData(static_cast<size_t>(FileSize));
    DllFile.seekg(0, std::ios::beg);
    DllFile.read(reinterpret_cast<char*>(DllData.data()), FileSize);
    DllFile.close();

    auto* DosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(DllData.data());
    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

    auto* NtHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(DllData.data() + DosHeader->e_lfanew);
    auto* OptHeader = &NtHeaders->OptionalHeader;

    if (NtHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) return false;

    BYTE* RemoteBase = reinterpret_cast<BYTE*>(VirtualAllocEx(Process, nullptr, OptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!RemoteBase) return false;

    MappingData MData{
        LoadLibraryA,
        GetProcAddress,
        nullptr
    };

    auto* SectionHeader = IMAGE_FIRST_SECTION(NtHeaders);
    for (UINT i = 0; i < NtHeaders->FileHeader.NumberOfSections; ++i, ++SectionHeader) {
        if (SectionHeader->SizeOfRawData && !WriteProcessMemory(Process, RemoteBase + SectionHeader->VirtualAddress, DllData.data() + SectionHeader->PointerToRawData, SectionHeader->SizeOfRawData, nullptr)) {
            VirtualFreeEx(Process, RemoteBase, 0, MEM_RELEASE);
            return false;
        }
    }

    memcpy(DllData.data(), &MData, sizeof(MData));
    if (!WriteProcessMemory(Process, RemoteBase, DllData.data(), 0x1000, nullptr)) {
        VirtualFreeEx(Process, RemoteBase, 0, MEM_RELEASE);
        return false;
    }

    auto RemoteShellcode = VirtualAllocEx(Process, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!RemoteShellcode) {
        VirtualFreeEx(Process, RemoteBase, 0, MEM_RELEASE);
        return false;
    }

    if (!WriteProcessMemory(Process, RemoteShellcode, Shellcode, 0x1000, nullptr)) {
        VirtualFreeEx(Process, RemoteBase, 0, MEM_RELEASE);
        VirtualFreeEx(Process, RemoteShellcode, 0, MEM_RELEASE);
        return false;
    }

    HMODULE Ntdll = GetModuleHandleA("ntdll.dll");
    if (!Ntdll) {
        VirtualFreeEx(Process, RemoteBase, 0, MEM_RELEASE);
        VirtualFreeEx(Process, RemoteShellcode, 0, MEM_RELEASE);
        return false;
    }

    NtCreateThreadEx_t NtCreateThreadEx = reinterpret_cast<NtCreateThreadEx_t>(GetProcAddress(Ntdll, "NtCreateThreadEx"));
    if (!NtCreateThreadEx) {
        VirtualFreeEx(Process, RemoteBase, 0, MEM_RELEASE);
        VirtualFreeEx(Process, RemoteShellcode, 0, MEM_RELEASE);
        return false;
    }

    HANDLE RemoteThread = nullptr;
    NTSTATUS Status = NtCreateThreadEx(&RemoteThread, THREAD_ALL_ACCESS, nullptr, Process, RemoteShellcode, RemoteBase, 0, 0, 0, 0, nullptr);

    if (Status != 0 || !RemoteThread) {
        VirtualFreeEx(Process, RemoteBase, 0, MEM_RELEASE);
        VirtualFreeEx(Process, RemoteShellcode, 0, MEM_RELEASE);
        return false;
    }

    CloseHandle(RemoteThread);

    HINSTANCE CheckModule = nullptr;
    while (!CheckModule) {
        MappingData Data{ 0 };
        ReadProcessMemory(Process, RemoteBase, &Data, sizeof(Data), nullptr);
        CheckModule = Data.ModuleHandle;
        Sleep(10);
    }

    VirtualFreeEx(Process, RemoteShellcode, 0, MEM_RELEASE);

    return true;
}

void WINAPI CInjector::Shellcode(LPVOID DataPtr) {
    auto* Data = static_cast<MappingData*>(DataPtr);
    if (!Data) return;

    auto* Base = reinterpret_cast<BYTE*>(Data);
    auto* OptHeader = &reinterpret_cast<IMAGE_NT_HEADERS*>(Base + reinterpret_cast<IMAGE_DOS_HEADER*>(Data)->e_lfanew)->OptionalHeader;

    auto LoadLibraryA = Data->LoadLibraryA;
    auto GetProcAddress = Data->GetProcAddress;
    auto DllMain = reinterpret_cast<DllEntryPointFunc>(Base + OptHeader->AddressOfEntryPoint);

    BYTE* LocationDelta = Base - OptHeader->ImageBase;
    if (LocationDelta) {
        if (!OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) return;

        auto* RelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(Base + OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        while (RelocData->VirtualAddress) {
            UINT EntriesCount = (RelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            auto* RelativeInfo = reinterpret_cast<WORD*>(RelocData + 1);

            for (UINT i = 0; i < EntriesCount; ++i, ++RelativeInfo) {
                if (RELOC_FLAG(*RelativeInfo)) {
                    auto* PatchAddr = reinterpret_cast<UINT_PTR*>(Base + RelocData->VirtualAddress + ((*RelativeInfo) & 0xFFF));
                    *PatchAddr += reinterpret_cast<UINT_PTR>(LocationDelta);
                }
            }

            RelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(RelocData) + RelocData->SizeOfBlock);
        }
    }

    if (OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        auto* ImportDesc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(Base + OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        while (ImportDesc->Name) {
            char* ModuleName = reinterpret_cast<char*>(Base + ImportDesc->Name);
            HINSTANCE Dll = LoadLibraryA(ModuleName);

            ULONG_PTR* ThunkRef = reinterpret_cast<ULONG_PTR*>(Base + ImportDesc->OriginalFirstThunk);
            ULONG_PTR* FuncRef = reinterpret_cast<ULONG_PTR*>(Base + ImportDesc->FirstThunk);

            if (!ThunkRef) ThunkRef = FuncRef;

            for (; *ThunkRef; ++ThunkRef, ++FuncRef) {
                if (IMAGE_SNAP_BY_ORDINAL(*ThunkRef)) {
                    *FuncRef = reinterpret_cast<ULONG_PTR>(GetProcAddress(Dll, reinterpret_cast<char*>(*ThunkRef & 0xFFFF)));
                }
                else {
                    auto* Import = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(Base + (*ThunkRef));
                    *FuncRef = reinterpret_cast<ULONG_PTR>(GetProcAddress(Dll, Import->Name));
                }
            }
            ++ImportDesc;
        }
    }

    if (OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
        auto* TLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(Base + OptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        auto* Callback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(TLS->AddressOfCallBacks);

        while (Callback && *Callback) {
            (*Callback)(Base, DLL_PROCESS_ATTACH, nullptr);
            ++Callback;
        }
    }

    DllMain(Base, DLL_PROCESS_ATTACH, nullptr);
    Data->ModuleHandle = reinterpret_cast<HINSTANCE>(Base);
}