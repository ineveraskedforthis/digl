namespace Image

opaque Ptr : Type

@[extern "lean_read_image"]
opaque Read (path : @& String) : IO Ptr

@[extern "lean_write_dds"]
opaque WriteDDS (path : @& String) (data : @& Ptr) : IO Unit

@[extern "lean_image_width"]
opaque Width (data : @& Ptr) : Int32

@[extern "lean_image_height"]
opaque Height (data : @& Ptr) : Int32

@[extern "downscale_image"]
opaque ScaleDown (data : @& Ptr) (x : Int32) (y : Int32) : IO Ptr

@[extern "paste_image"]
opaque Paste (Source : @& Ptr) (Destination : @& Ptr) (x : Int32) (y : Int32) : IO Unit

end Image
