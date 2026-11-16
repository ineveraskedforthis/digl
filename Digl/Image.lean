namespace Image

opaque Ptr : Type

@[extern "lean_read_image"]
opaque Read (path : String) : IO Ptr

@[extern "lean_image_width"]
opaque Width (data : @& Ptr) : Int32

@[extern "lean_image_height"]
opaque Height (data : @& Ptr) : Int32

end Image
