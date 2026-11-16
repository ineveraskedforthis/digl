import Digl.Token


namespace Script

inductive Sign where
| equality : Sign
| curly_bracket_start : Sign
| curly_bracket_stop : Sign
| word : String → Sign

def Sign.fromRawToken (t : RawToken) : Option Sign :=
match t with
| RawToken.Comment _ => none
| RawToken.Signifier s =>
  if s = "=" then
    Sign.equality
  else if s = "{" then
    Sign.curly_bracket_start
  else if s = "}" then
    Sign.curly_bracket_stop
  else
    Sign.word s.toLower

def SignArrayfromRawTokenArray (t : Subarray RawToken) (acc : Array Sign) : Array Sign :=
  let measure := t.size
  if h : measure ≤ 0 then
    acc
  else
    let current := Sign.fromRawToken (t.get ⟨0, by apply Nat.lt_of_not_le h ⟩ )
    match current with
    | none => SignArrayfromRawTokenArray (t.drop 1) (acc)
    | some s => SignArrayfromRawTokenArray (t.drop 1) (acc.push s)
  termination_by t.size

inductive Tree where
| error : Tree
| incomplete : Sign → Tree
| singleton : String → Tree
| assign : Tree → Tree → Tree
| gather : List Tree → Tree

mutual

def  Tree.ListToString (l : List Tree) (indent : String) : String :=
  match l with
  | [] => indent ++ "]"
  | head :: tail => Tree.toString head (indent ++ "\t") ++ (Tree.ListToString tail indent)

def Tree.toString (t : Tree) (indent : String) :=
  match t with
  | Tree.error => "ERROR!\n"
  | Tree.gather l =>
    indent ++ "[\n" ++  (Tree.ListToString l indent) ++ "\n"
  | Tree.assign a b =>
    indent ++ "ASSIGN\n" ++ (Tree.toString a (indent ++ "\t")) ++ indent ++  " WITH \n" ++ (Tree.toString b (indent ++ "\t")) ++ "\n"
  | Tree.incomplete _ =>
    indent ++ "INCOMPLETE\n"
  | Tree.singleton s =>
    indent ++ s ++ "\n"
end


instance Tree.toString_inst : ToString Tree where
  toString t := Tree.toString t ""

def Gather (stack : List Tree) (acc : List Tree) : List Tree :=
  match stack with
  | Tree.incomplete Sign.curly_bracket_start :: tail => (Tree.gather acc) :: tail
  | a :: tail => Gather tail (a :: acc)
  | [] => [Tree.gather acc]

def ResolveAssignment (stack : List Tree) : List Tree :=
  match stack with
  | rhs :: Tree.incomplete Sign.equality :: lhs :: before =>
    Tree.assign lhs rhs :: before
  | a => a

def SignArrayWalk (cursor : Subarray Sign) (stack : List Tree) : Tree :=
  let measure := cursor.size
  if h₀ : measure ≤ 0 then
    Tree.gather stack
  else
    let current := cursor.get ⟨0, by omega⟩
    match current with
    | Sign.curly_bracket_start =>
      SignArrayWalk (cursor.drop 1) (Tree.incomplete Sign.curly_bracket_start :: stack)
    | Sign.curly_bracket_stop =>
      SignArrayWalk (cursor.drop 1) (ResolveAssignment (Gather stack []))
    | Sign.equality =>
      SignArrayWalk (cursor.drop 1) (Tree.incomplete Sign.equality :: stack)
    | Sign.word s =>
      SignArrayWalk (cursor.drop 1) (ResolveAssignment (Tree.singleton s :: stack))
termination_by cursor.size

end Script

namespace V2_GFX

-- inductive Token where
-- | name : Token
-- | texturefile : Token
-- | frames : Token
-- | bracket_open : Token
-- | bracket_close : Token
-- | assign : Token
-- | unknown : String → Token

-- inductive LHS_value where
-- | name : LHS_value
-- | texture : LHS_value
-- | sprite_type : LHS_value

-- def Token.fromString (s : String) : Token :=
--   if s = "name" then
--     Token.name
--   else if s = "texturefile" then
--     Token.texturefile
--   else if s = "noOfFrames" then
--     Token.frames
--   else if s = "{" then
--     Token.bracket_open
--   else if s = "}" then
--     Token.bracket_close
--   else
--     Token.unknown s

structure SpriteType where
  name : Option String
  texture_path : Option System.FilePath
  frames : Option Nat

instance : ToString SpriteType where
  toString s := match s.name with
  | none => "Unknown"
  | some x => x

def ParseScript (seq : Array RawToken) : Script.Tree :=
  let temp := Script.SignArrayfromRawTokenArray (Array.toSubarray seq) #[]
  Script.SignArrayWalk (Array.toSubarray temp) []

def ParseSpriteField (s : Script.Tree) (sprite : SpriteType) : SpriteType :=
  match s with
  | Script.Tree.assign (Script.Tree.singleton lhs) (Script.Tree.singleton rhs) =>
    if lhs = "name" then
      {sprite with name := rhs}
    else if lhs = "texturefile" then
      {sprite with texture_path := rhs}
    else if lhs = "noofframes" then
      {sprite with frames := String.toNat? rhs}
    else
      sprite
  | _ => sprite

def ParseSprite (s : List Script.Tree) (sprite : SpriteType) : SpriteType :=
  match s with
  | [] => sprite
  | a :: tail => ParseSprite tail (ParseSpriteField a sprite)

def ParseSprites (tree : Script.Tree) (acc : Array SpriteType) : Array SpriteType :=
  match tree with
  | Script.Tree.assign (Script.Tree.singleton lhs) (Script.Tree.gather rhs) =>
    if lhs = "spritetype" then
      acc.push (ParseSprite rhs {
        name := none
        texture_path := none
        frames := none
      })
    else if lhs = "spritetypes" then rhs.foldr ParseSprites acc
    else acc
  | Script.Tree.gather l =>
    l.foldr ParseSprites acc
  | _ => acc


def test := "
A = {
	A_x = 1
}
B = {
	B_x = 1
}
C = {
	C_x = 1
}
D = {
	D_x = 1
}
F = {
	F_x = 1
}
"

-- def test_script := ParseScript (SplitTokens test test.startPos "" #[] false false)

-- #eval test_script

-- #eval (ParseSprites test_script #[]).toList

def ParseString_toTree (s : String) : Script.Tree :=
  ParseScript (SplitTokens s s.startPos "" #[] false false)
def ParseString (s : String) : Array SpriteType :=
  ParseSprites (ParseScript (SplitTokens s s.startPos "" #[] false false)) #[]

-- #eval (ParseString test).toList

-- def C_disk : System.FilePath := "C:"
-- def dir : System.FilePath := C_disk / "V2" / "mod" / "GGGGGGGGGGGGG/GGG" / "interface" / "core.gfx"
-- def content := IO.FS.readFile dir
-- def parsed_as_tree : IO Script.Tree := content.bind (fun x => pure (ParseString_toTree x))
-- def parsed : IO String := (content.bind (fun x => pure (toString (ParseString x))))
-- #eval parsed_as_tree
-- #eval parsed

end V2_GFX
