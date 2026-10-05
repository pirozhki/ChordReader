#include "chord.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace chordreader {
namespace {

constexpr int ToInt(Note note) noexcept {
    return static_cast<int>(note);
}

constexpr std::size_t Index(Constituent interval) noexcept {
    return static_cast<std::size_t>(interval);
}

constexpr int PitchClassOf(Constituent interval) noexcept {
    return static_cast<int>(Index(interval) % kNoteCount);
}

Note NoteFromLetter(char c) {
    switch (c) {
    case 'C': return Note::C;
    case 'D': return Note::D;
    case 'E': return Note::E;
    case 'F': return Note::F;
    case 'G': return Note::G;
    case 'A': return Note::A;
    case 'B': return Note::B;
    default:
        throw std::invalid_argument("invalid note letter");
    }
}

Note ParseNoteAt(std::string_view text, std::size_t pos, std::size_t* consumed) {
    if (pos >= text.size()) {
        throw std::invalid_argument("missing note");
    }

    Note note = NoteFromLetter(text[pos]);
    std::size_t count = 1;

    if (pos + 1 < text.size() && (text[pos + 1] == '#' || text[pos + 1] == 'b')) {
        int pitch = ToInt(note) + (text[pos + 1] == '#' ? 1 : -1);
        pitch = (pitch % 12 + 12) % 12;
        note = static_cast<Note>(pitch);
        count = 2;
    }

    if (consumed != nullptr) {
        *consumed = count;
    }
    return note;
}

bool Contains(std::string_view text, std::string_view token) noexcept {
    return text.find(token) != std::string_view::npos;
}

// Return true when the token occurs outside parentheses. Bare extensions such
// as C9/C11/C13 conventionally imply a 7th, while parenthesized additions such
// as C(9)/C(11)/C(#11)/C(13) do not.
bool ContainsOutsideParentheses(std::string_view text, std::string_view token) noexcept {
    int depth = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '(') {
            ++depth;
            continue;
        }
        if (text[i] == ')') {
            if (depth > 0) {
                --depth;
            }
            continue;
        }
        if (depth == 0 && i + token.size() <= text.size() &&
            text.substr(i, token.size()) == token) {
            return true;
        }
    }
    return false;
}

void Set(ChordData& chord, Constituent interval) noexcept {
    chord.intervals.set(Index(interval), true);
}

bool Has(const ChordData& chord, Constituent interval) noexcept {
    return chord.intervals.test(Index(interval));
}

void Clear(ChordData& chord, Constituent interval) noexcept {
    chord.intervals.set(Index(interval), false);
}

std::size_t FindBassDelimiter(std::string_view text, std::size_t start) noexcept {
    const auto slash = text.find('/', start);
    const auto on = text.find("on", start);

    if (slash == std::string_view::npos) return on;
    if (on == std::string_view::npos) return slash;
    return std::min(slash, on);
}

} // namespace

