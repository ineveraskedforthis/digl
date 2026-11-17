-- import Lean
import Std.WP
import Std.Tactic.Do

namespace ImGui

open Std.WP Lean.Order

set_option experimental.vcgen true

@[extern "lean_display_image"]
opaque DisplayImage (TextureIndex : Int32) (width height u0 v0 u1 v1 : Float32) : IO Unit

@[extern "lean_BeginCombo"]
opaque BeginDropdown (label preview : @& String) : IO Bool

@[extern "lean_Selectable"]
opaque Selectable (label : @& String) (selected : Bool) : IO Bool

@[extern "lean_SetItemDefaultFocus"]
opaque SetDefaultFocus : IO Unit

@[extern "lean_EndCombo"]
opaque EndDropdown : IO Unit


class ImGuiIO (io : Type _ → Type _) [Monad io] (context_t : Type _) where
  DisplayImage (TextureIndex : Int32) (width height u0 v0 u1 v1 : Float32) : StateT context_t io Unit
  BeginDropdown (label preview : String) : StateT context_t io Bool
  Selectable (label : String) (selected : Bool) : StateT context_t io Bool
  SetDefaultFocus : StateT context_t io Unit
  EndDropdown : StateT context_t io Unit

  -- expose_internals : Prop
  valid_context (ctx : context_t) : Prop
  -- dropdown_started (ctx : context_t) : Prop


instance Raw (t : Type _) : ImGuiIO IO t := {
  DisplayImage img w h u₀ v₀ u₁ v₁ := DisplayImage img w h u₀ v₀ u₁ v₁
  BeginDropdown label preview := BeginDropdown label preview
  Selectable label selected := Selectable label selected
  SetDefaultFocus := SetDefaultFocus
  EndDropdown := EndDropdown

  valid_context _ := True
}

structure ImGuiContext where
  dropdown_started : Bool
  mouse_over_dropdown : Bool
  mouse_over_selection : Bool
  valid_state : Bool

def DisplayImageModel (_ : Int32) (_ _ _ _ _ _ : Float32) : StateT ImGuiContext Id Unit :=
  pure ()


def BeginDropdownModel (_ _ : @& String) : StateT ImGuiContext Id Bool := do
  let s := ←get
  if s.valid_state then
    if s.dropdown_started then
      modify (fun x => {x with valid_state := False})
      pure false
    else
      if s.mouse_over_dropdown then
        modify (fun x => {x with dropdown_started := True})
        pure true
      else
        pure false
  else
    pure false


def SelectableModel (_ : @& String) (_ : Bool) : StateT ImGuiContext Id Bool := do
  pure ((←get).mouse_over_selection)

def SetDefaultFocusModel : StateT ImGuiContext Id Unit := do
  pure ()

def EndDropdownModel : StateT ImGuiContext Id Unit := do
  if (←get).dropdown_started then
    modify (fun x => {x with dropdown_started := False})
  else
    modify (fun x => {x with valid_state := False})

instance Model : ImGuiIO Id ImGuiContext := {
  DisplayImage img w h u₀ v₀ u₁ v₁ := DisplayImageModel img w h u₀ v₀ u₁ v₁
  BeginDropdown label preview := BeginDropdownModel label preview
  Selectable label selected := SelectableModel label selected
  SetDefaultFocus := SetDefaultFocusModel
  EndDropdown := EndDropdownModel

  valid_context ctx := ctx.valid_state
}


@[spec]
theorem BeginDropdownModel_spec
  (s₁ s₂ : String)
  (state : ImGuiContext)
:
  ⦃ fun x => x = state⦄
  BeginDropdownModel s₁ s₂
  ⦃ fun result x =>
    if state.valid_state then
      if state.dropdown_started then
        ¬x.valid_state ∧ result = false
      else
        if state.mouse_over_dropdown then
          x.valid_state ∧ x.dropdown_started ∧ result = true
        else
          x.valid_state ∧ ¬x.dropdown_started ∧ result = false
    else
      ¬x.valid_state ∧ result = false
  ⦄
:= by
  vcgen [BeginDropdownModel] <;> try grind

@[spec]
theorem instBeginDropdownModel_spec
  (s₁ s₂ : String)
  (state : ImGuiContext)
:
  ⦃ fun x => x = state⦄
  @ImGuiIO.BeginDropdown Id _ ImGuiContext _ s₁ s₂
  ⦃ fun result x =>
    if state.valid_state then
      if state.dropdown_started then
        ¬x.valid_state ∧ result = false
      else
        if state.mouse_over_dropdown then
          x.valid_state ∧ x.dropdown_started ∧ result = true
        else
          x.valid_state ∧ ¬x.dropdown_started ∧ result = false
    else
      ¬x.valid_state ∧ result = false
  ⦄
:= by
  vcgen [ImGuiIO.BeginDropdown] <;> try grind


@[spec]
theorem EndDropdownModel_spec
  (state : ImGuiContext)
:
  ⦃ fun x => x = state⦄
  EndDropdownModel
  ⦃ fun _ x =>
    if state.valid_state ∧ state.dropdown_started then
      ¬x.dropdown_started ∧ x.valid_state
    else
      ¬x.valid_state
  ⦄
:= by
  vcgen [EndDropdownModel] <;> try grind

@[spec]
theorem instEndDropdownModel_spec
  (state : ImGuiContext)
:
  ⦃ fun x => x = state⦄
  @ImGuiIO.EndDropdown Id _ ImGuiContext _
  ⦃ fun _ x =>
    if state.valid_state ∧ state.dropdown_started then
      ¬x.dropdown_started ∧ x.valid_state
    else
      ¬x.valid_state
  ⦄
:= by
  vcgen [ImGuiIO.EndDropdown] <;> try grind

def DropDownWidget
  (io : Type → Type) [Monad io]
  (ctx_t : Type) [ImGuiIO io ctx_t]
  (label : String)
  (preview : String)
  (content : StateT ctx_t io Unit)
:
  StateT ctx_t io Unit
:= do
  if ←(ImGuiIO.BeginDropdown label preview) then
    content
    ImGuiIO.EndDropdown
  else
    pure ()

def temp : ImGuiContext := {
  dropdown_started := False
  mouse_over_dropdown := False
  mouse_over_selection := False
  valid_state := False
}

abbrev content_spec (content : StateT ImGuiContext Id Unit)
  :=
  ∀ w : ImGuiContext,
  ⦃fun (x : ImGuiContext) =>  x = w ⦄
  content
  ⦃fun _ (x : ImGuiContext) => x = w   ⦄

@[spec]
theorem DropDownWidget_spec
  (label : String)
  (preview : String)
  (content : StateT ImGuiContext Id Unit)
  (h : content_spec content)
:
  ⦃fun (x : ImGuiContext) => ⌜ x.valid_state ∧ ¬x.dropdown_started ⌝ ⦄
  (DropDownWidget Id ImGuiContext label preview content)
  ⦃fun _ (x : ImGuiContext) => ⌜ x.valid_state ∧ ¬x.dropdown_started ⌝  ⦄
:= by
  vcgen [DropDownWidget, h] <;> simp_all


end ImGui
