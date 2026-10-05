#include "chord.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace chordreader;

int main() {
    {
        const ChordData c = ParseChord("Cm7");
        assert(c.root == Note::C);
        assert(c.bass == Note::C);
        assert(FormatChord(c) == "Cm7");
    }

    {
        const ChordData c = ParseChord("Bbmaj7");
        assert(c.root == Note::AS);
        assert(FormatChord(c, NoteNameStyle::Flats) == "BbM7");
        assert(FormatChord(c) == "A#M7");
    }

    {
        const ChordData c = ParseChord("Am7(b5)/C");
        assert(c.root == Note::A);
        assert(c.bass == Note::C);
        assert(FormatChord(c) == "Am7b5/C");
    }

    {
        const ChordData c = ParseChord("D7onF#");
        assert(c.root == Note::D);
        assert(c.bass == Note::FS);
        assert(FormatChord(c) == "D7/F#");
    }

    // Diminished and power-chord fixes.
    {
        const ChordData c = ParseChord("Cdim");
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::m3)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::Dim5)));
        assert(!c.intervals.test(static_cast<std::size_t>(Constituent::M6)));
        assert(FormatChord(c) == "Cdim");
    }

    {
        const ChordData c = ParseChord("Cdim7");
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::m3)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::Dim5)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::M6)));
        assert(c.diminished_seventh);
        assert(FormatChord(c) == "Cdim7");
    }

    {
        const ChordData c = ParseChord("A5");
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::Root)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::P5)));
        assert(!c.intervals.test(static_cast<std::size_t>(Constituent::m3)));
        assert(!c.intervals.test(static_cast<std::size_t>(Constituent::M3)));
        assert(FormatChord(c) == "A5");
    }

    // Original output ordering: 7sus4, not sus47.
    {
        assert(FormatChord(ParseChord("Bb7sus4"), NoteNameStyle::Flats) == "Bb7sus4");
        assert(FormatChord(ParseChord("Bb7sus4/G"), NoteNameStyle::Flats) == "Bb7sus4/G");
    }

    // Major ninth must retain a major 7th. The original formatter expresses it
    // canonically as M7(9), rather than changing it into 7(9).
    {
        const ChordData c = ParseChord("DbM9");
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::M7)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(!c.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(FormatChord(c, NoteNameStyle::Flats) == "DbM7(9)");

        ChordManager manager;
        manager.AddChord("DbM9");
        manager.Transpose(-1);
        assert(FormatChord(manager[0], NoteNameStyle::Flats) == "CM7(9)");
    }

    {
        const ChordData c = ParseChord("CM9");
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::M7)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(!c.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(FormatChord(c) == "CM7(9)");
    }

    {
        const ChordData c = ParseChord("AbmM7(13)");
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::m3)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::M7)));
        assert(!c.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::M13)));
        assert(FormatChord(c, NoteNameStyle::Flats) == "AbmM7(13)");
    }

    {
        const ChordData c = ParseChord("Am7(b5,11)");
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::m3)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::Dim5)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::P11)));
        assert(FormatChord(c) == "Am7b5(11)");
    }

    {
        const ChordData c = ParseChord("Am7-5(11)");
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::m3)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::Dim5)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(c.intervals.test(static_cast<std::size_t>(Constituent::P11)));
        assert(FormatChord(c) == "Am7b5(11)");
    }

    {
        // Diminished-triad spellings are normalized to dim, while diminished
        // seventh chords retain the explicit dim7 quality.
        const ChordData dim = ParseChord("Cdim");
        assert(!dim.diminished_seventh);
        assert(FormatChord(dim) == "Cdim");

        const ChordData mb5 = ParseChord("Cmb5");
        assert(FormatChord(mb5) == "Cdim");

        const ChordData mb5_alt = ParseChord("Cm-5");
        assert(FormatChord(mb5_alt) == "Cdim");

        const ChordData half_dim = ParseChord("Cm7b5");
        assert(FormatChord(half_dim) == "Cm7b5");

        const ChordData dim7 = ParseChord("Cdim7");
        assert(dim7.diminished_seventh);
        assert(FormatChord(dim7) == "Cdim7");
    }

    {
        // Bare extensions imply a 7th; parenthesized additions do not.
        const ChordData c9 = ParseChord("C9");
        assert(c9.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(c9.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(FormatChord(c9) == "C7(9)");

        const ChordData c9_add = ParseChord("C(9)");
        assert(!c9_add.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(c9_add.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(FormatChord(c9_add) == "Cadd9");

        const ChordData c11 = ParseChord("C11");
        assert(c11.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(c11.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(c11.intervals.test(static_cast<std::size_t>(Constituent::P11)));
        assert(FormatChord(c11) == "C7(9,11)");

        const ChordData cm11 = ParseChord("Cm11");
        assert(cm11.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(cm11.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(cm11.intervals.test(static_cast<std::size_t>(Constituent::P11)));
        assert(FormatChord(cm11) == "Cm7(9,11)");

        const ChordData cm11_major = ParseChord("CM11");
        assert(cm11_major.intervals.test(static_cast<std::size_t>(Constituent::M7)));
        assert(cm11_major.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(cm11_major.intervals.test(static_cast<std::size_t>(Constituent::P11)));
        assert(FormatChord(cm11_major) == "CM7(9,11)");

        const ChordData c11_add = ParseChord("C(11)");
        assert(!c11_add.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(!c11_add.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(c11_add.intervals.test(static_cast<std::size_t>(Constituent::P11)));

        const ChordData c7_11 = ParseChord("C7(11)");
        assert(c7_11.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(!c7_11.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(c7_11.intervals.test(static_cast<std::size_t>(Constituent::P11)));

        const ChordData fsharp11 = ParseChord("C(#11)");
        assert(!fsharp11.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(fsharp11.intervals.test(static_cast<std::size_t>(Constituent::Aug11)));
        assert(FormatChord(fsharp11) == "C(#11)");

        const ChordData fbsharp11 = ParseChord("Fb(#11)");
        assert(!fbsharp11.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(fbsharp11.intervals.test(static_cast<std::size_t>(Constituent::Aug11)));

        const ChordData c13 = ParseChord("C13");
        assert(c13.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(c13.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(c13.intervals.test(static_cast<std::size_t>(Constituent::M13)));
        assert(FormatChord(c13) == "C7(9,13)");

        const ChordData cm13 = ParseChord("Cm13");
        assert(cm13.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(cm13.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(cm13.intervals.test(static_cast<std::size_t>(Constituent::M13)));
        assert(FormatChord(cm13) == "Cm7(9,13)");

        const ChordData cm13_major = ParseChord("CM13");
        assert(cm13_major.intervals.test(static_cast<std::size_t>(Constituent::M7)));
        assert(cm13_major.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(cm13_major.intervals.test(static_cast<std::size_t>(Constituent::M13)));
        assert(FormatChord(cm13_major) == "CM7(9,13)");

        const ChordData c13_add = ParseChord("C(13)");
        assert(!c13_add.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(!c13_add.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(c13_add.intervals.test(static_cast<std::size_t>(Constituent::M13)));

        const ChordData c7_13 = ParseChord("C7(13)");
        assert(c7_13.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(!c7_13.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(c7_13.intervals.test(static_cast<std::size_t>(Constituent::M13)));

        const ChordData c13_b9 = ParseChord("C13(b9)");
        assert(c13_b9.intervals.test(static_cast<std::size_t>(Constituent::m7)));
        assert(c13_b9.intervals.test(static_cast<std::size_t>(Constituent::m9)));
        assert(!c13_b9.intervals.test(static_cast<std::size_t>(Constituent::M9)));
        assert(c13_b9.intervals.test(static_cast<std::size_t>(Constituent::M13)));
    }

    {
        const ChordData c = ParseChord("N.C.");
        assert(c.IsNoChord());
        assert(FormatChord(c) == "N.C.");
    }

    {
        ChordManager manager;
        std::vector<std::string> errors;
        manager.AddText("Cm7 F7 Bbmaj7 Ebmaj7 X", &errors);
        assert(manager.Size() == 4);
        assert(errors.size() == 1);
        const auto key = manager.EstimateKey();
        assert(key.has_value());
        assert(*key == Note::AS);
        manager.Transpose(-10);
        assert(FormatChord(manager[0]) == "Dm7");
    }

    std::cout << "All tests passed.\n";
    return 0;
}
