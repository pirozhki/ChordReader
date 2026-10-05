#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace chordreader {

constexpr std::size_t kNoteCount = 12;
constexpr std::size_t kConstituentCount = 24;

// Pitch class. C == 0, C# == 1, ... B == 11.
enum class Note : std::uint8_t {
    C  = 0,
    CS = 1,
    D  = 2,
    DS = 3,
    E  = 4,
    F  = 5,
    FS = 6,
    G  = 7,
    GS = 8,
    A  = 9,
    AS = 10,
    B  = 11,
};

enum class NoteNameStyle {
    Sharps,
    Flats,
};

// The bit layout intentionally keeps the original ChordReader representation.
// Indices 0..23 correspond to root, m2, M2, ... M14.
enum class Constituent : std::uint8_t {
    Root = 0,
    m2,
    M2,
    m3,
    M3,
    P4,
    Dim5,
    P5,
    m6,
    M6,
    m7,
    M7,
    P8,
    m9,
    M9,
    Aug9,
    m10,
    P11,
    Aug11,
    P12,
    m13,
    M13,
    m14,
    M14,
};

struct ChordData {
    std::bitset<kConstituentCount> intervals{};
    Note root{Note::C};
    Note bass{Note::C};
    bool no_chord{false};
    // Retain whether the diminished seventh was explicitly requested.
    // The pitch-class representation cannot distinguish dim7 from M6 alone.
    bool diminished_seventh{false};

    [[nodiscard]] bool IsNoChord() const noexcept { return no_chord; }
};

// Parse a single chord symbol such as Cm7, Bbmaj7, Am7(b5), C/G, or N.C.
// Throws std::invalid_argument when the symbol cannot be parsed.
[[nodiscard]] ChordData ParseChord(std::string_view chord_name);

// Convert chord data back to a canonical chord symbol.
[[nodiscard]] std::string FormatChord(
    const ChordData& chord,
    NoteNameStyle style = NoteNameStyle::Sharps);

[[nodiscard]] std::string GetNoteName(
    Note note,
    NoteNameStyle style = NoteNameStyle::Sharps);

[[nodiscard]] Note TransposeNote(Note note, int semitones) noexcept;

// Return the 12 pitch classes contained in the chord.
[[nodiscard]] std::bitset<kNoteCount> GetChordNotes(const ChordData& chord);

class ChordManager {
public:
    using container_type = std::vector<ChordData>;
    using const_iterator = container_type::const_iterator;

    // Parse and append one chord. The parsed value is also returned for convenience.
    ChordData AddChord(std::string_view chord_name);

    // Parse and append one chord without throwing. Returns false on invalid input.
    bool TryAddChord(std::string_view chord_name, std::string* error = nullptr) noexcept;

    // Parse whitespace-separated chord symbols. Invalid symbols are skipped and their
    // original text is optionally collected in errors. Returns number of chords added.
    std::size_t AddText(
        std::string_view text,
        std::vector<std::string>* errors = nullptr);

    void Clear() noexcept { m_chords.clear(); }

    [[nodiscard]] bool Empty() const noexcept { return m_chords.empty(); }
    [[nodiscard]] std::size_t Size() const noexcept { return m_chords.size(); }

    // No copy is made. The old API returned a full vector copy here.
    [[nodiscard]] const container_type& GetChords() const noexcept { return m_chords; }
    [[nodiscard]] const ChordData& operator[](std::size_t index) const { return m_chords.at(index); }

    // Transpose every chord in-place. No-chord entries are left unchanged.
    void Transpose(int semitones) noexcept;

    // Estimate a major key from the stored chord tones. Returns nullopt for empty/
    // pitch-less input instead of the old uninitialized-value behavior.
    [[nodiscard]] std::optional<Note> EstimateKey() const noexcept;

    [[nodiscard]] const_iterator begin() const noexcept { return m_chords.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return m_chords.end(); }

    // Convenience output; the library itself does not write to std::cout.
    void WriteTo(
        std::ostream& os,
        NoteNameStyle style = NoteNameStyle::Sharps,
        std::string_view separator = " ") const;

private:
    container_type m_chords;
};

} // namespace chordreader

// Backward-compatible aliases for the old global names.
using ChordData = chordreader::ChordData;
using ChordManager = chordreader::ChordManager;
using Root = chordreader::Note;
using Constituent = chordreader::Constituent;

constexpr auto C  = chordreader::Note::C;
constexpr auto CS = chordreader::Note::CS;
constexpr auto D  = chordreader::Note::D;
constexpr auto DS = chordreader::Note::DS;
constexpr auto E  = chordreader::Note::E;
constexpr auto F  = chordreader::Note::F;
constexpr auto FS = chordreader::Note::FS;
constexpr auto G  = chordreader::Note::G;
constexpr auto GS = chordreader::Note::GS;
constexpr auto A  = chordreader::Note::A;
constexpr auto AS = chordreader::Note::AS;
constexpr auto B  = chordreader::Note::B;
