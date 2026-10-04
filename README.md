# Chord Reader
This is a library that reads chords and stores them as a bit sequence.  
It has the ability to estimate keys and output chords in text again.  
For more details on how to use it, check out example.cpp.

### example.cpp  
Input chords as sample.txt  
```
Cm7 F7 Bbmaj7 Ebmaj7 Am(b5) D7 Gm6  
Cm7 F7 Bbmaj7 Ebmaj7 Am(b5) D7 Gm6  
Am7(b5) D7 Gm Cm7 F Bbmaj7 Ebmaj7  
Am7b5 D7 Gm7 Gb7 Fm7 E7 Am7(b5) D7 Gm  
```  
Program extimates key and transposes the chords to C major  
```
Dm7 G7 CM7 FM7 Bm(b5) E7 Am6  
Dm7 G7 CM7 FM7 Bm(b5) E7 Am6  
Bm7(b5) E7 Am Dm7 G CM7 FM7  
Bm7(b5) E7 Am7 G#7 Gm7 F#7 Bm7(b5) E7 Am  
```
