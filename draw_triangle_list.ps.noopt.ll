;
; Buffer Definitions:
;
; cbuffer $Globals
; {
;
;   [0 x i8] (type annotation not present)
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; $Globals                          cbuffer      NA          NA     CB0   cb4294967295     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%ConstantBuffer = type opaque

@"$Globals" = external constant %ConstantBuffer

; Function Attrs: nounwind
define <4 x float> @psmain(<4 x float> %color) #0 {
entry:
  %color.addr = alloca <4 x float>, align 4, !dx.temp !10
  store <4 x float> %color, <4 x float>* %color.addr, align 4, !tbaa !20
  %0 = load <4 x float>, <4 x float>* %color.addr, align 4, !dbg !23, !tbaa !20 ; line:18 col:10
  ret <4 x float> %0, !dbg !27 ; line:18 col:3
}

attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}
!dx.version = !{!3}
!dx.valver = !{!4}
!dx.shaderModel = !{!5}
!dx.typeAnnotations = !{!6}
!dx.entryPoints = !{!13}
!dx.fnprops = !{!17}
!dx.options = !{!18, !19}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-hlemit", !"hlsl-hlensure"}
!2 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!3 = !{i32 1, i32 0}
!4 = !{i32 1, i32 7}
!5 = !{!"ps", i32 6, i32 0}
!6 = !{i32 1, <4 x float> (<4 x float>)* @psmain, !7}
!7 = !{!8, !11}
!8 = !{i32 1, !9, !10}
!9 = !{i32 4, !"SV_TARGET", i32 7, i32 9}
!10 = !{}
!11 = !{i32 0, !12, !10}
!12 = !{i32 4, !"COLOR", i32 7, i32 9}
!13 = !{<4 x float> (<4 x float>)* @psmain, !"psmain", null, !14, null}
!14 = !{null, null, !15, null}
!15 = !{!16}
!16 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!17 = !{<4 x float> (<4 x float>)* @psmain, i32 0, i1 false}
!18 = !{i32 144}
!19 = !{i32 -1}
!20 = !{!21, !21, i64 0}
!21 = !{!"omnipotent char", !22, i64 0}
!22 = !{!"Simple C/C++ TBAA"}
!23 = !DILocation(line: 18, column: 10, scope: !24)
!24 = !DISubprogram(name: "psmain", scope: !25, file: !25, line: 17, type: !26, isLocal: false, isDefinition: true, scopeLine: 17, flags: DIFlagPrototyped, isOptimized: false, function: <4 x float> (<4 x float>)* @psmain)
!25 = !DIFile(filename: "draw_triangle_list.hlsl", directory: "")
!26 = !DISubroutineType(types: !10)
!27 = !DILocation(line: 18, column: 3, scope: !24)
