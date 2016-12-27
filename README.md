# BoostVXL
Vector Can case XL  Python wrapper.




solution for compile error:
1. modify the cmd file:
  b2_msvc_14.0_vcvarsall_x86.cmd:
  SET LIBPATH=C:\Windows\Microsoft.NET\Framework\v4.0.30319;C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\LIB;C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\ATLMFC\LIB;C:\Program Files (x86)\Windows Kits\8.1\References\CommonConfiguration\Neutral;\Microsoft.VCLibs\14.0\References\CommonConfiguration\neutral;C:\download\boost_1_62_0\bin.v2\libs\python\build\msvc-14.0\debug\threading-multi;C:\Python35\libs\;
2. run link manually.
  D:\BoostVXL>link /NOLOGO /INCREMENTAL:NO /DLL /DEBUG /MACHINE:X86 /MANIFEST /subsystem:console /out:"bin\msvc-14.0\debug\VectorCan.pyd" /IMPLIB:"bin\msvc-14.0\debug\VectorCan.lib" /LIBPATH:"C:\Python35\libs"   @"bin\msvc-14.0\debug\VectorCan.pyd.rsp" C:\download\boost_1_62_0\bin.v2\libs\python\build\msvc-14.0\debug\threading-multi\boost_python-vc140-mt-gd-1_62.lib
