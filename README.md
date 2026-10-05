# ChordReader

A small, dependency-free C++17 library for reading chord symbols, extracting their pitch classes, estimating a major key, and transposing chord progressions.

## Features

- Parse common chord symbols such as `Cm7`, `BbM7`, `C7(b9)`, `Fm7-5(11)`, and slash chords.
- Handle extensions and alterations including `6`, `7`, `9`, `11`, `13`, `b5`, `b9`, `#9`, `#11`, `b13`, `add9`, `sus4`, and `aug`.
- Handle diminished chords (`dim`, `dim7`) and power chords (`5`).
- Recognize `N.C.` (no chord) explicitly.
- Extract the chord's 12 pitch classes from the original 24-position constituent representation.
- Estimate a major key from the stored chord tones.
- Transpose an entire progression without requiring callers to manipulate root and bass notes directly.
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

The same principle applies when an altered extension is written inside parentheses: `C(b9)`, `C(#9)`, and `C(b13)` add only the specified altered tone and do not introduce a 7th by themselves. An explicitly written seventh, as in `C7(b9)` or `C7(#9)`, is of course retained.

### Diminished and flat-fifth notation

`dim` and `mb5` are accepted as diminished-triad spellings with the same pitch content, and the canonical output uses `dim`. A diminished seventh remains explicitly `dim7`.

| Input | Interpretation | Canonical output |
| --- | --- | --- |
| `Cdim` | C diminished triad | `Cdim` |
| `Cmb5` | C minor triad + flat 5th | `Cdim` |
| `Cm7b5` | C minor 7th + flat 5th | `Cm7b5` |
| `Cdim7` | C diminished 7th | `Cdim7` |

A flat fifth is treated as part of the chord quality and is formatted outside the tension parentheses. For example, `Cm7-5(11)` and `Cm7(b5,11)` are both normalized to `Cm7b5(11)`.

`M9` is treated as a major 7th plus a 9th, so `DbM9` is parsed as `DbM7(9)` rather than `Db7(9)`. Bare `11` and `13` extensions likewise imply the lower 9th: `C11` is parsed as `C7(9,11)` and `C13` as `C7(9,13)`. Major forms such as `CM11` and `CM13` retain the major 7th and become `CM7(9,11)` and `CM7(9,13)`. Parenthesized additions such as `C7(11)` and `C7(13)` do not add a 9th automatically.

The parser is intended for practical chord-chart notation rather than as a full formal music-notation grammar. Whitespace-separated chord symbols are supported by `ChordManager::AddText()`.

## Example

The included `example.cpp` reads chord symbols from `sample.txt`. The sample is intentionally fairly complex so that parsing, key estimation, and transposition are easy to inspect.

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

The example intentionally uses the canonical output conventions of ChordReader. For example, `Bb7sus4` is formatted as `Bb7sus4`, and a major-9 chord such as `DbM9` is represented as `DbM7(9)` after parsing because `M9` is interpreted as a major 7th plus a 9th. The major-9 chord therefore keeps its major 7th component and is not converted into a dominant `7(9)` chord. Bare `11` and `13` extensions include the natural 9th (`C11` → `C7(9,11)`, `C13` → `C7(9,13)`), while parenthesized additions do not. A flat fifth is formatted outside the tension parentheses, for example `Cm7b5(11)`.

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

For a simple progression:

```cpp
#include "chord.h"
#include <iostream>

using namespace chordreader;

int main() {
    ChordManager manager;
    manager.AddText("DbM9 Ebm9 Cm7-5(11) F7 F7/Bb Bbm Abm Db7 Gb");

    if (const auto key = manager.EstimateKey()) {
        std::cout << "Estimated key: " << GetNoteName(*key, NoteNameStyle::Flats)
                  << " major\n";

        manager.Transpose(-static_cast<int>(*key));
    }

    manager.WriteTo(std::cout, NoteNameStyle::Flats);
    std::cout << '\n';
}
```

For input that may contain invalid symbols, `AddText()` can collect them without aborting the whole parse:

```cpp
std::vector<std::string> errors;
const std::size_t added = manager.AddText(text, &errors);
```

For a single symbol:

```cpp
const ChordData chord = ParseChord("F7/Bb");
const auto notes = GetChordNotes(chord);
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

`ChordManager` owns a progression and provides parsing, iteration, key estimation, transposition, and formatted output. `ParseChord()` and `FormatChord()` are separate so parsing and presentation can evolve independently.

## Limitations

Chord notation is intentionally treated as a practical symbol format. Some highly specialized notation, compound textual expressions, or context-dependent shorthand may require preprocessing before being passed to the parser.

## License

This project is released under the MIT License. See `LICENSE` for the full license text.
