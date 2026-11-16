-- This module serves as the root of the `Digl` library.
-- Import modules here that should be built as part of the library.
import Digl.Basic
import Lean
import Std.WP
import Std.Tactic.Do

import Digl.Token
import Digl.Script

open Std.WP Lean.Order
open Std.Do

set_option experimental.vcgen true

@[extern "init_lean_classes"]
opaque InitClasses : IO Unit

-- An opaque type to represent `GLFWwindow *`
opaque GLFWwindow_ptr : Type

@[extern "lean_init_glfw_window"]
opaque InitWindow : IO GLFWwindow_ptr

@[extern "lean_shutdown_glfw_window"]
opaque ShutdownWindow (w : @& GLFWwindow_ptr) : IO Unit

@[extern "lean_start_frame"]
opaque StartFrame : IO Unit
@[extern "lean_end_frame"]
opaque EndFrame : IO Unit

@[extern "lean_swap_buffers"]
opaque SwapBuffers (w : @& GLFWwindow_ptr) : IO Unit
@[extern "lean_should_close"]
opaque ShouldClose (w : @& GLFWwindow_ptr) : IO Bool

@[extern "lean_poll_events"]
opaque PollEvents : IO Unit

@[extern "draw_thing"]
opaque Draw : IO Unit



namespace ImGui
inductive UIScope where
  | Root : UIScope
  | Window : UIScope
end ImGui



structure Vic2Mod (m) [Monad m] where
  mod_path : System.FilePath
  name : m String
  content_path : m System.FilePath
  user_dir : Option System.FilePath
  replace_path : List System.FilePath
  sprite_types : Array V2_GFX.SpriteType

structure Paths where
  v2_path : Option System.FilePath
  local_mods : Array System.FilePath
  selected_mod : Option (Vic2Mod Id)


structure ModelState where
  imgui_stack : List ImGui.UIScope
  imgui_element_over_button : Bool

structure ErrorList where
  mod_description : List String

structure SpritesFilter where
  starts_with : String
  frames : Nat

structure FiltersCollection where
  sprites : SpritesFilter := {
    frames := 30
    starts_with := ""
  }

structure ProgramState where
  window : GLFWwindow_ptr
  errors : ErrorList
  paths : Paths
  filters : FiltersCollection := {

  }


-- def NothingModel (s : WindowState) : WindowState := s

-- abbrev MIO_Bool := StateT WindowStateForeign IO Bool
-- abbrev M_Bool := StateM WindowStateManaged MIO_Bool

-- abbrev M' (t) := StateT WindowStateManaged Id (StateT GLFWwindow_ptr IO t)

structure VerificationPair (α) where
  execution : StateT ProgramState IO α
  model : StateT ModelState Id α

-- def VerificationPair.bind_internal {α} {β} (x : VerificationPair α) (f : α → VerificationPair β) : VerificationPair β := {
--   execution := x.execution >>= fun s => (f s).execution
--   model := x.model >>= fun s => (f s).model
-- }

-- -- No actual execution here to make it effect-free on model level
-- @[implemented_by VerificationPair.bind_internal]
-- def VerificationPair.bind {α} {β} (x : VerificationPair α) (f : α → VerificationPair β) : VerificationPair β := {
--   execution := x.execution >>= fun s => (f s).execution
--   model := x.model >>= fun s => (f s).model
-- }


def VerificationPair.run_model
  {α : Type}
  (x : VerificationPair α)
  (s : ModelState)
:
  (α × ModelState)
:=
  x.model s
-- def VerificationPair.run_model (x : Monad VerificationPair α) : α

instance VerificationPair.IsMonad : Monad VerificationPair := {
  pure x := {
    execution := pure x
    model := pure x
  }
  bind x f := {
    execution := x.execution >>= fun s => (f s).execution
    model := x.model >>= fun s => (f s).model
  }
}

namespace ImGui

@[extern "imgui_begin"]
opaque _Begin (s : String) : IO Unit
@[extern "imgui_end"]
opaque _End : IO Unit
@[extern "imgui_text"]
opaque _Text (s : String) : IO Unit


@[extern "imgui_button"]
opaque _Button (s : String) : IO Bool

end ImGui

namespace gl
def GL_COLOR_BUFFER_BIT : UInt32 := 0x00004000
@[extern "lean_glClear"]
opaque Clear (mask : UInt32) : IO Unit
end gl

@[extern "lean_get_path"]
opaque QueryPathString : IO String


def make_paths : Paths := {
  v2_path := none
  local_mods := Array.empty
  selected_mod := none
}

