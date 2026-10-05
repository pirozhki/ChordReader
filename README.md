# ChordReader

A small, dependency-free C++17 helper library for reading chord symbols and converting them into structured chord data that can be used by a chord player or other music software.

ChordReader is **not a chord player itself**. Its main purpose is to take practical chord-chart text such as `F7/Bb` or `DbM9` and turn it into a form that a playback program can use to determine the chord tones and bass note.

## Features

- Parse common chord symbols such as `Cm7`, `BbM7`, `C7(b9)`, `Fm7-5(11)`, and slash chords.
- Handle extensions and alterations including `6`, `7`, `9`, `11`, `13`, `b5`, `b9`, `#9`, `#11`, `b13`, `add9`, `sus4`, and `aug`.
- Handle diminished chords (`dim`, `dim7`) and power chords (`5`).
- Recognize `N.C.` (no chord) explicitly.
- Keep the root, chord quality, tensions, and slash-bass information available for use by a chord player.
- Extract the chord's 12 pitch classes from the internal constituent representation.
- Provide key estimation and transposition as optional utility functions.
- Format notes using either sharps or flats.
- Provide a simple `ChordManager` for parsing a single chord or a whitespace-separated text block.
- No external libraries are required.

The refactored implementation intentionally retains the original 24-bit constituent representation so the core design remains compatible with the original project while providing a cleaner C++17 API.

## Supported chord notation

Examples include:

```text
C
Cm
Cm7
CM7
C7
C7(b9)
C7(#11)
C9
C11
C13
CM9
CM11
CM13
Cadd9
C(#11)
Csus4
Caug
Cdim
Cmb5
Cdim7
C5
Am7-5
Cm7-5(11)
D7/G
Fm7/Bb
Bb7sus4/G
N.C.
```

### Extension and parenthesized-addition rules

ChordReader distinguishes between a bare extension and a parenthesized added tone:

| Input | Interpretation | Canonical output |
| --- | --- | --- |
| `C9` | C7 + 9th | `C7(9)` |
| `C11` | C7 + 9th + 11th | `C7(9,11)` |
| `C13` | C7 + 9th + 13th | `C7(9,13)` |
| `CM9` | CM7 + 9th | `CM7(9)` |
| `CM11` | CM7 + 9th + 11th | `CM7(9,11)` |
| `CM13` | CM7 + 9th + 13th | `CM7(9,13)` |
| `C(9)` | C + added 9th; no automatic 7th | `Cadd9` |
| `C(11)` | C + added 11th; no automatic 7th | `C(11)` |
| `C(#11)` | C + added #11; no automatic 7th | `C(#11)` |
| `C(13)` | C + added 13th; no automatic 7th | `C(13)` |
| `C7(#11)` | C7 + #11 | `C7(#11)` |

The same principle applies when an altered extension is written inside parentheses: `C(b9)`, `C(#9)`, and `C(b13)` add only the specified altered tone and do not introduce a 7th by themselves. An explicitly written seventh, as in `C7(b9)` or `C7(#9)`, is retained.

Bare `11` and `13` use the conventional stacked-extension interpretation: `C11` includes a 7th, 9th, and 11th, while `C13` includes a 7th, 9th, and 13th. The 11th is not automatically added to `C13` unless it is explicitly written.

### Diminished and flat-fifth notation

`dim` and `mb5` are accepted as diminished-triad spellings with the same pitch content, and the canonical output uses `dim`. A diminished seventh remains explicitly `dim7`.

| Input | Interpretation | Canonical output |
| --- | --- | --- |
| `Cdim` | C diminished triad | `Cdim` |
| `Cmb5` | C minor triad + flat 5th | `Cdim` |
| `Cm7b5` | C minor 7th + flat 5th | `Cm7b5` |
| `Cdim7` | C diminished 7th | `Cdim7` |

A flat fifth is treated as part of the chord quality and is formatted outside the tension parentheses. For example, `Cm7-5(11)` and `Cm7(b5,11)` are both normalized to `Cm7b5(11)`.

`M9` is treated as a major 7th plus a 9th, so `DbM9` is parsed as `DbM7(9)` rather than `Db7(9)`. Likewise, `CM11` becomes `CM7(9,11)` and `CM13` becomes `CM7(9,13)`.

