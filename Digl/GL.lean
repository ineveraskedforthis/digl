import Digl.Image

namespace gl

def GL_COLOR_BUFFER_BIT : Int32 := 0x00004000
def GL_TEXTURE_2D : Int32 := 0x0DE1

def GL_NEAREST : Int32 := 0x2600
def GL_LINEAR : Int32 := 0x2601
def GL_NEAREST_MIPMAP_NEAREST : Int32 := 0x2700
def GL_LINEAR_MIPMAP_NEAREST : Int32 := 0x2701
def GL_NEAREST_MIPMAP_LINEAR : Int32 := 0x2702
def GL_LINEAR_MIPMAP_LINEAR : Int32 := 0x2703
def GL_TEXTURE_MAG_FILTER : Int32 := 0x2800
def GL_TEXTURE_MIN_FILTER : Int32 := 0x2801
def GL_TEXTURE_WRAP_S : Int32 := 0x2802
def GL_TEXTURE_WRAP_T : Int32 := 0x2803
def GL_CLAMP : Int32 := 0x2900
def GL_REPEAT : Int32 := 0x2901

@[extern "lean_glClear"]
opaque Clear (mask : Int32) : IO Unit

@[extern "lean_glGenTextures"]
opaque GenTexture : IO Int32

@[extern "lean_glBindTexture"]
opaque BindTexture (target : Int32) (texture : Int32) : IO Unit

@[extern "lean_glTexParameteri"]
opaque TexParameteri (target : Int32) (property : Int32) (param : Int32) : IO Unit

@[extern "lean_load_image_to_gpu"]
opaque LoadImage (image : @& Image.Ptr) : IO Unit

end gl
