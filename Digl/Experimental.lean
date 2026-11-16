-- namespace ImGui

-- structure OGL_State where {
--   binded_texture : Option Int32
-- }

-- def GL_actual := IO
-- @[implemented_by GL_actual]
-- def GL : (Type → Type) := StateM OGL_State
-- instance GL.is_monad : Monad (GL) := StateT.instMonad
-- instance GL.is_WP [WP Id ps] : WP (GL) (.arg OGL_State ps) := {
--   wp x :=  PredTrans.pushArg (fun s => wp (x.run s))
-- }

-- abbrev GL' := StateT OGL_State Id
-- axiom GL'_WP {α : Type 0} (ps : outParam PostShape) (x : GL' α) : PredTrans (PostShape.arg OGL_State ps) α
-- instance GL'.is_monad : Monad (GL') := StateT.instMonad
-- noncomputable instance GL'.is_WP [WP Id ps] : WP (GL') (.arg OGL_State ps) := {
--   wp x := GL'_WP ps x
-- }
-- def BeginInternal (s : String) : GL' Unit  :=
--   StateT.lift (_Begin s)
-- @[implemented_by BeginInternal]
-- def Begin (s : String) : GL' Unit :=
--   pure ()

-- @[spec]
-- theorem BeginIsSafe
--   (s : String)
--   (gl : OGL_State)
-- :
--   ⦃fun x => x = gl⦄ (Begin s) ⦃fun _ x => x = gl⦄
-- := by
--   vcgen [Begin] with finish


-- end ImGui
