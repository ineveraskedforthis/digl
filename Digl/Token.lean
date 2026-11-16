inductive RawTokenType where
| Comment : RawTokenType
| Signifier : RawTokenType

inductive RawToken where
| Comment : String → RawToken
| Signifier : String → RawToken

instance : ToString RawToken where
  toString rt := match rt with
  | RawToken.Comment s => s
  | RawToken.Signifier s => s

def PushToken
  (token : String)
  (token_t : RawTokenType)
  (acc : Array RawToken)
:
  Array RawToken
:=
  if token.length == 0 then acc else
    match token_t with
    | RawTokenType.Signifier => acc.push (RawToken.Signifier token)
    | RawTokenType.Comment => acc.push (RawToken.Comment token)

def SplitTokens
  (s : String)
  (current_position : String.Pos s)
  (acc_token : String)
  (acc : Array RawToken)
  (reading_name : Bool)
  (reading_comment : Bool)
:
  Array RawToken
:=
  if h' : current_position < s.endPos then
    have h : current_position ≠ s.endPos := by exact String.Pos.ne_endPos_of_lt h'
    let current_character := current_position.get h
    if reading_comment then
      if current_character == '\n' ∨ current_character == '\r' then
        SplitTokens s (current_position.next h) "" (PushToken acc_token RawTokenType.Comment acc) false false
      else
        SplitTokens s (current_position.next h) (acc_token.push current_character) acc false true
    else
      if current_character == '#' then
        SplitTokens s (current_position.next h) "#" (PushToken acc_token RawTokenType.Signifier acc) false true
      else
        if current_character == '"' then
          if reading_name then
              SplitTokens s (current_position.next h) "" (PushToken acc_token RawTokenType.Signifier acc) false false
          else
              SplitTokens s (current_position.next h) "" (PushToken acc_token RawTokenType.Signifier acc) true false
        else
          if reading_name then
            SplitTokens s (current_position.next h) (acc_token.push current_character) (acc) reading_name false
          else
            if current_character = '=' ∨ current_character = '{' ∨ current_character = '}' then
              SplitTokens s (current_position.next h) "" ( PushToken current_character.toString RawTokenType.Signifier (PushToken acc_token RawTokenType.Signifier acc)) reading_name false
            else
              if current_character.isWhitespace then
                SplitTokens s (current_position.next h) "" (PushToken acc_token RawTokenType.Signifier acc) reading_name false
              else
                SplitTokens s (current_position.next h) (acc_token.push current_character) acc reading_name false
  else
    if reading_comment then
      PushToken acc_token RawTokenType.Comment acc
    else
      PushToken acc_token RawTokenType.Signifier acc
  termination_by String.Pos.Raw.byteDistance current_position.offset s.rawEndPos
  decreasing_by
  repeat (
    rw [String.Pos.offset_next]
    rw [String.Pos.Raw.byteDistance_eq]
    rw [String.Pos.Raw.byteIdx_add_char]
    rw [String.Pos.Raw.byteDistance_eq]
    apply Nat.sub_lt_sub_left
    exact String.Pos.Raw.lt_iff.mp h'
    have char_gt_zero (c : Char) : 0 < c.utf8Size := by
      have temp := c.utf8Size_eq
      grind
    apply Nat.lt_add_of_pos_right
    apply char_gt_zero
  )
