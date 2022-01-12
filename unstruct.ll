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
define float @main(<4 x float> %pos) #0 {
entry:
  %retval = alloca float, align 4, !dx.temp !10
  %pos.addr = alloca <4 x float>, align 4, !dx.temp !10
  %i = alloca i32, align 4
  store <4 x float> %pos, <4 x float>* %pos.addr, align 4, !tbaa !20
  %0 = load <4 x float>, <4 x float>* %pos.addr, align 4, !dbg !23 ; line:2 col:7
  %1 = extractelement <4 x float> %0, i32 0, !dbg !23 ; line:2 col:7
  %cmp = fcmp olt float %1, 1.000000e+00, !dbg !27 ; line:2 col:13
  %tobool = icmp ne i1 %cmp, false, !dbg !27 ; line:2 col:13
  %tobool1 = icmp ne i1 %tobool, false, !dbg !27 ; line:2 col:13
  br i1 %tobool1, label %if.then, label %if.end, !dbg !23 ; line:2 col:7

if.then:                                          ; preds = %entry
  %2 = load <4 x float>, <4 x float>* %pos.addr, align 4, !dbg !28 ; line:3 col:12
  %3 = extractelement <4 x float> %2, i32 0, !dbg !28 ; line:3 col:12
  store float %3, float* %retval, !dbg !29 ; line:3 col:5
  br label %return, !dbg !29 ; line:3 col:5

if.end:                                           ; preds = %entry
  store i32 0, i32* %i, align 4, !dbg !30, !tbaa !31 ; line:5 col:12
  br label %for.cond, !dbg !33 ; line:5 col:8

for.cond:                                         ; preds = %for.inc, %if.end
  %4 = load i32, i32* %i, align 4, !dbg !34, !tbaa !31 ; line:5 col:19
  %conv = sitofp i32 %4 to float, !dbg !34 ; line:5 col:19
  %5 = load <4 x float>, <4 x float>* %pos.addr, align 4, !dbg !35 ; line:5 col:23
  %6 = extractelement <4 x float> %5, i32 1, !dbg !35 ; line:5 col:23
  %cmp2 = fcmp olt float %conv, %6, !dbg !36 ; line:5 col:21
  %tobool3 = icmp ne i1 %cmp2, false, !dbg !36 ; line:5 col:21
  %tobool4 = icmp ne i1 %tobool3, false, !dbg !37 ; line:5 col:3
  br i1 %tobool4, label %for.body, label %for.end, !dbg !37 ; line:5 col:3

for.body:                                         ; preds = %for.cond
  %7 = load i32, i32* %i, align 4, !dbg !38, !tbaa !31 ; line:6 col:9
  %conv5 = sitofp i32 %7 to float, !dbg !38 ; line:6 col:9
  %8 = load <4 x float>, <4 x float>* %pos.addr, align 4, !dbg !39 ; line:6 col:13
  %9 = extractelement <4 x float> %8, i32 2, !dbg !39 ; line:6 col:13
  %cmp6 = fcmp ogt float %conv5, %9, !dbg !40 ; line:6 col:11
  %tobool7 = icmp ne i1 %cmp6, false, !dbg !40 ; line:6 col:11
  %tobool8 = icmp ne i1 %tobool7, false, !dbg !40 ; line:6 col:11
  br i1 %tobool8, label %if.then.9, label %if.else, !dbg !38 ; line:6 col:9

if.then.9:                                        ; preds = %for.body
  br label %for.end, !dbg !41 ; line:7 col:7

if.else:                                          ; preds = %for.body
  %10 = load i32, i32* %i, align 4, !dbg !42, !tbaa !31 ; line:8 col:14
  %cmp10 = icmp sgt i32 %10, 1000, !dbg !43 ; line:8 col:16
  %tobool11 = icmp ne i1 %cmp10, false, !dbg !43 ; line:8 col:16
  %tobool12 = icmp ne i1 %tobool11, false, !dbg !43 ; line:8 col:16
  br i1 %tobool12, label %if.then.13, label %if.end.15, !dbg !42 ; line:8 col:14

