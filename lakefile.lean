import Lake

open System Lake DSL


package digl where
  version := v!"0.1.0"
  moreLinkArgs := #[
    "-L./build",
    "-lDIGL_libraries"
  ]

-- extern_lib DIGL_libraries pkg := do
--   let libPath := pkg.dir / "build" / "libDIGL_libraries.a"
--   return Job.pure libPath

lean_lib Digl

@[default_target] lean_exe digl where
  root := `Main
  supportInterpreter := true
