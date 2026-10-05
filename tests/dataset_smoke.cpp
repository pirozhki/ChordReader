#include "chord.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {
    using namespace chordreader;

    const std::string data = R"DATA(
Bb7 AbM9 AbmM7(13) Gm7 C7(b9) Fm7 Fm7/Bb Fm7-5/Eb Eb N.C.
D/C GM7/B C/Bb FM7/A Bb/Ab EbM7/G Ebm7/Gb Fb/Gb Gb/Ab
DbM9 Ebm9 Cm7-5(11) F7 F7/Bb Bbm Abm Db7 Gb Ab/Gb Fm Bbm7 Ebm7 Fm7 GbM7 Bb7sus4/G Gb/Ab Ab Fm7/Gb Ebm7/Ab BbM7 Cm7 Am7-5(11) D7 D7/G Gm Fm Bb7 Eb F/Eb Dm Gm7 Gm7/C Cm7/F
GbM7/Ab Bbm7 GbM7/Ab Bbm7(11) DbM7/F Eb7 Ebm7 Fm7 GbM7 Dm7 FM7 Dm7/G G7 Dm/G G7 Fm13 Fbm13
AbM7 Bb7(11) Gm7 Cm7 Fm9 Bb7(11) Eb Eb7 AbM7 Fm7-5/Bb (Gm7) Gm7 Cm7 Fm9 Bb7(11) Eb Eb7 (Eb7) AbM7 Fm7-5/Bb (Gm7) Gm7 C7 Fm9 Cm/Bb N.C. Ebsus4 Eb
AbM7 Bb7(11) Gm7 Cm7 CbM7 Gb/Bb Adim7 Ab7sus4 Abm7
CbM7 Db7(11) Bbm7 Ebm7 Abm9 Db7(11) Gb Gb7 CbM7 Abm7-5/Db Bbm7 Ebm7 Abm7 Bbm7 AM7 DM7 GM7 Fb/Gb Ab/Bb Bbsus4 Bb Gb/Bb F/A
N.C. Bbsus4/Ab Eb/G N.C. Absus4/Gb Db/F N.C. Fb(#11) Fm7-5/Bb EbM9 Ddim7/Eb CbM7
Aadd9 Asus4 Aadd9 C#7sus4/B C#
F#m F#m C#7/E# Em6 A7(9) DM7 DmM7 Aadd9/C# DM7 C#sus4 C# F#m7 B7(9) A/C# F#m7 GM7(9) D/E C#7/E#
A/E F#7/E B/D# G#7/F# C#/E# BmM7-5/D Ebm Ebm7/Db Cm7-5 Bm6 F#/A# D#m7 DM7 C#7sus4 C#7
F#m C#7/E# Em7(11) G/A Faug/Eb DM7 DmM7 Aadd9/C# C#m7 DM7 C#7sus4 C#7 F#m7 B7(9) A/E E7sus4 A/E E7sus4
(F#M9) G#m/B F#/A# C# D#m G#m/B F#/A# Bm7 Bm7/F# B/C# C#
F# G#m A#m Bsus2-5 Bsus2 A#m7 G#m7 (C#sus24) F# G#m A#m Badd9-5 Badd9 A#m7 D#sus4 D#/G E#m D#
G#m7 A#m7 B A#m7 G#m7 A#m7 B C#7sus4 C#7 Am7/D D Bm7
N.C. CM7 D G Em Am7 D G D G Em7 CM7 D G Em Am7 B Em G/D Bm/D C#m7-5 Am9 Dsus4 Dadd9 N.C. (G)
G#m/B F#/A# C# D#m C#sus4 B#m7-5 E#dim/A# B69
C D Bm Em F#m7-5 B Em Em/F# Em/G Em/B C D D#dim Em FM7 F#m7-5 B
/B /A /F# /D# (E) E D#dim G# C#m Bm E A G#m C#m A A#dim B E D#dim G# C#m Bm E A G#m C# F#m7-5 B B/C# B/D# B/F# C#m
Absus4 Gbadd9 Db/F Ebm7 Ebm7/Ab Dbadd9 Bbm7 Db/Ab GbM7 Dbadd9/F Ebm7 Ebm7/Ab
AM7 E/G# F#m7 C#m7/E DM7 F#m7 C#m7 F#m7 B7/D# Esus4 E AM7 E/G# F#m7 C#m7/E DM7 B7/D# E F#m7 Ab7sus4
Dm C#aug F/C Bm7-5 Bb Am Gm7 C#dim BbM7 C Am Dm Gm C Dm C C#dim Dm Dm C/E F N.C.
FM7 FM7 Am9 Am9 FM7 FM7 Em7 Em7 Dm7 Dm7 G#dim G#dim G#dim
Csus4 C Csus2 C Bbsus4 Bb Bbsus2 Bb Gsus4 G Gsus2 G Fsus4 F Fsus2 F Ebsus4 Eb Ebsus2 Eb
G7sus4 Gadd9 Eb F Dm Gm Cm D Gm D Bb A
Abmaj7 Abmaj7b5 Abmaj7 Abmaj7b5 Gm7 C7sus4 Gm7 C7sus4 Fm7 Gm7 Abmaj7 G7
C/E F G F C/E F G Am Am G/B C G G/F C/E F Gsus4
CM7 B7 Em7 Dm7 G7 Dsus4 D Bm7 Em7 CM7
Em G6/B C7 B7 Em G A Bb B B7 Bb7 E
)DATA";

    ChordManager m;
    std::vector<std::string> errors;
    const std::size_t added = m.AddText(data, &errors);

    // Expected: slash-only and parenthesized tokens are intentionally ignored by
    // ParseChord/AddText only when they fail; everything else should parse.
    std::cout << "added=" << added << " errors=" << errors.size() << " total=" << m.Size() << "\n";
    for (const auto& e : errors) std::cout << "ERROR: " << e << "\n";

    // Spot-check the two fixes requested previously.
    assert(ParseChord("Cdim7").intervals.test(static_cast<std::size_t>(Constituent::m3)));
    assert(ParseChord("Cdim7").intervals.test(static_cast<std::size_t>(Constituent::Dim5)));
    assert(ParseChord("Cdim7").intervals.test(static_cast<std::size_t>(Constituent::M6)));
    assert(ParseChord("A5").intervals.test(static_cast<std::size_t>(Constituent::P5)));
    assert(!ParseChord("A5").intervals.test(static_cast<std::size_t>(Constituent::M3)));
    assert(ParseChord("B69").root == Note::B);
    assert(ParseChord("E#dim/A#").bass == Note::AS);

    // Parenthesized extensions add only the requested tone; they do not
    // implicitly add a 7th.
    const auto fb_sharp11 = ParseChord("Fb(#11)");
    assert(!fb_sharp11.intervals.test(static_cast<std::size_t>(Constituent::m7)));
    assert(fb_sharp11.intervals.test(static_cast<std::size_t>(Constituent::Aug11)));
    const auto c9 = ParseChord("C9");
    assert(c9.intervals.test(static_cast<std::size_t>(Constituent::m7)));
    const auto c11 = ParseChord("C11");
    assert(c11.intervals.test(static_cast<std::size_t>(Constituent::m7)));
    const auto c13 = ParseChord("C13");
    assert(c13.intervals.test(static_cast<std::size_t>(Constituent::m7)));
    const auto c11_add = ParseChord("C(11)");
    assert(!c11_add.intervals.test(static_cast<std::size_t>(Constituent::m7)));

    return 0;
}
