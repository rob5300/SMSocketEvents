# Sockets Extension

## Building
Ensure you have the required dependencies for [building Sourcemod](https://wiki.alliedmods.net/Building_sourcemod).

Metamod, Sourcemod and hl2sdk-tf2 are needed.
### Linux:
Install/update ``g++ gcc g++-multilib gcc-multilib libboost-all-dev openssl:i386`` via your package manager.

Use the provided make file via ``make``. Requires GCC 8+ (Ambuild cannot be used).
### Windows:
Install vcpkg and install from manifest: ``vcpkg install --triplet x86-windows``

Open and build using the solution file (``socket_ext.sln``) in VS 2022+
