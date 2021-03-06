#if 0
//
// Generated by Microsoft (R) D3DX9 Shader Compiler 
//
//   fxc /nologo /T vs_2_0 /E Simple1 /Fh Simple1_vs.h Simple1.vs
//
//
// Parameters:
//
//   float4x4 mWorldViewProj;
//
//
// Registers:
//
//   Name           Reg   Size
//   -------------- ----- ----
//   mWorldViewProj c0       4
//

    vs_2_0
    dcl_position v0
    dcl_color v1
    dcl_texcoord v2
    dp4 oPos.x, v0, c0
    dp4 oPos.y, v0, c1
    dp4 oPos.z, v0, c2
    dp4 oPos.w, v0, c3
    mov oD0, v1
    mov oT0.xy, v2

// approximately 6 instruction slots used
#endif

const DWORD g_vs20_Simple1[] =
{
    0xfffe0200, 0x0020fffe, 0x42415443, 0x0000001c, 0x00000057, 0xfffe0200, 
    0x00000001, 0x0000001c, 0x20000100, 0x00000050, 0x00000030, 0x00000002, 
    0x00000004, 0x00000040, 0x00000000, 0x726f576d, 0x6956646c, 0x72507765, 
    0xab006a6f, 0x00030003, 0x00040004, 0x00000001, 0x00000000, 0x325f7376, 
    0x4d00305f, 0x6f726369, 0x74666f73, 0x29522820, 0x44334420, 0x53203958, 
    0x65646168, 0x6f432072, 0x6c69706d, 0x00207265, 0x0200001f, 0x80000000, 
    0x900f0000, 0x0200001f, 0x8000000a, 0x900f0001, 0x0200001f, 0x80000005, 
    0x900f0002, 0x03000009, 0xc0010000, 0x90e40000, 0xa0e40000, 0x03000009, 
    0xc0020000, 0x90e40000, 0xa0e40001, 0x03000009, 0xc0040000, 0x90e40000, 
    0xa0e40002, 0x03000009, 0xc0080000, 0x90e40000, 0xa0e40003, 0x02000001, 
    0xd00f0000, 0x90e40001, 0x02000001, 0xe0030000, 0x90e40002, 0x0000ffff
};
