The source code to max2w3x.dle, max2w3d.dle, wdump.exe and memorymanager.dll is included with this package.

To compile it out-of-the-box you will need Microsoft Visual Studio 2019 (the community edition of VS 2019 will work for this and its recommended you have the latest patch), the 3D Studio Max 2023 SDK and the Microsoft DirectX SDK.
For the DirectX SDK you need to download this
https://www.microsoft.com/en-au/download/details.aspx?id=6812 and install it.
Go to the DirectX SDK install folder and go into the include folder. Copy d3dx9.h, d3dx9anim.h, d3dx9core.h, d3dx9effect.h, d3dx9math.h, d3dx9math.inl, d3dx9mesh.h, d3dx9shader.h, d3dx9shape.h, d3dx9tex.h and d3dx9xof.h into the dep\dxsdk_june10\include folder in the source tree. Do not copy any other files.
Then go to the DirectX SDK install folder and go into the lib\x64 folder. Copy d3dx9.lib (and only that file) into the dep\dxsdk_june10\lib\x64 folder in the source tree.

For the Max SDK you need to copy the include and lib folders from the 3D Studio Max 2023 SDK to the dep\maxsdk folder in the source tree.
Then to compile the project you open tt_vc2012.sln.

If you are unable to get it to compile please contact myself (jonwil on the w3dhub forums or Jonathan Wilson on the w3dhub Discord) for assistance.

The code is licensed under the GNU GPL version 3.0 as described in gpl-3.0.txt

In addition to the GNU GPL version 3.0 you are granted a specific exemption to allow linking of the source code for max2w3d.dle with the binaries from any 3D program (including any 3rd party plugins you may have installed) provided the source code to your
modified version of max2w3d.dle (or whatever plugin you are writing that includes code from max2w3d.dle) is released in accordance with the GNU GPL version 3.0