def make_program_state (w : GLFWwindow_ptr) : ProgramState := {
  window := w
  paths := make_paths
  errors := {
    mod_description := []
  }
}


def Window
  (label : String)
  (w : ProgramState)
  (content : (w : ProgramState) → IO ProgramState)
: IO ProgramState := do
  ImGui._Begin label
  let w := ← content w
  ImGui._End
  pure w

def WindowIf
  (label : String)
  (w : ProgramState)
  (condition : ProgramState → Bool)
  (content : (w : ProgramState) → (h : condition w = true) → IO ProgramState)
: IO ProgramState := do
  if h : condition w then
    ImGui._Begin label
    let w := ← content w h
    ImGui._End
    pure w
  else
    pure w

def WindowIfElse
  (label : String)
  (w : ProgramState)
  (condition : ProgramState → Bool)
  (content : (w : ProgramState) → (h : condition w) → IO ProgramState)
  (content_else : (w : ProgramState) → (h : ¬ condition w) → IO ProgramState)
: IO ProgramState := do
  if h : decide (condition w) then
    ImGui._Begin label
    let w := ← content w h
    ImGui._End
    pure w
  else
    ImGui._Begin label
    let w := ← content_else w h
    ImGui._End
    pure w

def IsMod (p : System.FilePath) : Bool :=
  match p.fileName with
  | some x => x.endsWith ".mod"
  | none => False

