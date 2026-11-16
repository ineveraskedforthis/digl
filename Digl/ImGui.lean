namespace ImGui

@[extern "lean_display_image"]
opaque DisplayImage (TextureIndex width height : Int32) : IO Unit

end ImGui
