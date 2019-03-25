import ctypes
from ctypes import *

# Load DLL into memory.

hllDll = WinDLL ("C:\\Users\\ahsan\\Desktop\\fingerprint\\inovace_Tarik\\API\\【源码】SYDemo_V2.7\\FGDemo\\lib\\SynoAPIEx.dll")

# Set up prototype and parameters for the desired function call.
# HLLAPI

hllApiProto = ctypes.WINFUNCTYPE (
    ctypes.c_int,      # Return type.
    ctypes.c_void_p,   # Parameters 1 ...
    c_int,
    c_int,
    c_int,
    c_int,
    c_int
    )   # ... thru 4.
hllApiParams = (1, "p1", 0), (1, "p2", 0), (1, "p3", 0), (1, "p4", 0), (1, "p5", 0), (1, "p6", 0),

# Actually map the call ("HLLAPI(...)") to a Python name.

hllApi = hllApiProto (("PSOpenDeviceEx", hllDll), hllApiParams)

# This is how you can actually call the DLL function.
# Set up the variables and call the Python name with them.

p1 = ctypes.c_void_p ()
for i in range(0,10):
    ret = hllApi (byref(p1), i)
    print(ret)
    if(ret==0):
        print(p1)
        break