def GatherMods (depth : Nat) (path : System.FilePath)  : IO (Array System.FilePath) := do
  let is_dir := ←path.isDir
  if is_dir then
    match depth with
    | 0 => pure #[]
    | k + 1 =>
        let dirs := ←path.walkDir  (fun x => pure (x == path))
        let children := dirs.map (GatherMods k)
        children.foldr (fun x y => do
          pure ((←x) ++ (←y))
        ) (pure #[])
  else
    if IsMod path then
      pure #[path]
    else
      pure #[]

def ModSelect
  (p : System.FilePath)
  (selected : Option System.FilePath)
:
  OptionT IO System.FilePath
:= do
  if ←ImGui._Button p.toString then
    pure p
  else
    match selected with
    | none => OptionT.fail
    | some x => pure x


  -- rw [Nat.sub_add_eq]

-- def test_string := "I play \"Project Alice\"\nSometimes...\n\r# it's quite fun..."
-- #eval SplitTokens test_string test_string.startPos "" #[] false false


inductive ModInfoLHS where
| name : ModInfoLHS
| path : ModInfoLHS
| user_dir : ModInfoLHS
| replace_path : ModInfoLHS

inductive ModInfoTokenKind where
| lhs : ModInfoLHS → ModInfoTokenKind
| eq : ModInfoTokenKind
| rhs : ModInfoTokenKind

def ExtractModInfo (data : List RawToken) (c : List ModInfoTokenKind) (acc : Vic2Mod Option) : Vic2Mod Option :=
  match data with
  | [] => acc
  | RawToken.Comment _ :: tail =>
    ExtractModInfo tail c acc
  | RawToken.Signifier token :: tail =>
    if token = "=" then
      ExtractModInfo tail (ModInfoTokenKind.eq :: c) acc
    else
      match c with
      | ModInfoTokenKind.eq :: ModInfoTokenKind.lhs a :: _ =>
        match a with
        | ModInfoLHS.name =>
          ExtractModInfo tail [] {acc with name := token}
        | ModInfoLHS.path =>
          ExtractModInfo tail [] {acc with content_path := token}
        | ModInfoLHS.user_dir =>
          ExtractModInfo tail [] {acc with user_dir := token}
        | ModInfoLHS.replace_path =>
          ExtractModInfo tail [] {acc with replace_path := token :: acc.replace_path}
      | c =>
        if token = "name" then
          ExtractModInfo tail (ModInfoTokenKind.lhs ModInfoLHS.name :: c) acc
        else if token = "path" then
          ExtractModInfo tail (ModInfoTokenKind.lhs ModInfoLHS.path :: c) acc
        else if token = "user_dir" then
          ExtractModInfo tail (ModInfoTokenKind.lhs ModInfoLHS.user_dir :: c) acc
        else if token = "replace_path" then
          ExtractModInfo tail (ModInfoTokenKind.lhs ModInfoLHS.replace_path :: c) acc
        else
          ExtractModInfo tail [] acc


def test_descritption := "
# random long comment which makes  things more difficult
name = \"Random Mod name\"
path = \"mod/random_mod/actual_folder\"
user_dir = \"RM\"
replace_path = \"history/provinces\"
replace_path = \"events\"
replace_path = \"decisions\"
"

def tokens := SplitTokens test_descritption test_descritption.startPos "" #[] false false
-- #eval tokens


def parsed := ExtractModInfo (tokens.toList) [] {
  mod_path := "something"
  name := none
  content_path := none
  user_dir := none
  replace_path := []
  sprite_types := #[]
}

#eval parsed.name
#eval parsed.content_path
#eval parsed.replace_path
#eval parsed.user_dir
#eval parsed.mod_path


def ParseModDescriptor
  (s : String) (mod : System.FilePath)
: Vic2Mod Option :=
  let tokens := SplitTokens s s.startPos "" #[] false false
  ExtractModInfo (tokens.toList) [] {
    mod_path := mod
    name := none
    content_path := none
    user_dir := none
    replace_path := []
    sprite_types := #[]
  }

def ExtractGFX_file
  (file_absolute_path : System.FilePath)
:
  IO (Array V2_GFX.SpriteType)
:= do
  IO.println file_absolute_path
  if ¬ (← file_absolute_path.isDir) then
    IO.println "Open: Path is not a directory."
    if file_absolute_path.extension = some "gfx" then
      let file := ←IO.FS.readFile file_absolute_path
      let sprites := V2_GFX.ParseString file
      IO.println (toString sprites.size ++ " sprited were detected")
      pure sprites
    else
      IO.println "Skip: Path is not a gfx file."
      pure #[]
  else
    IO.println "Skip: Path is a directory."
    pure #[]

def ExtractGFX_content_path (content_absolute_path : System.FilePath) : IO (Array V2_GFX.SpriteType) := do
  IO.println ("Extract gfx from " ++ (content_absolute_path / "interface").toString)
  let interface_path := content_absolute_path / "interface"
  if ←interface_path.isDir then
    IO.println "Valid directory"
    let files := ←interface_path.walkDir  (fun x => pure (x == interface_path))
    files.flatMapM ExtractGFX_file
  else
    IO.println "Invalid directory"
    pure #[]


def ValidateVic2ModDescription
  (v : Vic2Mod Option)
:
  Except (List ModInfoLHS) (Vic2Mod Id)
:=
  match v.name, v.content_path with
  | none, none => Except.error [ModInfoLHS.name, ModInfoLHS.path]
  | _, none => Except.error [ModInfoLHS.path]
  | none, _ => Except.error [ModInfoLHS.name]
  | some name, some path =>
    Except.ok {
      mod_path := v.mod_path
      name := name
      content_path := path
      user_dir := v.user_dir
      replace_path := v.replace_path
      sprite_types := #[]
    }

def ReadModDescriptor (p : Option System.FilePath) : ExceptT (List ModInfoLHS) IO (Vic2Mod Id) := do
  match p with
  | none => do
    ExceptT.mk (pure (Except.error []))
  | some x => do
    let file := ←IO.FS.readFile x
    ValidateVic2ModDescription (ParseModDescriptor file x)

def ModDescriptorErrors (l : List ModInfoLHS) (acc : List String) : List String :=
  match l with
  | [] => acc
  | e :: tail =>
    match e with
    | ModInfoLHS.name =>
      ModDescriptorErrors tail ("Missing mod name" :: acc)
    | ModInfoLHS.path =>
      ModDescriptorErrors tail ("Missing mod path" :: acc)
    | _ => acc


def LoadSprites (game_path : System.FilePath) (v : Vic2Mod Id) : IO (Vic2Mod Id) := do
  IO.println "Load sprites function"
  pure {v with sprite_types := ←ExtractGFX_content_path (game_path / v.content_path.run) }


def ShowMods (w : ProgramState) (_ : w.paths.v2_path.isSome = true) : IO ProgramState := do
  let result := ←w.paths.local_mods.foldrM ModSelect none
  match ←ReadModDescriptor result with
  | Except.error l =>
    pure { w with errors.mod_description := ModDescriptorErrors l [] }
  | Except.ok v =>
    pure { w with paths.selected_mod := v }

abbrev ValidModSelection (w : ProgramState) : Bool := w.paths.selected_mod.isSome = true ∧ w.paths.v2_path.isSome = true
abbrev ValidModSelection_Prop (w : ProgramState) : Prop := w.paths.selected_mod.isSome = true ∧ w.paths.v2_path.isSome = true


def ModDescription
  (w : ProgramState)
  (h : ValidModSelection w)
:
  IO ProgramState
:= do
  have valid : ValidModSelection_Prop w := by
    exact Bool.and_eq_true_iff.mp h
  let desc := w.paths.selected_mod.get valid.1
  ImGui._Text desc.name
  let mut load_sprites : Bool := false
  if ← ImGui._Button "Load sprites" then
    load_sprites := true
  ImGui._Text ("Content folder path: " ++ desc.content_path.toString)
  ImGui._Text ("Path to the .mod file: " ++ desc.mod_path.toString)
  ImGui._Text "Replaced paths: "
  desc.replace_path.forM (fun x => ImGui._Text x.toString )

  if load_sprites then
    IO.println "Load sprites"
    let updated_desc := ←(LoadSprites (w.paths.v2_path.get valid.2) desc)
    pure {w with paths.selected_mod := some updated_desc }
  else
    pure w

def ModDescriptionErrors
  (w : ProgramState)
  (_ : ¬(ValidModSelection w))
:
  IO ProgramState
:= do
  ImGui._Text "Mod description errors:"
  w.errors.mod_description.forM (fun x => ImGui._Text x )
  pure w

def SpriteDescription (s : V2_GFX.SpriteType) (w : ProgramState)  : IO ProgramState := do
  if h: s.name.isSome ∧ s.frames.isSome then
    let name := s.name.get h.1
    let frames := s.frames.get h.2
    if ¬ (name.startsWith w.filters.sprites.starts_with ∧ frames ≥ w.filters.sprites.frames) then
      pure w
    else
      if h : s.name.isSome then
        ImGui._Text ("Name: " ++ (s.name.get h))
      else
        ImGui._Text ("Name: Unknown")
      if h : s.texture_path.isSome then
        ImGui._Text ("Path: " ++ (s.texture_path.get h).toString)
      else
        ImGui._Text ("Path: Unknown")
      if h : s.frames.isSome then
        ImGui._Text ("Frames: " ++ toString (s.frames.get h))
      else
        ImGui._Text ("Frames: default (1)")
      pure w
  else
    pure w



def SpritesList (w : ProgramState) : IO ProgramState := do
  let desc := w.paths.selected_mod
  ImGui._Text "Sprites list"
  match desc with
  | none =>
    ImGui._Text "Select a mod."
    pure w
  | some d =>
    if d.sprite_types.size = 0 then
      ImGui._Text "There are no sprites in the mod or you haven't loaded them."
      pure w
    else
      d.sprite_types.foldrM (SpriteDescription) w

def ChooseFolder (w : ProgramState) : IO ProgramState := do
  let button_result := ←ImGui._Button "Choose V2 folder"
  if button_result then
    let path : System.FilePath := ←QueryPathString
    let mods_path := path / "mod"
    pure {
      window := w.window
      paths := {
        v2_path := path
        local_mods := ←(GatherMods 2 mods_path)
        selected_mod := none
      }
      errors := {
        mod_description := ["No mod description was selected"]
      }
    }
  else
    pure w

def DrawWindow (w : ProgramState) : IO ProgramState := do
  -- IO.println s!"Frame"
  StartFrame
  gl.Clear gl.GL_COLOR_BUFFER_BIT

  -- gather computations into wio
  let w := ←Window "Path picker" w  ChooseFolder
  let w := ←WindowIf "Vic2 Mods" w _ ShowMods
  let w := ←WindowIfElse "Selected mod" w ValidModSelection ModDescription ModDescriptionErrors
  let w := ←Window "Sprites table" w SpritesList

  EndFrame
  SwapBuffers w.window
  PollEvents
  pure w

-- def DrawWindowN (w : WindowState) (n : Nat) : EST IO.Error IO.RealWorld Unit := do
--   match n with
--   | 0 => pure ()
--   | k + 1 =>
--     StartFrame
--     Draw
--     EndFrame
--     SwapBuffers w.window
--     PollEvents
--     DrawWindowN w k

def NextFrame (w : ProgramState) : IO (Sum ProgramState Unit) := do
  if ← ShouldClose w.window then
    IO.println s!"Shutdown!"
    ShutdownWindow w.window
    pure (Sum.inr ())
  else
    (DrawWindow w).bind (fun x => pure (Sum.inl x))

def Run : IO Unit := do
  IO.println s!"Init?"
  -- InitClasses
  IO.println s!"Hello, {hello}!"
  -- let w := InitWindow.bind (fun x => make_window_state x)

  -- w.bind DrawWindow
  InitWindow.bind (
    fun wb => IO.iterate (make_program_state wb) NextFrame
  )
  -- InitWindow.bind (
  --   fun wb => DrawWindow (make_window_state wb)
  -- )
  -- InitWindow.bind (fun wb => DrawWindowN (make_window_state wb) (100))
  IO.println s!"Bye, {hello}!"
  pure ()
