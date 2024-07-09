# Microsoft Developer Studio Project File - Name="pdlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=pdlib - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "pdlib.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "pdlib.mak" CFG="pdlib - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "pdlib - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "pdlib - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pdlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PDLIB_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PDLIBDLL_EXPORTS" /D "NT" /D "PD" /D "PD_INTERNAL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libc.lib oldnames.lib wsock32.lib winmm.lib /nologo /dll /pdb:"..\bin/pdlib.pdb" /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"oldnames" /nodefaultlib:"kernel" /nodefaultlib:"uuid" /nodefaultlib:"libcmt" /out:"..\bin/pd.dll" /export:sys_main
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "pdlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PDLIB_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PDLIBDLL_EXPORTS" /D "NT" /D "PD" /D "PD_INTERNAL" /YX /FD /GZ /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libc.lib oldnames.lib wsock32.lib winmm.lib /nologo /dll /pdb:"..\bin/pdlib.pdb" /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"oldnames" /nodefaultlib:"kernel" /nodefaultlib:"uuid" /nodefaultlib:"libcmtd" /out:"..\bin/pd.dll" /export:sys_main
# SUBTRACT LINK32 /pdb:none /incremental:no /nodefaultlib

!ENDIF 

# Begin Target

# Name "pdlib - Win32 Release"
# Name "pdlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\d_arithmetic.c
# End Source File
# Begin Source File

SOURCE=..\src\d_array.c
# End Source File
# Begin Source File

SOURCE=..\src\d_ctl.c
# End Source File
# Begin Source File

SOURCE=..\src\d_dac.c
# End Source File
# Begin Source File

SOURCE=..\src\d_delay.c
# End Source File
# Begin Source File

SOURCE=..\src\d_fft.c
# End Source File
# Begin Source File

SOURCE=..\src\d_fftroutine.c
# End Source File
# Begin Source File

SOURCE=..\src\d_filter.c
# End Source File
# Begin Source File

SOURCE=..\src\d_global.c
# End Source File
# Begin Source File

SOURCE=..\src\d_math.c
# End Source File
# Begin Source File

SOURCE=..\src\d_mayer_fft.c
# End Source File
# Begin Source File

SOURCE=..\src\d_misc.c
# End Source File
# Begin Source File

SOURCE=..\src\d_osc.c
# End Source File
# Begin Source File

SOURCE=..\src\d_soundfile.c
# End Source File
# Begin Source File

SOURCE=..\src\d_ugen.c
# End Source File
# Begin Source File

SOURCE=..\src\g_7_guis.c
# End Source File
# Begin Source File

SOURCE=..\src\g_array.c
# End Source File
# Begin Source File

SOURCE=..\src\g_canvas.c
# End Source File
# Begin Source File

SOURCE=..\src\g_graph.c
# End Source File
# Begin Source File

SOURCE=..\src\g_guiconnect.c
# End Source File
# Begin Source File

SOURCE=..\src\g_io.c
# End Source File
# Begin Source File

SOURCE=..\src\g_rtext.c
# End Source File
# Begin Source File

SOURCE=..\src\g_scalar.c
# End Source File
# Begin Source File

SOURCE=..\src\g_template.c
# End Source File
# Begin Source File

SOURCE=..\src\g_text.c
# End Source File
# Begin Source File

SOURCE=..\src\g_traversal.c
# End Source File
# Begin Source File

SOURCE=..\src\m_atom.c
# End Source File
# Begin Source File

SOURCE=..\src\m_binbuf.c
# End Source File
# Begin Source File

SOURCE=..\src\m_class.c
# End Source File
# Begin Source File

SOURCE=..\src\m_conf.c
# End Source File
# Begin Source File

SOURCE=..\src\m_glob.c
# End Source File
# Begin Source File

SOURCE=..\src\m_memory.c
# End Source File
# Begin Source File

SOURCE=..\src\m_obj.c
# End Source File
# Begin Source File

SOURCE=..\src\m_pd.c
# End Source File
# Begin Source File

SOURCE=..\src\m_sched.c
# End Source File
# Begin Source File

SOURCE=..\src\s_file.c
# End Source File
# Begin Source File

SOURCE=..\src\s_inter.c
# End Source File
# Begin Source File

SOURCE=..\src\s_loader.c
# End Source File
# Begin Source File

SOURCE=..\src\s_main.c
# End Source File
# Begin Source File

SOURCE=..\src\s_nt.c
# End Source File
# Begin Source File

SOURCE=..\src\s_path.c
# End Source File
# Begin Source File

SOURCE=..\src\s_print.c
# End Source File
# Begin Source File

SOURCE=..\src\s_unix.c
# End Source File
# Begin Source File

SOURCE=..\src\x_acoustics.c
# End Source File
# Begin Source File

SOURCE=..\src\x_arithmetic.c
# End Source File
# Begin Source File

SOURCE=..\src\x_connective.c
# End Source File
# Begin Source File

SOURCE=..\src\x_gui.c
# End Source File
# Begin Source File

SOURCE=..\src\x_interface.c
# End Source File
# Begin Source File

SOURCE=..\src\x_midi.c
# End Source File
# Begin Source File

SOURCE=..\src\x_misc.c
# End Source File
# Begin Source File

SOURCE=..\src\x_net.c
# End Source File
# Begin Source File

SOURCE=..\src\x_qlist.c
# End Source File
# Begin Source File

SOURCE=..\src\x_time.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