ChordData ParseChord(std::string_view input) {
    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.front()))) {
        input.remove_prefix(1);
    }
    while (!input.empty() && std::isspace(static_cast<unsigned char>(input.back()))) {
        input.remove_suffix(1);
    }

    if (input.empty()) {
        throw std::invalid_argument("empty chord symbol");
    }

    // Keep compatibility with the original N.C./NC handling, while producing a
    // value that can be distinguished from C when formatting it again.
    if (input == "N.C." || input == "N.C" || input == "NC") {
        ChordData chord;
        chord.no_chord = true;
        return chord;
    }

    std::size_t root_length = 0;
    const Note root_note = ParseNoteAt(input, 0, &root_length);

    // Modifiers stop at slash/on so bass parsing never accidentally sees chord text.
    const std::size_t bass_delimiter = FindBassDelimiter(input, root_length);
    const std::size_t modifier_end =
        bass_delimiter == std::string_view::npos ? input.size() : bass_delimiter;
    const std::string_view modifiers = input.substr(root_length, modifier_end - root_length);

    ChordData chord;
    chord.root = root_note;
    chord.bass = root_note;
    Set(chord, Constituent::Root);

    // A bare "5" chord is a power chord: root + perfect 5th, with no 3rd.
    // Keep this deliberately narrow so that modifiers such as b5, #5, or -5
    // continue through the normal chord-building path below.
    const bool power_chord = (modifiers == "5");

    // Diminished chords use a minor 3rd and diminished 5th. A diminished
    // seventh adds a diminished 7th, which is enharmonic to a major 6th in
    // our pitch-class representation.
    const bool diminished = Contains(modifiers, "dim");
    const bool diminished_seventh = diminished && Contains(modifiers, "7");
    chord.diminished_seventh = diminished_seventh;

    if (power_chord) {
        Set(chord, Constituent::P5);
    } else {
        // 3rd / 4th
        if (Contains(modifiers, "sus4")) {
            Set(chord, Constituent::P4);
        } else if (
            Contains(modifiers, "m") &&
            !Contains(modifiers, "maj") &&
            !diminished &&
            !Contains(modifiers, "omit")) {
            Set(chord, Constituent::m3);
        } else if (diminished) {
            Set(chord, Constituent::m3);
        } else {
            Set(chord, Constituent::M3);
        }

        // 5th
        if (Contains(modifiers, "b5") || Contains(modifiers, "-5") || diminished) {
            Set(chord, Constituent::Dim5);
        } else if (Contains(modifiers, "#5") || Contains(modifiers, "+5") || Contains(modifiers, "aug")) {
            Set(chord, Constituent::m6); // enharmonic #5 / b6 shares this pitch class
        } else {
            Set(chord, Constituent::P5);
        }

        // 6th
        if (Contains(modifiers, "6")) {
            Set(chord, Constituent::M6);
        }

        // 7th
        if (diminished_seventh) {
            Set(chord, Constituent::M6); // dim7 is enharmonic to M6 in pitch-class storage
        } else if (
            Contains(modifiers, "M7") ||
            Contains(modifiers, "maj7") ||
            Contains(modifiers, "Maj7")) {
            Set(chord, Constituent::M7);
        } else if (Contains(modifiers, "7")) {
            Set(chord, Constituent::m7);
        }
    }

    // 9th
    // Bare 9th notation (C9) conventionally includes a 7th, while a
    // parenthesized addition (C(9), C7(9), etc.) must not create one.
    // A major-9 chord contains a major 7th as well as the 9th.
    const bool major_ninth =
        Contains(modifiers, "M9") || Contains(modifiers, "maj9") || Contains(modifiers, "Maj9");

    const bool bare_b9 = ContainsOutsideParentheses(modifiers, "b9") ||
                         ContainsOutsideParentheses(modifiers, "-9");
    const bool bare_sharp9 = ContainsOutsideParentheses(modifiers, "#9") ||
                             ContainsOutsideParentheses(modifiers, "+9");

    if (Contains(modifiers, "b9") || Contains(modifiers, "-9")) {
        if (bare_b9) {
            Set(chord, Constituent::m7);
        }
        Set(chord, Constituent::m9);
    } else if (Contains(modifiers, "#9") || Contains(modifiers, "+9")) {
        if (bare_sharp9) {
            Set(chord, Constituent::m7);
        }
        Set(chord, Constituent::Aug9);
    } else if (Contains(modifiers, "add9")) {
        Set(chord, Constituent::M9);
    } else if (major_ninth) {
        Set(chord, Constituent::M7);
        Set(chord, Constituent::M9);
    } else if (Contains(modifiers, "9")) {
        Set(chord, Constituent::M9);
        if (ContainsOutsideParentheses(modifiers, "9") &&
            !Has(chord, Constituent::M7) && !Has(chord, Constituent::M6)) {
            Set(chord, Constituent::m7);
        }
    }

    // 11th
    const bool major_eleventh = Contains(modifiers, "M11") ||
                                Contains(modifiers, "maj11") ||
                                Contains(modifiers, "Maj11");
    if (major_eleventh) {
        Set(chord, Constituent::M7);
    }

    const bool bare_sharp11 = ContainsOutsideParentheses(modifiers, "#11") ||
                              ContainsOutsideParentheses(modifiers, "+11");
    const bool bare_11 = ContainsOutsideParentheses(modifiers, "11");
    const bool has_any_9 = Contains(modifiers, "9");

    if (Contains(modifiers, "#11") || Contains(modifiers, "+11")) {
        if (bare_sharp11 && !Has(chord, Constituent::M7) && !Has(chord, Constituent::M6)) {
            Set(chord, Constituent::m7);
        }
        Set(chord, Constituent::Aug11);
    } else if (Contains(modifiers, "11")) {
        if (bare_11 && !Has(chord, Constituent::M7) && !Has(chord, Constituent::M6)) {
            Set(chord, Constituent::m7);
        }
        if (bare_11 && !has_any_9) {
            // A bare 11th chord conventionally contains the natural 9th as well.
            // An explicitly written 9/b9/#9 is left to its own notation.
            Set(chord, Constituent::M9);
        }
        Set(chord, Constituent::P11);
    }

    // 13th
    const bool major_thirteenth = Contains(modifiers, "M13") ||
                                  Contains(modifiers, "maj13") ||
                                  Contains(modifiers, "Maj13");
    if (major_thirteenth) {
        Set(chord, Constituent::M7);
    }

    const bool bare_b13 = ContainsOutsideParentheses(modifiers, "b13") ||
                          ContainsOutsideParentheses(modifiers, "-13");
    const bool bare_13 = ContainsOutsideParentheses(modifiers, "13");
    if (Contains(modifiers, "b13") || Contains(modifiers, "-13")) {
        if (bare_b13) {
            Set(chord, Constituent::m7);
        }
        Set(chord, Constituent::m13);
    } else if (Contains(modifiers, "13")) {
        if (bare_13 && !Has(chord, Constituent::M7) && !Has(chord, Constituent::M6)) {
            Set(chord, Constituent::m7);
        }
        if (bare_13 && !has_any_9) {
            // A bare 13th chord conventionally contains the natural 9th as well.
            // An explicitly written 9/b9/#9 is left to its own notation.
            Set(chord, Constituent::M9);
        }
        Set(chord, Constituent::M13);
    }

    // Explicit omissions are applied last.
    if (Contains(modifiers, "omit5")) {
        Clear(chord, Constituent::Dim5);
        Clear(chord, Constituent::P5);
        Clear(chord, Constituent::m6);
    }
    if (Contains(modifiers, "omit3")) {
        Clear(chord, Constituent::m3);
        Clear(chord, Constituent::M3);
    }

    if (bass_delimiter != std::string_view::npos) {
        const std::size_t bass_start =
            input[bass_delimiter] == '/' ? bass_delimiter + 1 : bass_delimiter + 2;
        std::size_t bass_length = 0;
        chord.bass = ParseNoteAt(input, bass_start, &bass_length);

        const std::size_t remaining = bass_start + bass_length;
        if (remaining != input.size()) {
            throw std::invalid_argument("invalid bass note in chord symbol");
        }
    }

    return chord;
}