### Slash bass

The part after `/` is treated as a **bass note**, not as a tension. For example:

```text
D#aug/F
```

means an augmented chord rooted on D# with F as the bass note. The bass specification is kept separate from the chord quality so that a chord player can use it when constructing the voicing.

This also applies to forms such as:

```text
F7/Bb
C7(b9)/E
Daug/Ab
```

The parser is intended for practical chord-chart notation rather than as a full formal music-notation grammar. Whitespace-separated chord symbols are supported by `ChordManager::AddText()`.

## Example

The included `example.cpp` reads chord symbols from `sample.txt`. The sample is intentionally fairly complex so that the chord parsing can be inspected using realistic chord-chart notation.

`sample.txt` contains:

```text
DbM9 Ebm9 Cm7-5(11) F7 F7/Bb Bbm Abm Db7 Gb Ab/Gb Fm Bbm7 Ebm7 Fm7 GbM7 Bb7sus4/G Gb/Ab Ab Fm7/Gb Ebm7/Ab BbM7 Cm7 Am7-5(11) D7 D7/G Gm Fm Bb7 Eb F/Eb Dm Gm7 Gm7/C Cm7/F
```

Build and run it with:

```bash
cmake -S . -B build
cmake --build build --config Release
./build/ChordReader
```

On Windows with a Visual Studio generator, run the executable from the corresponding `Release` directory.

The example demonstrates the same information that a chord player can consume from the parsed progression. It also shows the optional key-estimation and transposition helpers.

For the sample above, the example program produces:

```text
Parsed 34 chord(s).
Estimated key: Db major
Transposed to C major:
CM7(9) Dm7(9) Bm7b5(11) E7 E7/A Am Gm C7 F G/F Em Am7 Dm7 Em7 FM7 A7sus4/Gb F/G G Em7/F Dm7/G AM7 Bm7 Abm7b5(11) Db7 Db7/Gb Gbm Em A7 D E/D Dbm Gbm7 Gbm7/B Bm7/E
```

With no argument, the program reads `sample.txt` from the current working directory. A different chord file can be supplied as the first argument:

```bash
./build/ChordReader path/to/chords.txt
```

## Library usage

The main use case is to parse chord symbols and pass the resulting `ChordData` to another component such as a chord player:

```cpp
#include "chord.h"
#include <iostream>

using namespace chordreader;

int main() {
    const ChordData chord = ParseChord("F7/Bb");
    const auto notes = GetChordNotes(chord);

    // `chord` contains the root, chord information, and bass note.
    // A chord player can use that data to construct its own voicing.
    std::cout << FormatChord(chord, NoteNameStyle::Flats) << '\n';
    return 0;
}
```

For a chord progression:

```cpp
ChordManager manager;
manager.AddText("DbM9 Ebm9 Cm7-5(11) F7 F7/Bb Bbm Abm Db7 Gb");

for (const auto& chord : manager.GetChords()) {
    // Pass `chord` to the playback layer.
}
```

For input that may contain invalid symbols, `AddText()` can collect them without aborting the whole parse:

```cpp
std::vector<std::string> errors;
const std::size_t added = manager.AddText(text, &errors);
```

`TryAddChord()` is available when exception-free control flow is preferred.

## Build

The project uses CMake and requires C++17 or later.

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure -C Release
```

## Design notes

`ChordData` stores the root, bass, no-chord state, and the original 24-position interval representation. `GetChordNotes()` converts that representation into the corresponding 12 pitch classes.

`ChordManager` owns a progression and provides parsing, iteration, optional key estimation, optional transposition, and formatted output. `ParseChord()` and `FormatChord()` are separate so parsing and presentation can evolve independently.

The intended architecture is:

```text
Chord chart / text input
        |
        v
   ChordReader
        |
        v
    ChordData
        |
        v
   Chord player / playback engine
```

ChordReader handles parsing and chord representation; sound generation, voicing, timing, MIDI, and audio playback are left to the application using the library.

## Limitations

Chord notation is intentionally treated as a practical symbol format. Some highly specialized notation, compound textual expressions, or context-dependent shorthand may require preprocessing before being passed to the parser.

## License

This project is released under the MIT License. See `LICENSE` for the full license text.
