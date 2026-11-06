# AGENTS.md

Valentina is a Qt 5 and Qt 6 patternmaking suite (Valentina, Tape, and Puzzle) built with Qbs. These notes are for anyone changing the tree, including coding agents. Platform build steps live in the [wiki](https://gitlab.com/smart-pattern/valentina/-/wikis/home). The job steps in `.github/workflows/` are the definition of each CI configuration. When a mirror job fails and you have that compiler and Qt, reproduce the job with those. Day to day, build with the toolchain you have.

## Keep this file current

When a failed build, a review, or a bug teaches a constraint the next contributor would hit, add it here in the same change. State the failure mode and how to apply it. Name the file or the function. Leave out line numbers and "fixed in" commits. Those rot, and the failure mode does not.

When a note here stops being true, rewrite or delete it in the change that made it stale. Private notes for one organization do not belong in this file. It is published with the source.

## Where changes land

The canonical repository is GitLab `smart-pattern/valentina`. GitHub `dismine/valentina` is a push mirror that GitLab force-syncs. Push to GitLab. A merge that exists only on the GitHub mirror is discarded on the next sync.

`master` holds major releases. `develop` is the next release, and it is the branch CI tests. `vZ.Y.x` branches hold bugfixes for a stable release. History on these branches is linear. Bring a change in by cherry-pick, then push to GitLab.

Some commit hooks append a `Branch:` trailer and append another one when the message already has one. After a squash or amend, `git log -1 --format=%B` should show a single `Branch:` line, naming the branch the commit sits on.

## CI is the acceptance gate

Workflows run on the GitHub mirror:

- `linux.yml` on pushes and pull requests to `develop`
- `windows.yml` and `macos.yml` on pushes and pull requests to `develop` and `master`

A source change builds Linux Qt 5 and Qt 6 (GCC and Clang), an AppImage, Sonar, two Windows Qt 6 variants, and several macOS architectures and Qt versions. macOS wall time is about two hours when those jobs are healthy. Linux and Windows usually finish within the hour. A path filter skips the build jobs when the change does not touch sources, the workflow file, or `scripts/upload_symbols.py`.

Push `develop` when the change is ready for that gate. Build and run the tests you can with the compilers you have. A follow-up push whose only purpose is to read an error you could have produced locally repeats the whole matrix.

Most contributors have one Qt and one operating system. A configuration you cannot build is proved only by the mirror job for that configuration. Say which jobs you could not run. A green build on the Qt you have says nothing about the others.

What each missing environment is the only proof of:

- Qt 5 has no `QList` sized constructors (`QList<T>(count)`, `QList<T>(count, value)`) and no `QStringList::fill`. Build the list with `reserve` and `append`. `_L1` comes from `src/libs/vmisc/compatibility.h`. Include that header when `QT_VERSION < 6.4.0`. A guard of `< 6.0.0` still breaks the AppImage job.
- The AppImage image carries an older Qt 6 than the distro Qt 6 jobs. `Qt::Literals` is not a namespace on that image (the suggestion `QtLiterals` is not a substitute), and `_L1` still comes from the compatibility header.
- Clang Qt 5 builds with `-Werror,-Wsign-conversion`. `QFlags | Qt::Key` is an unsigned `QFlags::Int`. Passing that to `QKeySequence(int)` fails `vvalentinashortcutmanager.h`. On Qt 5 combine `static_cast<int>(modifiers) | static_cast<int>(key)`. On Qt 6 construct a `QKeyCombination`.
- MSVC includes `windows.h`, which defines `far` and `near` as macros. A local with either name is a syntax error on Windows only. CI runs `valentinaTest` with `-silent` and `QT_QPA_PLATFORM=offscreen`. QTest writes to `OutputDebugString` when stderr is not a console. The autotest runner replaces the process environment (`qbs.commonRunEnvironment` is empty), so `QT_FORCE_STDERR_LOGGING` set only on the workflow step never reaches `valentinaTest`. The variable has to be on the runner's environment list in `valentina.qbs`. Until it is, the log shows only `exit code 1` after a full suite. `parserTest` is not QTest, so its output next to a silent `valentinaTest` failure is the tell. A test that calls `show()` and then asserts `isVisible()` depends on the platform window. On Linux offscreen, `show()` does make `isVisible()` true, and `isHidden()` is already false after `setVisible(true)` with no `show()`. `LengthShortcutBarFollowsTheEditor` was the only test that showed a widget. Assert `isHidden()` after `setVisible`. `QToolButton::click()` and `QAction::trigger()` do not need the widget to be shown. Where you have a Windows Qt 6 build, run `valentinaTest` with no `-silent`. One data row is `valentinaTest 'TestName:<data tag>'`, with no class prefix. With no Windows machine, that job's log is the result.
- After cherry-picking a workflow, grep the destination matrix for every literal the commit branches on (macOS target names, Qt versions). A condition that matched on the source branch can match nothing after the matrix has moved, and the job still goes green. Reading the workflow file needs no extra toolchain.
- Signing, notarization, and symbol-upload secrets live on the mirror. Those steps have no local equivalent.

Read the Linux and Windows logs when those jobs finish. A macOS result arrives later and is a separate fact.

## CPU baseline

Keep generated code at the Qt 5.15 baseline: generic x86-64, SSE2. POPCNT, SSE4.2, and AVX wait for an explicit maintainer decision to raise that baseline. An MSVC intrinsic such as `__popcnt` ignores `/arch` and emits the opcode directly.

Classic simple nesting is the layout mode that keeps running on this baseline. Modern nesting implementations are likely to need a newer instruction set. They are an additional engine for machines that can run them. They leave classic simple nesting in place.

immer 0.9.1 hardcodes `#define IMMER_HAS_BUILTIN_POPCOUNT 1` with no `#ifndef`, so a compiler `-D` cannot override it. On MSVC that becomes `__popcnt` with no CPUID check, and the first insert into a `VContainer` immer map illegal-instructions any CPU older than Nehalem. `scripts/patch_immer_popcount.py` rewrites the cached header after `conan install` on Windows. If `conanfile.py` bumps immer, re-read the script. It fails when the line it expects is gone. The upstream report is [immer issue 327](https://github.com/arximboldi/immer/issues/327).

## Style

Format C++ with `clang-format-20`. The tree is not fully clean under that version. Format the lines you changed, or match the surrounding style by hand. Running `clang-format-20 -i` on a whole existing file reflows unrelated code. A new file you authored can be formatted in full.

For Sonar `cpp:S5350`, write `const T *p` when every use through the pointer is const. `T *const p` does not satisfy the rule.

## Pattern file format

A new element or attribute needs a new XSD version and a converter step, including on `develop` before a release. Copy the current max schema forward. `PatternMaxVer` is 1.2.1 in `src/libs/ifc/xml/vpatternconverter.h`. Add the new schema to `src/libs/ifc/schema.qrc`, bump `PatternMaxVer`, `PatternMaxVerStr`, and `CurrentSchema`, register it in `XSDSchemas()` and `ApplyPatches()`, and update the `IsReadOnly` static assert. Bump test `.val` files that use the new markup. `TST_XSDShema` picks the new schema up on its own. Editing the current max XSD in place ships a format change with no version.

`VFormula` stores a raw `VContainer *` and does not copy the container. A helper that points it at a stack `VContainer` dangles after the call returns. Transient variables such as `CurrentLength` have to live in the tool's own `data` for the property-browser formula editor. The immer migration moved `RemoveVariable("CurrentLength")` earlier in `VToolCut::Create` and `VToolAlongLine::Create`, which dropped that name from the tool snapshot the editor reads. When a formula used to work, run `git log -S` on the line that mutates the shared container, not only on the function that crashes. A moved call keeps the same text, so a log of the crashing function alone hides it.

## Piece geometry and classic simple nesting

`VToolSeamAllowance::ComputePieceGeometry` and the `Render*` helpers evaluate formulas through the shared `VContainer`. qmuparser evaluation is not thread-safe across pieces, because it mutates `QSharedData` refcounts on objects the pieces share. Recompute pieces one after another on the GUI thread. A cross-piece `QtConcurrent::map` has corrupted the heap on multi-piece patterns, and only in release CI.

Classic simple nesting is the in-process layout: `VLayoutGenerator::Generate` and `VPosition::ArrangeDetail`, used when "Nest with the chosen engine" is off. A pluggable external engine or a remote nesting API goes through `GenerateLayoutOnline` and is a separate path. The thread-pool rule here is about classic simple nesting only.

That mode stays, so a machine on the CPU baseline still has a layout. The reason is in the CPU baseline section.

Classic simple nesting nests `QtConcurrent`. `GenerateLayout` runs `VLayoutGenerator::Generate` on the global pool, and `VPosition::ArrangeDetail` then starts a `QtConcurrent::mapped`. The inner `mapped` uses the dedicated `nestingThreadPool` in `src/libs/vlayout/vposition.cpp`. Blocking a global-pool worker on more global-pool work deadlocks when `QThread::idealThreadCount()` is 1: the process sits at 0% CPU with one pooled thread. `QtConcurrent::run(...).result()` can run the work on the waiting thread. `QFuture::waitForFinished()` on a `mapped` future does not. Reproduce with `taskset -c 0`.

Prove a seam-allowance or piece-outline change with a headless export, then assert a number:

```
valentina <file>.val --basename X --destination <dir> --format 0 --exportOnlyDetails
```

`--exportOnlyDetails` writes the Details-tab geometry before any layout pass, classic simple nesting included. In a `fill-rule="nonzero"` path, subpath 1 is the seam line and subpath 2 is the seam allowance. A screenshot hides a single bad vertex.

A graduated curve is one curve whose offset width varies along its length. A parallel curve is one curve at one constant distance.

## Operations, names, and undo

Move, rotate, and flip connect `NeedFullParsing` with `Qt::QueuedConnection` so the reparse does not delete the tool while Apply is still on the stack. The queued full parse clears the scene and recreates the tool from XML. The options dialog stays open, parented to the main window, and its connections to the old tool are dead. A second Apply does nothing.

Apply stays enabled only when the source object set is unchanged, so Apply takes the lite-parse path and the dialog survives. Adding or removing a source object is committed with OK, which closes the dialog before the full parse. Compare record ids, the same key `SyncDestination` uses. Recreating the preview on the orphaned dialog leaks it, and Apply is already dead.

Default names for curves have to be uniqueness-checked with the type-head prefix (`Spl_` and the other heads from `GetTypeHead()`). `data->IsUnique()` on the bare `base__suffix` string returns true, because the registered name carries the prefix. `FillDefSourceNames()` and `GetDefSourceName()` in `src/libs/vtools/tools/toolsdef.cpp` go through `GetSourceItemName()`, which `IsValidSourceName()` already did. Any new default-name generator does the same. `VContainer::AddGObject` accepts a duplicate name. The dialogs enforce uniqueness.

The duplicate-name warning fires only when the object being registered is `Draw::Calculation`. A modeling object is a copy of a calculation object and keeps its name on purpose. Formulas never resolve a name to a modeling object. Several modeling tools never call `setIdObject()`, so a walk back through `idObject` will not find the legitimate copies. If the warning fires again, check the `Draw::Calculation` gate.

A new per-node flag on `VPieceNode` needs a term in the node loop of both `SavePieceOptions::mergeWith` (`src/libs/vtools/undocommands/savepieceoptions.cpp`) and `SavePiecePathOptions::mergeWith` (`savepiecepathoptions.cpp`). `mergeWith` folds consecutive edits of the same piece into one undo step, and it refuses to merge when a flag differs. Leave the new flag out and its toggle is swallowed by the previous command. The file still compiles.

On a type that is both a `VAbstractTool` and a `QGraphicsItem`, call `QGraphicsItem::setData`. `VDataTool::setData(const VContainer *)` hides `QGraphicsItem::setData(int, const QVariant &)`. Child items that are not a `VDataTool` can call `setData` unqualified.

`VApplication::notify`, `MApplication::notify`, and `VPApplication::notify` terminate fatal exceptions through `TerminateOnFatalException()`, which calls `::exit`. Inside a `QCoreApplication` subclass, an unqualified `exit()` resolves to `QCoreApplication::exit()`, which flags the event loop to quit and returns. Execution continues on data already known to be bad, and the crash shows up later in teardown. A critical line that says the program will be terminated, followed by more log lines, means the process did not exit. Grep for an unqualified `exit(` in any `QCoreApplication` subclass before chasing the later crash.

`VAbstractPattern::ChangeNamePP()` renames the piece element and then rewrites every history record whose `nameDraw` is the old name. `getLocalHistory()` otherwise returns nothing for the renamed piece.

## Tape, themes, and translations

An image-title edit in Tape refreshes the Images tab through `ShowImageData()`. `ShowMData()` refreshes the Measurements tab. The diagram widget is shared, so the wrong call snaps the preview to whatever row is selected on Measurements. Open a known-measurements file with `tape --known file.vkm`. `tape file.vkm` tries to open it as `.vit` or `.vst` and reports an unknown format.

On Linux, `VTheme::IsInDarkTheme()` compares WindowText and Window lightness. When that heuristic already says dark, `InitThemeMode()` skips Valentina's own stylesheet and leaves `QPalette::PlaceholderText` unset. Read-only fields then draw near-black placeholder text. `SetPlaceholderTextPalette` runs at the end of `InitThemeMode()` on Linux so that branch is covered. A contrast bug that shows up only on Linux is this heuristic before it is a stylesheet bug.

`tx push -l` and `tx pull -l` take the local file suffix (`hr_HR`, `es_ES`), which is the right-hand side of `lang_map` in `.tx/config`. They do not take the Transifex remote code (`es`, `cs`, `nl`). `tx pull` replaces the `.ts` file with Transifex's copy, and that copy has no `type="vanished"` entries, so every vanished message for that language disappears. A small translation batch can show tens of thousands of diff lines from those blocks. Compare vanished counts against `HEAD` before treating the pull as corrupt. Active translations are still in the file.

## Clicking from a screenshot

A window capture is in device pixels. A viewer often displays that image scaled down. Multiply click coordinates by capture width divided by displayed width before sending them to the window. Check the capture size against the real window geometry when the two might already match.