if.then.13:                                       ; preds = %if.else
  %11 = load i32, i32* %i, align 4, !dbg !44, !tbaa !31 ; line:9 col:14
  %conv14 = sitofp i32 %11 to float, !dbg !44 ; line:9 col:14
  store float %conv14, float* %retval, !dbg !45 ; line:9 col:7
  br label %return, !dbg !45 ; line:9 col:7

if.end.15:                                        ; preds = %if.else
  br label %if.end.16

if.end.16:                                        ; preds = %if.end.15
  br label %for.inc, !dbg !46 ; line:10 col:3

for.inc:                                          ; preds = %if.end.16
  %12 = load i32, i32* %i, align 4, !dbg !47, !tbaa !31 ; line:5 col:31
  %inc = add nsw i32 %12, 1, !dbg !47 ; line:5 col:31
  store i32 %inc, i32* %i, align 4, !dbg !47, !tbaa !31 ; line:5 col:31
  br label %for.cond, !dbg !37 ; line:5 col:3

for.end:                                          ; preds = %if.then.9, %for.cond
  store float 0.000000e+00, float* %retval, !dbg !48 ; line:12 col:3
  br label %return, !dbg !48 ; line:12 col:3

return:                                           ; preds = %for.end, %if.then.13, %if.then
  %13 = load float, float* %retval, !dbg !49 ; line:13 col:1
  ret float %13, !dbg !49 ; line:13 col:1
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
!5 = !{!"vs", i32 6, i32 0}
!6 = !{i32 1, float (<4 x float>)* @main, !7}
!7 = !{!8, !11}
!8 = !{i32 1, !9, !10}
!9 = !{i32 7, i32 9}
!10 = !{}
!11 = !{i32 0, !12, !10}
!12 = !{i32 4, !"POSITION", i32 7, i32 9}
!13 = !{float (<4 x float>)* @main, !"main", null, !14, null}
!14 = !{null, null, !15, null}
!15 = !{!16}
!16 = !{i32 0, %ConstantBuffer* @"$Globals", !"$Globals", i32 0, i32 -1, i32 1, i32 0, null}
!17 = !{float (<4 x float>)* @main, i32 1}
!18 = !{i32 144}
!19 = !{i32 -1}
!20 = !{!21, !21, i64 0}
!21 = !{!"omnipotent char", !22, i64 0}
!22 = !{!"Simple C/C++ TBAA"}
!23 = !DILocation(line: 2, column: 7, scope: !24)
!24 = !DISubprogram(name: "main", scope: !25, file: !25, line: 1, type: !26, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, function: float (<4 x float>)* @main)
!25 = !DIFile(filename: "unstruct.hlsl", directory: "")
!26 = !DISubroutineType(types: !10)
!27 = !DILocation(line: 2, column: 13, scope: !24)
!28 = !DILocation(line: 3, column: 12, scope: !24)
!29 = !DILocation(line: 3, column: 5, scope: !24)
!30 = !DILocation(line: 5, column: 12, scope: !24)
!31 = !{!32, !32, i64 0}
!32 = !{!"int", !21, i64 0}
!33 = !DILocation(line: 5, column: 8, scope: !24)
!34 = !DILocation(line: 5, column: 19, scope: !24)
!35 = !DILocation(line: 5, column: 23, scope: !24)
!36 = !DILocation(line: 5, column: 21, scope: !24)
!37 = !DILocation(line: 5, column: 3, scope: !24)
!38 = !DILocation(line: 6, column: 9, scope: !24)
!39 = !DILocation(line: 6, column: 13, scope: !24)
!40 = !DILocation(line: 6, column: 11, scope: !24)
!41 = !DILocation(line: 7, column: 7, scope: !24)
!42 = !DILocation(line: 8, column: 14, scope: !24)
!43 = !DILocation(line: 8, column: 16, scope: !24)
!44 = !DILocation(line: 9, column: 14, scope: !24)
!45 = !DILocation(line: 9, column: 7, scope: !24)
!46 = !DILocation(line: 10, column: 3, scope: !24)
!47 = !DILocation(line: 5, column: 31, scope: !24)
!48 = !DILocation(line: 12, column: 3, scope: !24)
!49 = !DILocation(line: 13, column: 1, scope: !24)
