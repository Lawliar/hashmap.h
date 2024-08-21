import ctypes,os
import numpy as np
dir_path = os.path.dirname(os.path.realpath(__file__))
_embhash = ctypes.cdll.LoadLibrary("{}/libhashmap.so".format(dir_path))

def hashmap_hash(number,seed, map_size):
    log2_dict={4:2, 8:3, 16:4, 32:5, 64:6, 128:7, 256:8, 512:9, 1024:10, 2048:11, 4096:12}
    global _embhash
    
    c_int32_p_ty = ctypes.POINTER(ctypes.c_int32)

    wrapped_key = ctypes.c_int32(number)
    key = ctypes.cast(ctypes.addressof(wrapped_key), c_int32_p_ty)
    
    len = ctypes.c_int32(4)

    if(map_size not in log2_dict):
        print("{} not supported".format(map_size))
        exit(0)
    log2cap = ctypes.c_int(log2_dict.get(map_size))
    result = _embhash.hashmap_crc32_hash_helper_int_helper(key,len,seed,log2cap)
    return result
if __name__ == '__main__':
    print(hashmap_hash(342,8))