std::string GetNoteName(Note note, NoteNameStyle style) {
    static constexpr std::array<std::string_view, 12> sharps = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    static constexpr std::array<std::string_view, 12> flats = {
        "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
    };

    const std::size_t index = static_cast<std::size_t>(note) % 12;
    return std::string((style == NoteNameStyle::Flats ? flats : sharps)[index]);
}

Note TransposeNote(Note note, int semitones) noexcept {
    int pitch = ToInt(note) + semitones;
    pitch = (pitch % 12 + 12) % 12;
    return static_cast<Note>(pitch);
}

std::bitset<kNoteCount> GetChordNotes(const ChordData& chord) {
    std::bitset<kNoteCount> result;
    if (chord.no_chord) {
        return result;
    }

    for (std::size_t i = 0; i < kConstituentCount; ++i) {
        if (!chord.intervals.test(i)) {
            continue;
        }

        const int pitch = (ToInt(chord.root) + static_cast<int>(i % kNoteCount)) % 12;
        result.set(static_cast<std::size_t>(pitch));
    }
    return result;
}

std::string FormatChord(const ChordData& chord, NoteNameStyle style) {
    if (chord.no_chord) {
        return "N.C.";
    }

    const bool dim7 = chord.diminished_seventh;

    const bool power_chord =
        Has(chord, Constituent::P5) &&
        !Has(chord, Constituent::m3) &&
        !Has(chord, Constituent::M3) &&
        !Has(chord, Constituent::P4) &&
        !Has(chord, Constituent::Dim5) &&
        !Has(chord, Constituent::m6) &&
        !Has(chord, Constituent::M6) &&
        !Has(chord, Constituent::m7) &&
        !Has(chord, Constituent::M7) &&
        !Has(chord, Constituent::M9) &&
        !Has(chord, Constituent::m9) &&
        !Has(chord, Constituent::Aug9) &&
        !Has(chord, Constituent::P11) &&
        !Has(chord, Constituent::Aug11) &&
        !Has(chord, Constituent::m13) &&
        !Has(chord, Constituent::M13);

    std::string name = GetNoteName(chord.root, style);

    // Keep the original ChordReader's naming order and conventions.
    // This is important for compatibility: 7 comes before sus4, so
    // Bb7sus4 remains Bb7sus4 rather than Bb sus47.
    const bool diminished_triad =
        Has(chord, Constituent::m3) &&
        Has(chord, Constituent::Dim5) &&
        !dim7 &&
        !Has(chord, Constituent::m7) &&
        !Has(chord, Constituent::M7);

    if (Has(chord, Constituent::m3) && !diminished_triad && !dim7) {
        name += "m";
    }
    if (diminished_triad) {
        name += "dim";
    }
    if (Has(chord, Constituent::m3) && Has(chord, Constituent::Dim5) && dim7) {
        name += "dim7";
    }
    if (Has(chord, Constituent::m6)) {
        name += "aug";
    }
    if (Has(chord, Constituent::M6) && !Has(chord, Constituent::Dim5)) {
        name += "6";
    }
    if (Has(chord, Constituent::m7)) {
        name += "7";
    }
    if (Has(chord, Constituent::M7)) {
        name += "M7";
    }
    if (Has(chord, Constituent::P4)) {
        name += "sus4";
    }

    if ((!Has(chord, Constituent::m7) &&
         !Has(chord, Constituent::M7) &&
         !Has(chord, Constituent::m6) &&
         !dim7) &&
        Has(chord, Constituent::M9)) {
        name += "add9";
    }

    // b5 is a chord-quality alteration, so keep it outside the parentheses.
    // For example: Cm7b5(11), not Cm7(b5,11).
    const bool has_b5 = Has(chord, Constituent::Dim5) && !diminished_triad && !dim7;
    if (has_b5) {
        name += "b5";
    }

    std::vector<std::string> tensions;
    if (Has(chord, Constituent::m9)) {
        tensions.emplace_back("b9");
    }

    if ((dim7 || Has(chord, Constituent::m7) || Has(chord, Constituent::M7)) &&
        Has(chord, Constituent::M9)) {
        tensions.emplace_back("9");
    }

    if (Has(chord, Constituent::P11)) {
        tensions.emplace_back("11");
    }
    if (Has(chord, Constituent::Aug11)) {
        tensions.emplace_back("#11");
    }
    if (Has(chord, Constituent::m13)) {
        tensions.emplace_back("b13");
    }
    if (Has(chord, Constituent::M13)) {
        tensions.emplace_back("13");
    }

    if (!tensions.empty()) {
        name += "(";
        for (std::size_t i = 0; i < tensions.size(); ++i) {
            if (i != 0) {
                name += ",";
            }
            name += tensions[i];
        }
        name += ")";
    }

    // The original library represented a bare power chord simply as root5.
    // Omit markers are otherwise kept compatible with the original formatter.
    if (power_chord) {
        name = GetNoteName(chord.root, style) + "5";
    } else {
        if (!Has(chord, Constituent::m3) &&
            !Has(chord, Constituent::M3) &&
            !Has(chord, Constituent::P4)) {
            name += "omit3";
        }

        if (!Has(chord, Constituent::Dim5) &&
            !Has(chord, Constituent::P5) &&
            !Has(chord, Constituent::m6)) {
            name += "omit5";
        }
    }

    if (chord.root != chord.bass) {
        name += "/";
        name += GetNoteName(chord.bass, style);
    }

    return name;
}

