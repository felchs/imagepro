#pragma once
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

namespace imagepro
{
    class Int
    {
    private:
        int* data;
        DWORD oldProtect;
        int key;
        size_t size;

        void protectMemory()
        {
            VirtualProtect(data, size, PAGE_NOACCESS, &oldProtect);
        }

        void unprotectMemory()
        {
            VirtualProtect(data, size, PAGE_READWRITE, &oldProtect);
        }

        int encrypt(int value) const
        {
            return value ^ key;
        }

        int decrypt(int value) const
        {
            return value ^ key;
        }

    public:
        Int(int value)
        {
            key = rand(); // Random XOR key
            size = sizeof(int);

            data = (int*)VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!data)
            {
                throw std::runtime_error("Memory allocation failed.");
            }

            *data = encrypt(value);
            protectMemory();
        }

        ~Int()
        {
            unprotectMemory();
            VirtualFree(data, 0, MEM_RELEASE);
        }

        int get()
        {
            int result = 0;
            __try
            {
                unprotectMemory();
                result = decrypt(*data);
                protectMemory();
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                std::cerr << "Access violation in get()\n";
            }
            return result;
        }

        void set(int value)
        {
            __try
            {
                unprotectMemory();
                *data = encrypt(value);
                protectMemory();
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                std::cerr << "Access violation in set()\n";
            }
        }
    };
}