ChordData ChordManager::AddChord(std::string_view chord_name) {
    ChordData chord = ParseChord(chord_name);
    m_chords.push_back(chord);
    return chord;
}

bool ChordManager::TryAddChord(std::string_view chord_name, std::string* error) noexcept {
    try {
        AddChord(chord_name);
        return true;
    } catch (const std::exception& ex) {
        if (error != nullptr) {
            *error = ex.what();
        }
        return false;
    } catch (...) {
        if (error != nullptr) {
            *error = "unknown parse error";
        }
        return false;
    }
}

std::size_t ChordManager::AddText(
    std::string_view text,
    std::vector<std::string>* errors) {

    std::size_t added = 0;
    std::istringstream stream{std::string(text)};
    std::string token;

    while (stream >> token) {
        std::string error;
        if (TryAddChord(token, &error)) {
            ++added;
        } else if (errors != nullptr) {
            errors->push_back(token + ": " + error);
        }
    }

    return added;
}

void ChordManager::Transpose(int semitones) noexcept {
    for (ChordData& chord : m_chords) {
        if (chord.no_chord) {
            continue;
        }
        chord.root = TransposeNote(chord.root, semitones);
        chord.bass = TransposeNote(chord.bass, semitones);
    }
}

std::optional<Note> ChordManager::EstimateKey() const noexcept {
    std::array<int, 12> note_count{};

    for (const ChordData& chord : m_chords) {
        const auto notes = GetChordNotes(chord);
        for (std::size_t i = 0; i < 12; ++i) {
            if (notes.test(i)) {
                ++note_count[i];
            }
        }
    }

    if (std::all_of(note_count.begin(), note_count.end(), [](int count) { return count == 0; })) {
        return std::nullopt;
    }

    constexpr std::array<int, 12> score = {
        +1, -1, +1, -1, +1, +1, -1, +1, -1, +1, -1, +1
    };

    int best_score = std::numeric_limits<int>::min();
    int best_key = 0;

    for (int key = 0; key < 12; ++key) {
        int sum = 0;
        for (int interval = 0; interval < 12; ++interval) {
            sum += note_count[(key + interval) % 12] * score[interval];
        }

        if (sum > best_score) {
            best_score = sum;
            best_key = key;
        }
    }

    return static_cast<Note>(best_key);
}

void ChordManager::WriteTo(
    std::ostream& os,
    NoteNameStyle style,
    std::string_view separator) const {

    for (std::size_t i = 0; i < m_chords.size(); ++i) {
        if (i != 0) {
            os << separator;
        }
        os << FormatChord(m_chords[i], style);
    }
}

} // namespace chordreader

